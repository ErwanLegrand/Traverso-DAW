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
#include "AbstractAudioReader.h"
#include "ResampleAudioReader.h"

#include "Peak.h"
#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"
#include "DiskIO.h"
#include "Utils.h"
#include "Song.h"
#include "AudioDevice.h"
#include <QFile>
#include "Config.h"

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
{
	private_init();
	
	Project* project = pm().get_project();
	
	// Check if the audiofile exists in our project audiosources dir
	// and give it priority over the dir as given by the project.tpf file
	// This makes it possible to move project directories without Traverso being
	// unable to find it's audiosources!
	if ( QFile::exists(project->get_root_dir() + "/audiosources/" + m_name) || 
	     QFile::exists(project->get_root_dir() + "/audiosources/" + m_name + "-ch0.wav") ) {
		set_dir(project->get_root_dir() + "/audiosources/");
	}
	
	m_silent = (m_channelCount == 0);
}	

// constructor for file import
ReadSource::ReadSource(const QString& dir, const QString& name)
	: AudioSource(dir, name)
{
	private_init();
	
	AbstractAudioReader* reader = AbstractAudioReader::create_audio_reader(m_fileName);

	if (reader) {
		m_channelCount = reader->get_num_channels();
		delete reader;
	} else {
		m_channelCount = 0;
	}

	m_silent = false;
}


// Constructor for recorded audio.
ReadSource::ReadSource(const QString& dir, const QString& name, int channelCount)
	: AudioSource(dir, name)
{
	private_init();
	
	m_channelCount = channelCount;
	m_silent = false;
	m_name = name  + "-" + QString::number(m_id);
	m_fileName = m_dir + m_name;
	m_length = 0;
	m_rate = pm().get_project()->get_rate();
	m_wasRecording = true;
	m_shortName = m_name.left(m_name.length() - 20);
}


// Constructor for silent clips
ReadSource::ReadSource()
	: AudioSource("", tr("Silence"))
{
	private_init();
	
	m_channelCount = 0;
	m_silent = true;
}


void ReadSource::private_init()
{
	m_refcount = 0;
	m_error = 0;
	m_clip = 0;
	m_audioReader = 0;
	m_bufferstatus = 0;
}


ReadSource::~ReadSource()
{
	PENTERDES;
	for(int i=0; i<m_buffers.size(); ++i) {
		delete m_buffers.at(i);
	}
	
	if (m_audioReader) {
		delete m_audioReader;
	}
	
	if (m_bufferstatus) {
		delete m_bufferstatus;
	}
}


int ReadSource::init( )
{
	PENTER;
	
	Q_ASSERT(m_refcount);
	
	Project* project = pm().get_project();
	
	// Fake the samplerate, until it's set by an AudioReader!
	m_rate = project->get_rate();
	
	if (m_silent) {
		m_length = INT_MAX;
		m_channelCount = 0;
		m_origBitDepth = project->get_bitdepth();
		return 1;
	}
	
	if (m_channelCount == 0) {
		PERROR("ReadSource channel count is 0");
		return (m_error = INVALID_CHANNEL_COUNT);
	}
	
	
	m_rbFileReadPos = 0;
	m_rbRelativeFileReadPos = 0;
	m_rbReady = 0;
	m_needSync = 0;
	m_syncInProgress = 0;
	m_bufferUnderRunDetected = m_wasActivated = 0;
	
	bool useResampling = config().get_property("Conversion", "DynamicResampling", false).toBool();
	
	if (useResampling) {
		int converter_type;
		converter_type = config().get_property("Conversion", "RTResamplingConverterType", 2).toInt();
		// There should be another config option for ConverterType to use for export (higher quality)
		//converter_type = config().get_property("Conversion", "ExportResamplingConverterType", 0).toInt();
		m_audioReader = new ResampleAudioReader(m_fileName, converter_type);
		if (m_audioReader->get_num_channels()) {
			output_rate_changed();
		}
		else {
			delete m_audioReader;
			m_audioReader = 0;
		}
	}
	else {
		m_audioReader = AbstractAudioReader::create_audio_reader(m_fileName);
	}
	
	if (m_audioReader == 0) {
		return COULD_NOT_OPEN_FILE;
	}
	
	if (m_channelCount > m_audioReader->get_num_channels()) {
		PERROR("ReadAudioSource: file only contains %d channels; %d is invalid as a channel number", m_audioReader->get_num_channels(), m_channelCount);
		delete m_audioReader;
		m_audioReader = 0;
		return INVALID_CHANNEL_COUNT;
	}

	if (m_audioReader->get_num_channels() == 0) {
		PERROR("ReadAudioSource: not a valid channel count: %d", m_audioReader->get_num_channels());
		delete m_audioReader;
		m_audioReader = 0;
		return ZERO_CHANNELS;
	}

	m_length = m_audioReader->get_length();
	m_rate = m_audioReader->get_file_rate();
	
	m_bufferstatus = new BufferStatus;
	
	return 1;
}


