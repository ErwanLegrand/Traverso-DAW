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
#include "Utils.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**	\class ResourcesManager
	\brief A class used to load / save the state of, create and delete ReadSources and AudioClips
 
 */

ResourcesManager::ResourcesManager()
{
	PENTERCONS;
	m_silentReadSource = 0;
}


ResourcesManager::~ResourcesManager()
{
	PENTERDES;
	foreach(ReadSource* source, m_sources) {
		if (! source->ref()) {
			delete source;
		}
	}
	
	foreach(AudioClip* clip, m_clips) {
		delete clip;
	}

}


QDomNode ResourcesManager::get_state( QDomDocument doc )
{
	PENTER;

	QDomElement asmNode = doc.createElement("ResourcesManager");
	
	QDomElement audioSourcesElement = doc.createElement("AudioSources");
	
	foreach(AudioSource* source, m_sources) {
		audioSourcesElement.appendChild(source->get_state(doc));
	}
	
	asmNode.appendChild(audioSourcesElement);
	
	
	QDomElement audioClipsElement = doc.createElement("AudioClips");
	
	QList<AudioClip*> list = m_clips.values();
	
	
	for (int i=0; i<list.size(); ++i) {
		AudioClip* clip = list.at(i);
		
		// Omit all clips that were deprecated:
		if (m_deprecatedClips.contains(clip->get_id())) {
			continue;
		}
		
		// If the clip was refcounted, then it's state has been fully set
		// and likely changed, so we can get the 'new' state from it.
		if (clip->get_ref_count()) {
			audioClipsElement.appendChild(clip->get_state(doc));
		} else {
			// In case it wasn't we should use the 'old' domNode which 
			// was set during set_state();
			audioClipsElement.appendChild(clip->get_dom_node()); 
		}
	}
	
	asmNode.appendChild(audioClipsElement);
	
	return asmNode;
}


int ResourcesManager::set_state( const QDomNode & node )
{
	QDomNode sourcesNode = node.firstChildElement("AudioSources").firstChild();
	
	while(!sourcesNode.isNull()) {
		ReadSource* source = new ReadSource(sourcesNode);
		m_sources.insert(source->get_id(), source);
		sourcesNode = sourcesNode.nextSibling();
		if (source->get_channel_count() == 0) {
			m_silentReadSource = source;
		}
	}
	
	
	QDomNode clipsNode = node.firstChildElement("AudioClips").firstChild();
	
	while(!clipsNode.isNull()) {
		AudioClip* clip = new AudioClip(clipsNode);
		m_clips.insert(clip->get_id(), clip);
		clipsNode = clipsNode.nextSibling();
	}
	
	
	emit sourceAdded();
	
	return 1;
}


int ResourcesManager::remove_clip_from_database(qint64 id)
{
	if ( ! m_clips.contains(id) ) {
		info().critical(tr("ResourcesManager : AudioClip with id %1 not in database,"
				"unable to remove it!").arg(id));
		return -1;
	}
	
	printf("ResourcesManager:: Scheduling AudioClip with id %lld for removal\n", id);
	
	m_deprecatedClips.insert(id, m_clips.value(id));
	
	return 1;
}

int ResourcesManager::undo_remove_clip_from_database(qint64 id)
{
	if ( ! m_clips.contains(id) ) {
		info().critical(tr("ResourcesManager: AudioClip with id %1 not in database,"
				"unable to UNDO removal!").arg(id));
		return -1;
	}
	
	printf("ResourcesManager:: UNDO scheduling AudioClip with id %lld for removal\n", id);
	
	m_deprecatedClips.remove(id);
	
	return 1;
}


int ResourcesManager::get_total_sources()
{
	return m_sources.size();
}


ReadSource* ResourcesManager::create_new_readsource(const QString& dir, const QString& name)
{
	ReadSource* source = new ReadSource(dir, name);
	m_sources.insert(source->get_id(), source);
	source->set_created_by_song( -1 );
	
	source = get_readsource(source->get_id());
	
	if (source->get_error() < 0) {
		m_sources.remove(source->get_id());
		delete source;
		return 0;
	}

	return source;
}


