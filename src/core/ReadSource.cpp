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

$Id: ReadSource.cpp,v 1.12 2006/09/13 12:51:07 r_sijrier Exp $
*/

#include "ReadSource.h"
#include "PrivateReadSource.h"

#include "Peak.h"
#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"

#include <QFile>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


// This constructor is called for existing (recorded/imported) audio sources
ReadSource::ReadSource(const QDomNode node)
	: AudioSource(node),  m_sources(), refcount(0)
{
	Project* project = pm().get_project();
	
	// Check if the audiofile exists in our project audiosources dir
	// and give it priority over the dir as given by the project.traverso file
	// This makes it possible to move project directories without Traverso being
	// unable to find it's audiosources!
	if (QFile::exists(project->get_root_dir() + "/audiosources/" + m_name)) {
		set_dir(project->get_root_dir() + "/audiosources/");
	}
}	


ReadSource::ReadSource(const QString& dir, const QString& name)
	: AudioSource(dir, name), refcount(0)
{
	SNDFILE* sf;
	SF_INFO  sfinfo;
	m_channelCount = 0;
	
	if ((sf = sf_open (QS_C(m_fileName), SFM_READ, &sfinfo)) != 0) {
		m_channelCount = sfinfo.channels;
		sf_close(sf);
	}

	m_fileCount = 1;
}


ReadSource::~ReadSource()
{
	PENTERDES;
}


int ReadSource::init( )
{
	PENTER;
	
	Q_ASSERT(refcount);
	
	if (m_channelCount == 0) {
		PERROR("ReadSource channel count is 0");
		return -1;
	}
	
	for (int i=0; i<m_channelCount; ++i) {
		QString fileName = m_dir + m_name;
		if (m_fileCount > 1) {
			fileName.append("-ch" + QByteArray::number(i) + ".wav");
		}
		
		int channel = m_fileCount > 1 ? 0 : 1;
		
		PrivateReadSource* source = new PrivateReadSource(this, channel, i, fileName);
		
		if (source->init() > 0) {
			m_sources.append(source);
		} else {
			PERROR("Failed to initialize a PrivateReadSource (%s)", QS_C(fileName));
			return -1;
		}
	}
	
	return 1;
}


int ReadSource::file_read (int channel, audio_sample_t* dst, nframes_t start, nframes_t cnt) const
{
	Q_ASSERT(channel < m_sources.size());
	return m_sources.at(channel)->file_read(dst, start, cnt);
}


int ReadSource::rb_read(int channel, audio_sample_t* dst, nframes_t start, nframes_t cnt)
{
	Q_ASSERT(channel < m_sources.size());
	return m_sources.at(channel)->rb_read(dst, start, cnt);

}


void ReadSource::rb_seek_to_file_position( nframes_t position )
{
	foreach(PrivateReadSource* source, m_sources) {
		source->rb_seek_to_file_position(position);
	}
}


int ReadSource::process_ringbuffer( audio_sample_t * framebuffer )
{
	foreach(PrivateReadSource* source, m_sources) {
		source->process_ringbuffer(framebuffer);
	}
	
	return 0;
	
}


void ReadSource::sync( )
{
	foreach(PrivateReadSource* source, m_sources) {
		source->sync();
	}
}


void ReadSource::set_rb_ready( bool ready )
{
	foreach(PrivateReadSource* source, m_sources) {
		source->set_rb_ready(ready);
	}
}


void ReadSource::set_active( )
{
	PENTER2;
	m_active = true;
}


void ReadSource::set_inactive( )
{
	PENTER2;
	m_active = false;
}


ReadSource * ReadSource::deep_copy( )
{
	PENTER;
	
	QDomDocument doc("ReadSource");
	QDomNode rsnode = get_state(doc);
	ReadSource* source = new ReadSource(rsnode);
	return source;
}


int ReadSource::ref( )
{
	return refcount++;
}


void ReadSource::set_audio_clip( AudioClip * clip )
{
	foreach(PrivateReadSource* source, m_sources) {
		source->set_audio_clip(clip);
	}
}

bool ReadSource::need_sync( )
{
	foreach(PrivateReadSource* source, m_sources) {
		if (source->need_sync())
			return true;
	}
	
	return false;
}

Peak * ReadSource::get_peak( int channel )
{
	Q_ASSERT(channel < m_sources.size());
	return m_sources.at(channel)->get_peak();
}

void ReadSource::prepare_buffer( )
{
	foreach(PrivateReadSource* source, m_sources) {
		source->prepare_buffer();
	}	
}

nframes_t ReadSource::get_nframes( ) const
{
	return m_length;
}


//eof