void ReadSource::output_rate_changed()
{
	((ResampleAudioReader*)m_audioReader)->set_output_rate(audiodevice().get_sample_rate());
}


int ReadSource::file_read(DecodeBuffer* buffer, nframes_t start, nframes_t cnt) const
{
#if defined (profile)
	trav_time_t starttime = get_microseconds();
#endif
	if (m_audioReader->get_num_channels() == 1) {
		nframes_t result = m_audioReader->read_from(buffer, start, cnt);
#if defined (profile)
		int processtime = (int) (get_microseconds() - starttime);
		if (processtime > 40000)
			printf("Process time for %s: %d useconds\n\n", QS_C(m_fileName), processtime);
#endif
		return (int)result;
	}

	// The readbuffer 'assumes' that there is max 2 channels...
	Q_ASSERT(m_audioReader->get_num_channels() <= 2);
	
	nframes_t nread = m_audioReader->read_from(buffer, start, cnt);
#if defined (profile)
	int processtime = (int) (get_microseconds() - starttime);
	if (processtime > 40000)
		printf("Process time for %s: %d useconds\n\n", QS_C(m_fileName), processtime);
#endif
	
	return nread;
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
}

nframes_t ReadSource::get_nframes( ) const
{
	return m_length;
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
	
	emit stateChanged();
	
	return 1;
}




int ReadSource::rb_read(audio_sample_t** dst, nframes_t start, nframes_t count)
{

	if ( ! m_rbReady ) {
// 		printf("ringbuffer not ready\n");
		return 0;
	}

	if (start != m_rbRelativeFileReadPos) {
		int available = m_buffers.at(0)->read_space();
// 		printf("start %d, m_rbFileReadPos %d\n", start, m_rbRelativeFileReadPos);
		if ( (start > m_rbRelativeFileReadPos) && (m_rbRelativeFileReadPos + available) > (start + count)) {
			int advance = start - m_rbRelativeFileReadPos;
			if (available < advance) {
				printf("available < advance !!!!!!!\n");
			}
			for (int i=m_buffers.size()-1; i>=0; --i) {
				m_buffers.at(i)->increment_read_ptr(advance);
			}
			m_rbRelativeFileReadPos += advance;
		} else {
			start_resync(start + (m_clip->get_track_start_frame() + m_clip->get_source_start_frame()));
			return 0;
		}
	}

	nframes_t readcount = 0;
	
	for (int chan=0; chan<m_channelCount; ++chan) {
		
		readcount = m_buffers.at(chan)->read(dst[chan], count);

		if (readcount != count) {
		// Hmm, not sure what to do in this case....
		}
		
	}

	m_rbRelativeFileReadPos += readcount;
	
	return readcount;
}


int ReadSource::rb_file_read(DecodeBuffer* buffer, nframes_t cnt)
{
	int readFrames = file_read(buffer, m_rbFileReadPos, cnt);
	m_rbFileReadPos += readFrames;

	return readFrames;
}


void ReadSource::rb_seek_to_file_position( nframes_t position )
{
	Q_ASSERT(m_clip);
	
// 	printf("rb_seek_to_file_position:: seeking to %d\n", position);
	
	// calculate position relative to the file!
	long fileposition = position - (m_clip->get_track_start_frame() + m_clip->get_source_start_frame());
	
	if ((long)m_rbFileReadPos == fileposition) {
// 		printf("ringbuffer allready at position %d\n", position);
		return;
	}

	// check if the clip's start position is within the range
	// if not, fill the buffer from the earliest point this clip
	// will come into play.
	if (fileposition < 0) {
// 		printf("not seeking to %ld, but too %d\n\n", fileposition,m_clip->get_source_start_frame()); 
		// Song's start from 0, this makes a period start from
		// 0 - 1023 for example, the nframes is 1024!
		// Setting a songs new position is on 1024, and NOT 
		// 1023.. Hmm, something isn't correct here, but at least substract 1
		// to make this thing work!
		fileposition = m_clip->get_source_start_frame() - 1;
	}
	
// 	printf("rb_seek_to_file_position:: seeking to relative pos: %d\n", fileposition);
	for (int i=0; i<m_buffers.size(); ++i) {
		m_buffers.at(i)->reset();
	}
	m_rbFileReadPos = fileposition;
	m_rbRelativeFileReadPos = fileposition;
}

