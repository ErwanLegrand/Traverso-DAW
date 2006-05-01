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

$Id: AudioSourceManager.cpp,v 1.1 2006/05/01 21:20:13 r_sijrier Exp $
*/

#include "AudioSourceManager.h"
#include "WriteSource.h"
#include "ReadSource.h"
#include "Information.h"

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
}


QDomNode AudioSourceManager::get_state( QDomDocument doc )
{
	QDomNode sourcesNode = doc.createElement("AudioSources");
	
	foreach(AudioSource* source, sources) {
		sourcesNode.appendChild(source->get_state(doc));
	}
	
	return sourcesNode;
}


int AudioSourceManager::set_state( const QDomNode & node )
{
	QDomNode sourcesNode = node.firstChild();
	
	while(!sourcesNode.isNull()) {
		
		ReadSource* source = new ReadSource(sourcesNode);
		
		if ( source->init() < 0) {
			info().warning( tr( "Failed to initialize ReadSource : %s1").arg(source->get_filename()) );
			delete source;
			continue;
		}
		
		sources.insert(source->get_id(), source);
		sourcesNode = sourcesNode.nextSibling();
	}
	
	return 1;
}


int AudioSourceManager::add(ReadSource* source)
{
	PENTER;
	
	sources.insert(source->get_id(), source);
	
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
	return sources.size();
}


ReadSource* AudioSourceManager::get_source(qint64 id)
{
	return sources.value(id);
}


ReadSource* AudioSourceManager::get_source(QString fileName, int channel)
{
	foreach(ReadSource* as, sources) {
		if ((as->get_filename() == fileName) && (as->get_channel() == channel)) {
			return as;
		}
	}
	
	return (ReadSource*) 0;
}


ReadSource * AudioSourceManager::new_readsource( QString dir, QString name, uint channel, int songId, int bitDepth, int rate )
{
	ReadSource* newSource = new ReadSource(channel, dir, name);
	
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
	
	add( newSource );
		
	return newSource;
}

ReadSource * AudioSourceManager::get_readsource( qint64 id )
{
	ReadSource* source = sources.value(id);
	
	if ( ! source ) {
		return 0;
	}
	
	if (source->ref()) {
		source = source->deep_copy();
	}
	
	if ( source->init() < 0) {
		info().warning( tr( "Failed to initialize ReadSource : %1").arg(source->get_filename()) );
		delete source;
		source = 0;
	}
	
	return source;
}

//eof

