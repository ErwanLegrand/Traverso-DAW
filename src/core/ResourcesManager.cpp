/*
Copyright (C) 2006-2007 Remon Sijrier 

This file is part of Traverso

Traverso is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

*/

#include "ResourcesManager.h"
#include "WriteSource.h"
#include "ReadSource.h"
#include "Information.h"
#include "AudioClip.h"
#include "Project.h"
#include "Song.h"
#include "Utils.h"
#include "AudioDevice.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**	\class ResourcesManager
	\brief A class used to load / save the state of, create and delete ReadSources and AudioClips
 
 */

ResourcesManager::ResourcesManager(Project* project)
	: QObject(project)
	, m_project(project)
{
	PENTERCONS;
	m_silentReadSource = 0;
}


ResourcesManager::~ResourcesManager()
{
	PENTERDES;
	foreach(SourceData* data, m_sources) {
		if (! data->source->ref()) {
			delete data->source;
		}
		delete data;
	}
	
	foreach(ClipData* data, m_clips) {
		delete data->clip;
		delete data;
	}

}


QDomNode ResourcesManager::get_state( QDomDocument doc )
{
	PENTER;

	QDomElement rsmNode = doc.createElement("ResourcesManager");
	
	QDomElement audioSourcesElement = doc.createElement("AudioSources");
	
	foreach(SourceData* data, m_sources) {
		AudioSource* source = (AudioSource*)data->source;
		audioSourcesElement.appendChild(source->get_state(doc));
	}
	
	rsmNode.appendChild(audioSourcesElement);
	
	
	QDomElement audioClipsElement = doc.createElement("AudioClips");
	
	foreach(ClipData* data, m_clips) {
		AudioClip* clip = data->clip;
		
		if (data->isCopy && data->removed) {
			continue;
		}
		
		// If the clip was 'getted', then it's state has been fully set
		// and likely changed, so we can get the 'new' state from it.
		if (data->inUse) {
			audioClipsElement.appendChild(clip->get_state(doc));
		} else {
			// In case it wasn't we should use the 'old' domNode which 
			// was set during set_state();
			audioClipsElement.appendChild(clip->get_dom_node()); 
		}
	}
	
	rsmNode.appendChild(audioClipsElement);
	
	return rsmNode;
}


int ResourcesManager::set_state( const QDomNode & node )
{
	QDomNode sourcesNode = node.firstChildElement("AudioSources").firstChild();
	
	while(!sourcesNode.isNull()) {
		ReadSource* source = new ReadSource(sourcesNode);
		SourceData* data = new SourceData();
		data->source = source;
		m_sources.insert(source->get_id(), data);
		sourcesNode = sourcesNode.nextSibling();
		if (source->get_channel_count() == 0) {
			m_silentReadSource = source;
		}
	}
	
	
	QDomNode clipsNode = node.firstChildElement("AudioClips").firstChild();
	
	while(!clipsNode.isNull()) {
		AudioClip* clip = new AudioClip(clipsNode);
		ClipData* data = new ClipData();
		data->clip = clip;
		m_clips.insert(clip->get_id(), data);
		clipsNode = clipsNode.nextSibling();
	}
	
	
	emit stateRestored();
	
	return 1;
}


ReadSource* ResourcesManager::import_source(const QString& dir, const QString& name)
{
	QString fileName = dir + name;
	foreach(SourceData* data, m_sources) {
		if (data->source->get_filename() == fileName) {
			printf("id is %lld\n", data->source->get_id());
			return get_readsource(data->source->get_id()); 
		}
	}
	
	ReadSource* source = new ReadSource(dir, name);
	SourceData* data = new SourceData();
	data->source = source;
	source->set_created_by_song(m_project->get_current_song()->get_id());
	
	m_sources.insert(source->get_id(), data);
	
	source = get_readsource(source->get_id());
	
	if (source->get_error() < 0) {
		m_sources.remove(source->get_id());
		delete source;
		return 0;
	}
	
	emit sourceAdded(source);

	return source;
}


ReadSource* ResourcesManager::create_recording_source(
	const QString& dir,
	const QString& name,
	int channelCount,
 	qint64 songId)
{
	PENTER;
	
	ReadSource* source = new ReadSource(dir, name, channelCount);
	SourceData* data = new SourceData();
	data->source = source;
	
	source->set_original_bit_depth(audiodevice().get_bit_depth());
	source->set_created_by_song(songId);
	source->ref();
	
	m_sources.insert(source->get_id(), data);
	
	emit sourceAdded(source);
	
	return source;
}


ReadSource* ResourcesManager::get_silent_readsource()
{
	if (!m_silentReadSource) {
		m_silentReadSource = new ReadSource();
		SourceData* data = new SourceData();
		data->source = m_silentReadSource;
		m_sources.insert(m_silentReadSource->get_id(), data);
		m_silentReadSource->set_created_by_song( -1 );
	}
	
	m_silentReadSource = get_readsource(m_silentReadSource->get_id());
	
	return m_silentReadSource;
}