void ReadSource::process_ringbuffer(DecodeBuffer* buffer, bool seeking)
{
	// Do nothing if we passed the lenght of the AudioFile.
	if (m_rbFileReadPos >= m_length) {
// 		printf("returning, m_rbFileReadPos > m_length! (%d >  %d)\n", m_rbFileReadPos, m_source->m_length);
		if (m_syncInProgress) {
			finish_resync();
		}
		return;
	}
	
	// Calculate the number of samples we can write into the buffer
	int writeSpace = m_buffers.at(0)->write_space();

	// The amount of chunks which can be 'read'
	int chunkCount = (int)(writeSpace / m_chunkSize);
	
	int toRead = m_chunkSize;
	
	if (seeking) {
		toRead = writeSpace;
// 		printf("doing a full seek buffer fill\n");
	} else if (m_syncInProgress) {
		// Currently, we fill the buffer completely.
		// For some reason, filling it with 1/4 at a time
		// doesn't fill it consitently, and thus giving audible artifacts.
		/*		toRead = m_chunkSize * 2;*/
		toRead = writeSpace;
	} else if (chunkCount == 0) {
		// If we are nearing the end of the source file it could be possible
		// we only need to read the last samples which is smaller in size then 
		// chunksize. If so, set toRead to m_source->m_length - rbFileReasPos
		if ( (int) (m_length - m_rbFileReadPos) <= m_chunkSize) {
			toRead = m_length - m_rbFileReadPos;
		} else {
			printf("ReadSource:: chunkCount == 0, but not at end of file, this shouldn't happen!!\n");
			return;
		}
	}
	
	// Read in the samples from source
	nframes_t toWrite = rb_file_read(buffer, toRead);

	// and write it to the ringbuffer
	for (int i=m_buffers.size()-1; i>=0; --i) {
		m_buffers.at(i)->write(buffer->destination[i], toWrite);
	}
}


void ReadSource::recover_from_buffer_underrun(nframes_t position)
{
// 	printf("buffer underrun detected!\n");
	m_bufferUnderRunDetected = 1;
	start_resync(position);
}

void ReadSource::start_resync( nframes_t position )
{
// 	printf("starting resync!\n");
	m_syncPos = position;
	m_rbReady = 0;
	m_needSync = 1;
}

void ReadSource::finish_resync()
{
// 	printf("sync finished\n");
	m_needSync = 0;
	m_bufferUnderRunDetected = 0;
	m_rbReady = 1;
	m_syncInProgress = 0;
}

void ReadSource::sync(DecodeBuffer* buffer)
{
	PENTER2;
	if (!m_needSync) {
		return;
	}
	
	if (!m_syncInProgress) {
		rb_seek_to_file_position(m_syncPos);
		m_syncInProgress = 1;
	}
	
	// Currently, we fill the buffer completely.
	// For some reason, filling it with 1/4 at a time
	// doesn't fill it consitently, and thus giving audible artifacts.
	process_ringbuffer(buffer);
	
	if (m_buffers.at(0)->write_space() == 0) {
		finish_resync();
	}
	
//         PWARN("Resyncing ringbuffer finished");
}



void ReadSource::prepare_buffer( )
{
	PENTER;
	
	Q_ASSERT(m_clip);

	float size = config().get_property("Hardware", "readbuffersize", 1.0).toDouble();

	m_bufferSize = (int) (size * audiodevice().get_sample_rate());

	m_chunkSize = m_bufferSize / DiskIO::bufferdividefactor;

	for (int i=0; i<m_channelCount; ++i) {
		m_buffers.append(new RingBufferNPT<float>(m_bufferSize));
	}

	start_resync(m_clip->get_song()->get_working_frame());
}

BufferStatus* ReadSource::get_buffer_status()
{
	int freespace = m_buffers.at(0)->write_space();

	if (m_rbFileReadPos >= m_length) {
		m_bufferstatus->fillStatus =  100;
		freespace = 0;
	} else {
		m_bufferstatus->fillStatus = (int) (((float)freespace / m_bufferSize) * 100);
	}
	
	m_bufferstatus->bufferUnderRun = m_bufferUnderRunDetected;
	m_bufferstatus->needSync = m_needSync;

	m_bufferstatus->priority = (int) (freespace / m_chunkSize);
	
	return m_bufferstatus;
}

void ReadSource::set_active(bool active)
{
	if (m_active == active)
		return;

	if (active) {
		m_active = 1;
// 		m_wasActivated = 1;
// 		printf("setting private readsource %s to active\n", QS_C(m_fileName));
	} else {
// 		printf("setting private readsource %s to IN-active\n", QS_C(m_fileName));
		m_active = 0;
	}
}


//eof

