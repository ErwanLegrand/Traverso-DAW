/*
Copyright (C) 2006 Remon Sijrier 

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

$Id: AudioSourceManager.cpp,v 1.12 2006/10/18 12:03:16 r_sijrier Exp $
*/

#include "AudioSourceManager.h"
#include "WriteSource.h"
#include "ReadSource.h"
#include "Information.h"
#include "AudioClip.h"
#include "Utils.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AudioSourceManager::AudioSourceManager()
{
	PENTERCONS;
}


AudioSourceManager::~AudioSourceManager()
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


QDomNode AudioSourceManager::get_state( QDomDocument doc )
{

	QDomElement asmNode = doc.createElement("AudioSourcesManager");
	
	QDomElement audioSourcesElement = doc.createElement("AudioSources");
	
	foreach(AudioSource* source, m_sources) {
		audioSourcesElement.appendChild(source->get_state(doc));
	}
	
	asmNode.appendChild(audioSourcesElement);
	
	
	QDomElement audioClipsElement = doc.createElement("AudioClips");
	
	foreach(AudioClip* clip, m_clips) {
// 		PWARN("Getting state of clip %s", QS_C(clip->get_name()));
		if (clip->get_ref_count()) {
			audioClipsElement.appendChild(clip->get_state(doc));
		} else {
			audioClipsElement.appendChild(clip->m_domNode);
		}
	}
	
	asmNode.appendChild(audioClipsElement);
	
	return asmNode;
}


int AudioSourceManager::set_state( const QDomNode & node )
{
	QDomNode sourcesNode = node.firstChildElement("AudioSources").firstChild();
	
	while(!sourcesNode.isNull()) {
		ReadSource* source = new ReadSource(sourcesNode);
		m_sources.insert(source->get_id(), source);
		sourcesNode = sourcesNode.nextSibling();
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


int AudioSourceManager::remove(AudioSource* )
{
	PENTER;
	
	emit sourceRemoved();
	
	return -1;
}


int AudioSourceManager::get_total_sources()
{
	return m_sources.size();
}


ReadSource* AudioSourceManager::new_readsource(const QString& dir, const QString& name)
{
	ReadSource* source = new ReadSource(dir, name);
	m_sources.insert(source->get_id(), source);
	source->set_created_by_song( -1 );
	return get_readsource(source->get_id());
}


ReadSource* AudioSourceManager::new_readsource( const QString& dir, const QString& name,  int channelCount, int fileCount, int songId, int bitDepth, int rate )
{
	PENTER;
	
	ReadSource* source = new ReadSource(dir, name, channelCount, fileCount);
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
	

	return get_readsource(source->get_id());
}

ReadSource * AudioSourceManager::get_readsource( qint64 id )
{
	ReadSource* source = m_sources.value(id);
	
	if ( ! source ) {
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
		info().warning( tr( "Failed to initialize ReadSource : %1").arg(source->get_filename()) );
		delete source;
		source = 0;
	}
	
	return source;
}

ReadSource* AudioSourceManager::get_readsource(const QString& fileName)
{
	foreach(ReadSource* rs, m_sources) {
// 		PMESG("rs filename %s, filename %s", QS_C(rs->get_filename()), QS_C(fileName));
		if (rs->get_filename() == fileName) {
			return get_readsource(rs->get_id()); 
		}
	}
	
	return 0;
}

AudioClip* AudioSourceManager::get_clip( qint64 id )
{
	AudioClip* clip = m_clips.value(id);
	 
	if (! clip) {
		return 0;
	}
	
	if (clip->ref()) {
		PWARN("Creating deep copy of Clip %s", QS_C(clip->get_name()));
		clip = clip->create_copy();
	}
	
	ReadSource* source = get_readsource(clip->get_readsource_id());
	
	if (source) {
		clip->set_audio_source(source);
	}
	
	return clip;
}

AudioClip* AudioSourceManager::new_audio_clip(const QString& name)
{
	PENTER;
	AudioClip* clip = new AudioClip(name);
	m_clips.insert(clip->get_id(), clip);
	return get_clip(clip->get_id());
}

QList<ReadSource*> AudioSourceManager::get_all_audio_sources( ) const
{
	return m_sources.values();
}

QList< AudioClip * > AudioSourceManager::get_clips_for_source( ReadSource * source ) const
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