ReadSource* ResourcesManager::create_new_readsource(
		const QString& dir,
		const QString& name,
  		int channelCount,
    		int fileCount,
      		int songId,
		int bitDepth,
  		int rate,
		bool wasRecording)
{
	PENTER;
	
	ReadSource* source = new ReadSource(dir, name, channelCount, fileCount);
	source->set_was_recording(wasRecording);
	m_sources.insert(source->get_id(), source);
	
	if ( bitDepth ) {
		source->set_original_bit_depth( bitDepth );
	} else {
		source->set_original_bit_depth( 16 );
	}
	
	if ( songId ) {
		source->set_created_by_song( songId );
	} else {
		source->set_created_by_song( -1 );
	}
	
	source = get_readsource(source->get_id());
	if (source->get_error() < 0) {
		m_sources.remove(source->get_id());
		delete source;
		return 0;
	}

	return source;
}


ReadSource* ResourcesManager::get_silent_readsource()
{
	if (!m_silentReadSource) {
		m_silentReadSource = new ReadSource();
		m_sources.insert(m_silentReadSource->get_id(), m_silentReadSource);
		m_silentReadSource->set_created_by_song( -1 );
	}
	
	m_silentReadSource = get_readsource(m_silentReadSource->get_id());
	
	return m_silentReadSource;
}


ReadSource * ResourcesManager::get_readsource( qint64 id )
{
	ReadSource* source = m_sources.value(id);
	
	if ( ! source ) {
		PERROR("ReadSource with id %lld is not in my list!", id);
		return 0;
	}
	
	// When the AudioSource is first "get", do a ref counting.
	// If the source allready was ref counted, create a deep copy
	// and do the initialization
	if (source->ref()) {
		PWARN("Creating deep copy of ReadSource: %s", QS_C(source->get_name()));
		source = source->deep_copy();
		source->ref();
	}
		
	if ( source->init() < 0) {
		info().warning( tr("ResourcesManager::  Failed to initialize ReadSource %1")
				.arg(source->get_filename()));
	}
	
	emit stateChanged();
	
	return source;
}

/**
 * 	Mainly used for Importing audio files. If the file with \a fileName 
	allready is in the database, a ReadSource will be returned, else 0.

 * @param fileName The file name of the audio source
 * @return A ReadSource if a ReadSource with the same file name is in the database, else 0
 */
ReadSource* ResourcesManager::get_readsource(const QString& fileName)
{
	foreach(ReadSource* rs, m_sources) {
// 		PMESG("rs filename %s, filename %s", QS_C(rs->get_filename()), QS_C(fileName));
		if (rs->get_filename() == fileName) {
			return get_readsource(rs->get_id()); 
		}
	}
	
	return 0;
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
AudioClip* ResourcesManager::get_clip( qint64 id )
{
	AudioClip* clip = m_clips.value(id);
	 
	if (! clip) {
		return 0;
	}
	
	if (clip->ref()) {
		PMESG("Creating deep copy of Clip %s", QS_C(clip->get_name()));
		clip = clip->create_copy();
		
		// It is NOT possible for 2 clips to have the same ID
		// check for this, since it's absolutely crucial, and 
		// indicates a design error somewhere ! (most likely in 
		// AudioClip::create_copy();
		Q_ASSERT( ! m_clips.contains(clip->get_id()) );
		
		m_clips.insert(clip->get_id(), clip);
		
		// Now that we have created a copy of the audioclip, start
		// the usual get_clip routine again to properly init the clip
		// and other stuff.
		return get_clip(clip->get_id());
	}
	
	ReadSource* source = get_readsource(clip->get_readsource_id());
	
	if (source) {
		clip->set_audio_source(source);
	} else {
		// sometimes the source is set later...
		// maybe that should become the default ?
/*		info().critical(
		     tr("ResourcesManager: AudioClip %1 required ReadSource with ID %2, but I don't have it!!")
			.arg(clip->get_name()).arg(clip->get_readsource_id()));*/
	}
	
	emit stateChanged();
	
	return clip;
}

AudioClip* ResourcesManager::new_audio_clip(const QString& name)
{
	PENTER;
	AudioClip* clip = new AudioClip(name);
	m_clips.insert(clip->get_id(), clip);
	return get_clip(clip->get_id());
}

QList<ReadSource*> ResourcesManager::get_all_audio_sources( ) const
{
	return m_sources.values();
}

QList< AudioClip * > ResourcesManager::get_all_clips() const
{
	return m_clips.values();
}

QList< AudioClip * > ResourcesManager::get_clips_for_source( ReadSource * source ) const
{
	QList<AudioClip*> clips;
	
	foreach(AudioClip* clip, m_clips) {
		if (clip->get_readsource_id() == source->get_id()) {
			clips.append(clip);
		}
	}
	
	return clips;
}



//eof
