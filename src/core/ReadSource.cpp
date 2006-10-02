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

$Id: ReadSource.cpp,v 1.15 2006/10/02 19:04:38 r_sijrier Exp $
*/

#include "ReadSource.h"
#include "PrivateReadSource.h"

#include "Peak.h"
#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"
#include "DiskIO.h"

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


ReadSource::ReadSource(const QString& dir, const QString& name, int channelCount, int fileCount)
	: AudioSource(dir, name), 
	  refcount(0)
{
	  m_channelCount = channelCount;
	  m_fileCount = fileCount;
}


ReadSource::~ReadSource()
{
	PENTERDES;
	foreach(PrivateReadSource* source, m_sources) {
		delete source;
	}
}


int ReadSource::init( )
{
	PENTER;
	
	Q_ASSERT(refcount);
	
	if (m_channelCount == 0) {
		PERROR("ReadSource channel count is 0");
		return -1;
	}
	
	
	QString fileName = m_dir + m_name;
	
	if (m_channelCount == 1 && m_fileCount == 1) {
		if (add_private_source(1, 0, fileName) < 0) {
			return -1;
		}
	} else if (m_channelCount == 2 && m_fileCount == 2) {
		if ((add_private_source(1, 0, fileName + "-ch" + QByteArray::number(0) + ".wav") < 0) || 
		    (add_private_source(1, 1, fileName + "-ch" + QByteArray::number(1) + ".wav") < 0)) {
			return -1;
		}
	} else if (m_channelCount == 2 && m_fileCount == 1) {
		if ((add_private_source(2, 0, fileName) < 0) || 
		    (add_private_source(2, 1, fileName) < 0)) {
			return -1;
		}
	} else {
		PERROR("Unsupported combination of channelcount/filecount (%d/%d)", m_channelCount, m_fileCount);
		return -1;
	}
	
	return 1;
}

int ReadSource::add_private_source(int sourceChannelCount, int channelNumber, const QString& fileName)
{
	PrivateReadSource* source = new PrivateReadSource(this, sourceChannelCount, channelNumber, fileName);
	
	if (source->init() > 0) {
		m_sources.append(source);
	} else {
		PERROR("Failed to initialize a PrivateReadSource (%s)", QS_C(fileName));
		return -1;
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


void ReadSource::sync(audio_sample_t* framebuffer)
{
// 	printf("entering sync\n");
	foreach(PrivateReadSource* source, m_sources) {
		source->sync(framebuffer);
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
	PENTER;
	return refcount++;
}


void ReadSource::set_audio_clip( AudioClip * clip )
{
	foreach(PrivateReadSource* source, m_sources) {
		source->set_audio_clip(clip);
	}
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