ReadSource * ResourcesManager::get_readsource(qint64 id)
{
	SourceData* data = m_sources.value(id);
	
	if (!data) {
		PWARN("ResourcesManager::get_readsource(): ReadSource with id %lld is not in my database!", id);
		return 0;
	}
	
	ReadSource* source = data->source;
	
	// When the AudioSource is "get", do a ref counting.
	// If the source allready was ref counted, create a deep copy
	if (source->ref()) {
		PMESG("Creating deep copy of ReadSource: %s", QS_C(source->get_name()));
		source = source->deep_copy();
		source->ref();
	}
	
	if ( source->init() < 0) {
		info().warning( tr("ResourcesManager::  Failed to initialize ReadSource %1")
				.arg(source->get_filename()));
	}
	
	return source;
}


/**
 * 	Get the AudioClip with id \a id

	This function will return 0 if no AudioClip was found with id \a id.
	
	Only ONE AudioClip instance with this id can be retrieved via this function. 
	Using this function multiple times with the same id will implicitely create 
	a new AudioClip with a new unique id!!

 * @param id 	The unique id of the AudioClip to get
 * @return 	The AudioClip with id \a id, 0 if no AudioClip was found, and a 
		'deep copy' of the AudioClip with id \a id if the AudioClip was allready
		getted before via this function.
 */
AudioClip* ResourcesManager::get_clip(qint64 id)
{
	ClipData* data = m_clips.value(id);
	 
	if (!data) {
		return 0;
	}
	
	AudioClip* clip = data->clip;
	
	if (data->inUse && !data->removed) {
		PMESG("Creating deep copy of Clip %s", QS_C(clip->get_name()));
		clip = clip->create_copy();
		
		// It is NOT possible for 2 clips to have the same ID
		// check for this, since it's absolutely crucial, and 
		// indicates a design error somewhere ! (most likely in 
		// AudioClip::create_copy();
		Q_ASSERT( ! m_clips.contains(clip->get_id()) );
		ClipData* copy = new ClipData();
		copy->clip = clip;
		copy->isCopy = true;
		
		m_clips.insert(clip->get_id(), copy);
		
		// Now that we have created a copy of the audioclip, start
		// the usual get_clip routine again to properly init the clip
		// and other stuff.
		return get_clip(clip->get_id());
	}
	
	ReadSource* source = get_readsource(data->clip->get_readsource_id());
	clip->set_audio_source(source);
	
	data->inUse = true;
	
	return clip;
}

AudioClip* ResourcesManager::new_audio_clip(const QString& name)
{
	PENTER;
	AudioClip* clip = new AudioClip(name);
	ClipData* data = new ClipData();
	data->clip = clip;
	m_clips.insert(clip->get_id(), data);
	return get_clip(clip->get_id());
}

QList<ReadSource*> ResourcesManager::get_all_audio_sources( ) const
{
	QList< ReadSource * > list;
	foreach(SourceData* data, m_sources) {
		list.append(data->source);
	}
	if (m_silentReadSource) {
		list.removeAll(m_silentReadSource);
	}
	return list;
}

QList< AudioClip * > ResourcesManager::get_all_clips() const
{
	QList<AudioClip* > list;
	foreach(ClipData* data, m_clips) {
		list.append(data->clip);
	}
	return list;
}


void ResourcesManager::mark_clip_removed(AudioClip * clip)
{
	ClipData* data = m_clips.value(clip->get_id());
	if (!data) {
		PERROR("Clip with id %lld is not in my database!", clip->get_id());
		return;
	}
	data->removed = true;
	
	SourceData* sourcedata = m_sources.value(clip->get_readsource_id());
	if (!sourcedata) {
		PERROR("Source %lld not in database", clip->get_readsource_id());
	} else {
		sourcedata->clipCount--;
	}
	
	emit clipRemoved(clip);
}

void ResourcesManager::mark_clip_added(AudioClip * clip)
{
	ClipData* clipdata = m_clips.value(clip->get_id());
	if (!clipdata) {
		PERROR("Clip with id %lld is not in my database!", clip->get_id());
		return;
	}
	clipdata->removed = false;
	
	SourceData* sourcedata = m_sources.value(clip->get_readsource_id());
	if (!sourcedata) {
		PERROR("Source %lld not in database", clip->get_readsource_id());
	} else {
		sourcedata->clipCount++;
	}
	
	emit clipAdded(clip);
}


bool ResourcesManager::is_clip_in_use(qint64 id) const
{
	ClipData* data = m_clips.value(id);
	if (!data) {
		PERROR("Clip with id %lld is not in my database!", id);
		return false;
	}
	return data->inUse && !data->removed;
}

bool ResourcesManager::is_source_in_use(qint64 id) const
{
	SourceData* data = m_sources.value(id);
	if (!data) {
		PERROR("Source with id %lld is not in my database!", id);
		return false;
	}
	
	return data->clipCount > 0 ? true : false;
}

void ResourcesManager::set_source_for_clip(AudioClip * clip, ReadSource * source)
{
	clip->set_audio_source(source);
}

ResourcesManager::SourceData::SourceData()
{
	source = 0;
	clipCount = 0;
}

ResourcesManager::ClipData::ClipData()
{
	clip = 0;
	inUse = false;
	isCopy = false;
	removed = false;
}

void ResourcesManager::destroy_clip(AudioClip * clip)
{
	ClipData* data = m_clips.value(clip->get_id());
	if (!data) {
		PERROR("Clip with id %lld not in database", clip->get_id());
	} else {
		m_clips.remove(clip->get_id());
		delete data;
		delete clip;
	}
	
}

