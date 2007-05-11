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

#include "ReadSource.h"
#include "MonoReader.h"

#include "Peak.h"
#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"
#include "DiskIO.h"
#include "Utils.h"
#include <QFile>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/**
 *	\class ReadSource
	\brief A class for (buffered) reading of audio files.
 */

// This constructor is called for existing (recorded/imported) audio sources
ReadSource::ReadSource(const QDomNode node)
	: AudioSource(node)
	, m_refcount(0)
	, m_unrefcount(0)
	, m_error(0)
	, m_clip(0)
{
	Project* project = pm().get_project();
	
	// Check if the audiofile exists in our project audiosources dir
	// and give it priority over the dir as given by the project.tpf file
	// This makes it possible to move project directories without Traverso being
	// unable to find it's audiosources!
	if (QFile::exists(project->get_root_dir() + "/audiosources/" + m_name)) {
		set_dir(project->get_root_dir() + "/audiosources/");
	}
	
	m_silent = (m_channelCount == 0);
}	

// constructor for file import
ReadSource::ReadSource(const QString& dir, const QString& name)
	: AudioSource(dir, name)
	, m_refcount(0)
	, m_unrefcount(0)
	, m_error(0)
	, m_clip(0)
{
	SNDFILE* sf;
	SF_INFO  sfinfo;
	m_channelCount = 0;
	
	if ((sf = sf_open (QS_C(m_fileName), SFM_READ, &sfinfo)) != 0) {
		m_channelCount = sfinfo.channels;
		sf_close(sf);
	}

	m_fileCount = 1;
	m_silent = false;
}


// Constructur for recorded audio.
ReadSource::ReadSource(const QString& dir, const QString& name, int channelCount, int fileCount)
	: AudioSource(dir, name)
	, m_refcount(0)
	, m_unrefcount(0)
	, m_error(0)
	, m_clip(0)
{
	m_channelCount = channelCount;
	m_fileCount = fileCount;
	m_silent = false;
	m_name = name  + "-" + QString::number(m_id);
	m_fileName = m_dir + m_name;
}


ReadSource::ReadSource()
	: AudioSource("", tr("Silence"))
	, m_refcount(0)
	, m_unrefcount(0)
	, m_error(0)
	, m_clip(0)
{
	m_channelCount = 0;
	m_fileCount = 0;
	m_silent = true;
}


ReadSource::~ReadSource()
{
	PENTERDES;
	foreach(MonoReader* source, m_sources) {
		delete source;
	}
}


int ReadSource::init( )
{
	PENTER;
	
	Q_ASSERT(m_refcount);
	
	Project* project = pm().get_project();
	
	// Fake the samplerate, until it's set by a MonoReader!
	m_rate = project->get_rate();
	
	if (m_silent) {
		m_length = 2147483648L; // 2^31
		m_channelCount = 0;
		m_origBitDepth = project->get_bitdepth();
		return 1;
	}
	
	if (m_channelCount == 0) {
		PERROR("ReadSource channel count is 0");
		return (m_error = INVALID_CHANNEL_COUNT);
	}
	
	
	QString fileName = m_dir + m_name;
	
	if (m_wasRecording) {
		if (m_channelCount == 1 && m_fileCount == 1) {
			if ( (m_error = add_mono_reader(1, 0, fileName + "-ch" + QByteArray::number(0) + ".wav")) < 0) {
				return m_error;
			}
		} else if (m_channelCount == 2 && m_fileCount == 2) {
			if (((m_error = add_mono_reader(1, 0, fileName + "-ch" + QByteArray::number(0) + ".wav") < 0)) || 
				  ((m_error = add_mono_reader(1, 1, fileName + "-ch" + QByteArray::number(1) + ".wav")) < 0)) {
				return m_error;
			}
		} else {
			PERROR("WasRecording section: Unsupported combination of channelcount/filecount (%d/%d)", m_channelCount, m_fileCount);
			return (m_error = CHANNELCOUNT_FILECOUNT_MISMATCH);
		}
	} else {
	
		if (m_channelCount == 1 && m_fileCount == 1) {
			if ((m_error = add_mono_reader(1, 0, fileName)) < 0) {
				return m_error;
			}
		} else if (m_channelCount == 2 && m_fileCount == 1) {
			if (((m_error = add_mono_reader(2, 0, fileName)) < 0) || ((m_error = add_mono_reader(2, 1, fileName)) < 0)) {
				return m_error;
			}
		} else {
			PERROR("Unsupported combination of channelcount/filecount (%d/%d)", m_channelCount, m_fileCount);
			return (m_error = CHANNELCOUNT_FILECOUNT_MISMATCH);
		}
	}
	
 	
	return 1;
}

int ReadSource::add_mono_reader(int sourceChannelCount, int channelNumber, const QString& fileName)
{
	int result = 1;
	
	MonoReader* source = new MonoReader(this, sourceChannelCount, channelNumber, fileName);
	
	if ( (result = source->init()) > 0) {
		m_sources.append(source);
	} else {
		PERROR("Failed to initialize a MonoReader (%s)", QS_C(fileName));
		delete source;
		return result;
	}
	
	return result;
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


void ReadSource::set_active(bool active)
{
	PENTER2;
	foreach(MonoReader* source, m_sources) {
		source->set_active(active);
	}
}


ReadSource * ReadSource::deep_copy( )
{
	PENTER;
	
	QDomDocument doc("ReadSource");
	QDomNode rsnode = get_state(doc);
	ReadSource* source = new ReadSource(rsnode);
	return source;
}


void ReadSource::set_audio_clip( AudioClip * clip )
{
	PENTER;
	Q_ASSERT(clip);
	m_clip = clip;
	foreach(MonoReader* source, m_sources) {
		source->set_audio_clip(clip);
	}
}


Peak * ReadSource::get_peak( int channel )
{
	Q_ASSERT(channel < m_sources.size());
	return m_sources.at(channel)->get_peak();
}

nframes_t ReadSource::get_nframes( ) const
{
	return m_length;
}

void ReadSource::set_was_recording(bool wasRecording)
{
	m_wasRecording = wasRecording;
	m_shortName = m_name.left(m_name.length() - 20);
}

int ReadSource::set_file(const QString & filename)
{
	PENTER;
	
	Q_ASSERT(m_clip);

	m_error = 0;
	
	int splitpoint = filename.lastIndexOf("/") + 1;
	int length = filename.length();
	
	QString dir = filename.left(splitpoint - 1) + "/";
	QString name = filename.right(length - splitpoint);
		
	set_dir(dir);
	set_name(name);
	
	if (init() < 0) {
		return -1;
	}
	
	set_audio_clip(m_clip);
	
	return 1;
}

//eof
