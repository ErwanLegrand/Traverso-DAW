/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: AudioSourcesList.cpp,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#include "AudioSourcesList.h"
#include "WriteSource.h"
#include "ReadSource.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AudioSourcesList::AudioSourcesList()
{
        PENTERCONS;
}


AudioSourcesList::~AudioSourcesList()
{
        PENTERDES;
/*        foreach(AudioSource* source, sources)
	        delete source;*/
}


QDomNode AudioSourcesList::get_state( QDomDocument doc )
{
        QDomNode sourcesNode = doc.createElement("AudioSources");
        foreach(AudioSource* source, sources) {
                sourcesNode.appendChild(source->get_state(doc));
        }
        return sourcesNode;
}


int AudioSourcesList::set_state( const QDomNode & node )
{
        QDomNode sourcesNode = node.firstChild();
        while(!sourcesNode.isNull()) {
                ReadSource* source = new ReadSource(sourcesNode);
                sources.insert(source->get_id(), source);
                sourcesNode = sourcesNode.nextSibling();
        }
        return 1;
}


int AudioSourcesList::add(AudioSource* source, int channel)
{
        PENTER;
/*        QString fileName = source->get_filename();
        foreach(AudioSource* as, sources) {
                if ((as->get_filename() == fileName) && (as->get_channel() == channel)) {
                        PWARN("AudioSource with fileName %s already exists", fileName.toAscii().data());
                        return -1;
                }
        }*/
        sources.insert(source->get_id(), source);
        return 1;
}


int AudioSourcesList::remove
        (AudioSource* )
{
        PENTER;
        return -1;
}


int AudioSourcesList::get_total_sources()
{
        return sources.size();
}


AudioSource* AudioSourcesList::get_source(qint64 id)
{
        return sources.value(id);
}


AudioSource* AudioSourcesList::get_source(QString fileName, int channel)
{
        foreach(AudioSource* as, sources) {
                if ((as->get_filename() == fileName) && (as->get_channel() == channel)) {
                        return as;
                }
        }
        return (AudioSource*) 0;
}


AudioSource * AudioSourcesList::new_audio_source( int rate, int bitDepth, int songId, QString name, uint channel, QString dir )
{
        /*	// Create an AudioSource for channel:
        	WriteSource* source = new WriteSource(channel);
        	
        	// Set source member values
        	source->set_dir( dir );
        	source->set_name( name );
        	source->set_original_bit_depth( bitDepth );
        	source->set_created_by_song( songId );
        	source->set_sample_rate( rate );
        	
        	// This function is called for recording purposes, so open the new source for writing:
        	if (source->init_recording() < 0)
        		{
        		// Failed to create the new file, return 0
        		delete source;
        		return (AudioSource*) 0;
        		}
        	
        	sources.insert(source->get_id(), source);
        	return source;*/
}

//eof

