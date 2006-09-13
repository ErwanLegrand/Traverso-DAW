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

$Id: AudioSourceManager.cpp,v 1.10 2006/09/13 12:51:07 r_sijrier Exp $
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
		audioClipsElement.appendChild(clip->get_state(doc));
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


ReadSource * AudioSourceManager::new_readsource( const QString& dir, const QString& name, int songId, int bitDepth, int rate )
{
	PENTER;
	
	ReadSource* newSource = new ReadSource(dir, name);
	
	if (newSource->init() < 0) {
		PWARN("ReadSource init() failed");
		delete newSource;
		return 0;
	}
	
	if ( bitDepth ) {
		newSource->set_original_bit_depth( bitDepth );
	} else {
		newSource->set_original_bit_depth( 16 );
	}
	
	if ( songId ) {
		newSource->set_created_by_song( songId );
	} else {
		newSource->set_created_by_song( -1 );
	}
	
	m_sources.insert(newSource->get_id(), newSource);
	
	newSource->ref();
	
	emit sourceAdded();
		
	return newSource;
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
		PMESG("Creating deep copy of ReadSource: %s", source->get_name().toAscii().data());
		source = source->deep_copy();
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
	
	clip->set_clip_readsource(get_readsource(clip->get_readsource_id()));
	
	return clip;
}

AudioClip* AudioSourceManager::new_audio_clip(const QString& name)
{
	PENTER;
	AudioClip* clip = new AudioClip(name);
	m_clips.insert(clip->get_id(), clip);
	return clip;
}

//eof

