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

$Id: PrivateReadSource.cpp,v 1.8 2007/02/15 21:15:11 r_sijrier Exp $
*/


#include "PrivateReadSource.h"

#include "Peak.h"
#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"
#include "ReadSource.h"
#include "Utils.h"
#include <Config.h>
#include <AudioDevice.h>
#include <DiskIO.h>
#include <Song.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



PrivateReadSource::PrivateReadSource(ReadSource* source, int sourceChannelCount, int channelNumber, const QString& fileName)
	: AudioSource(),
	  m_source(source),
	  m_buffer(0),
	  m_peak(0),
	  m_sf(0),
	  m_sourceChannelCount(sourceChannelCount), 
	  m_channelNumber(channelNumber), 
	  m_fileName(fileName)
{
	PENTERCONS;
}

PrivateReadSource::~PrivateReadSource()
{
	PENTERDES;
	if (m_buffer) {
		delete m_buffer;
	}
	if (m_peak) {
		delete m_peak;
	}
	if (m_sf) {
		if (sf_close (m_sf)) {
			qWarning("sf_close returned an error!");
		}
	}
}


int PrivateReadSource::init( )
{
	PENTER;
	
	Q_ASSERT(m_sf == 0);
	
	m_rbFileReadPos = 0;
	m_rbRelativeFileReadPos = 0;
	m_rbReady = 0;
	m_needSync = 0;
	m_syncInProgress = 0;
	m_clip = 0;
	m_bufferUnderRunDetected = m_wasActivated = 0;
	m_isCompressedFile = false;

	/* although libsndfile says we don't need to set this,
	valgrind and source code shows us that we do.
	Really? Look it up !
	*/
	memset (&m_sfinfo, 0, sizeof(m_sfinfo));


	if ((m_sf = sf_open (QS_C(m_fileName), SFM_READ, &m_sfinfo)) == 0) {
		PERROR("Couldn't open soundfile (%s)", QS_C(m_fileName));
		return -1;
	}

	if (m_sourceChannelCount > m_sfinfo.channels) {
		PERROR("ReadAudioSource: file only contains %d channels; %d is invalid as a channel number", m_sfinfo.channels, m_sourceChannelCount);
		sf_close (m_sf);
		m_sf = 0;
		return -1;
	}

	if (m_sfinfo.channels == 0) {
		PERROR("ReadAudioSource: not a valid channel count: %d", m_sfinfo.channels);
		sf_close (m_sf);
		m_sf = 0;
		return -1;
	}

	m_source->m_length = m_sfinfo.frames;
	m_length = m_sfinfo.frames;
	m_source->m_rate = m_sfinfo.samplerate;

	if ( (m_sfinfo.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_FLAC ) {
		m_isCompressedFile = true;
	}
	
	m_peak = new Peak(m_source, m_channelNumber);
	
	return 1;
}


int PrivateReadSource::file_read (audio_sample_t* dst, nframes_t start, nframes_t cnt) const
{
// 	PWARN("file_read");
	// this equals checking if init() is called!
	Q_ASSERT(m_sf);
	
	if (start >= m_source->m_length) {
		return 0;
	}
	
	if (sf_seek (m_sf, (off_t) start, SEEK_SET) < 0) {
		char errbuf[256];
		sf_error_str (0, errbuf, sizeof (errbuf) - 1);
		PERROR("ReadAudioSource: could not seek to frame %d within %s (%s)", start, QS_C(m_fileName), errbuf);
		return 0;
	}
	

	if (m_sfinfo.channels == 1) {
		return sf_read_float (m_sf, dst, cnt);
	}

	float *ptr;
	int real_cnt = cnt * m_sfinfo.channels;


	audio_sample_t readbuffer[real_cnt];

	int nread = sf_read_float (m_sf, readbuffer, real_cnt);
	ptr = readbuffer + m_channelNumber;
	nread /= m_sfinfo.channels;

	/* stride through the interleaved data */

	for (int32_t n = 0; n < nread; ++n) {
		dst[n] = *ptr;
		ptr += m_sfinfo.channels;
	}


	return nread;
}


int PrivateReadSource::rb_read(audio_sample_t* dst, nframes_t start, nframes_t count)
{

	if ( ! m_rbReady ) {
// 		printf("ringbuffer not ready\n");
		return 0;
	}


	if (start != m_rbRelativeFileReadPos) {
		int available = m_buffer->read_space();
		if ( (start > m_rbRelativeFileReadPos) && ( (m_rbRelativeFileReadPos + available) > (start + count) ) ) {
			int advance = start - m_rbRelativeFileReadPos;
			if (available < advance)
				printf("available < advance !!!!!!!\n");
			m_buffer->increment_read_ptr(advance);
			m_rbRelativeFileReadPos += advance;
		} else {
/*			if (m_wasActivated) {
				m_wasActivated = 0;*/
				start_resync(start);
/*			} else {
				recover_from_buffer_underrun(start);
			}*/
			return 0;
		}
	}

	nframes_t readcount = m_buffer->read(dst, count);

	if (readcount != count) {
		// Hmm, not sure what to do in this case....
	}

	m_rbRelativeFileReadPos += readcount;

	return readcount;
}


int PrivateReadSource::rb_file_read( audio_sample_t * dst, nframes_t cnt )
{
	int readFrames = file_read( dst, m_rbFileReadPos, cnt);
	m_rbFileReadPos += readFrames;

	return readFrames;
}


void PrivateReadSource::rb_seek_to_file_position( nframes_t position )
{
	Q_ASSERT(m_clip);
	
	if (m_rbFileReadPos == position) {
		PMESG("ringbuffer allready at position %d", position);
		return;
	}

/*	if (m_needSync) {
		finish_resync();
	}*/
	
	long fileposition = position;
	
	// check if the clip's start position is within the range
	// if not, fill the buffer from the earliest point this clip
	// will come into play.
	if (fileposition < (int)m_clip->get_source_start_frame()) {
// 		printf("not seeking to %d, but too %d\n\n", fileposition,m_clip->get_source_start_frame()); 
		// Song's start from 0, this makes a period start from
		// 0 - 1023 for example, the nframes is 1024!
		// Setting a songs new position is on 1024, and NOT 
		// 1023.. Hmm, something isn't correct here, but at least substract 1
		// to make this thing work!
		fileposition = m_clip->get_source_start_frame() - 1;
	}
	
	m_buffer->reset();
	m_rbFileReadPos = fileposition;
	m_rbRelativeFileReadPos = fileposition;
}

void PrivateReadSource::process_ringbuffer( audio_sample_t * framebuffer, bool seeking)
{
	// Do nothing if we passed the lenght of the AudioFile.
	if (m_rbFileReadPos >= m_length) {
		printf("returning, m_rbFileReadPos > m_length! (%d >  %d)\n", m_rbFileReadPos, m_source->m_length);
		if (m_syncInProgress) {
			finish_resync();
		}
		return;
	}
	
	// Calculate the number of samples we can write into the buffer
	int writeSpace = m_buffer->write_space();

	// The amount of chunks which can be 'read'
	int chunkCount = (int)(writeSpace / m_chunkSize);
	
	int toRead = m_chunkSize;
	
	if (seeking) {
		toRead = writeSpace;
		// For whatever reason, but FLAC crashes when refilling the 
		// buffer completely in one round! :-(
		if (m_isCompressedFile) {
			toRead = writeSpace / 2;
		}
		printf("doing a full seek buffer fill\n");
	} else 	if (m_syncInProgress) {
		toRead = m_chunkSize * 2;
	} else if (chunkCount == 0) {
		// If we are nearing the end of the source file it could be possible
		// we only need to read the last samples which is smaller in size then 
		// chunksize. If so, set toRead to m_source->m_length - rbFileReasPos
		if ( (int) (m_source->m_length - m_rbFileReadPos) <= m_chunkSize) {
			toRead = m_source->m_length - m_rbFileReadPos;
		} else {
		printf("chunkCount == 0, but not at end of file, this shouldn't happen!!\n");
			return;
		}
	}
	
	// Read in the samples from source
	nframes_t toWrite = rb_file_read(framebuffer, toRead);

	// and write it to the ringbuffer
	m_buffer->write(framebuffer, toWrite);
	
}

void PrivateReadSource::recover_from_buffer_underrun(nframes_t position)
{
	printf("buffer underrun detected!\n");
	m_bufferUnderRunDetected = 1;
	start_resync(position);
}

void PrivateReadSource::start_resync( nframes_t position )
{
// 	printf("starting resync!\n");
	m_syncPos = position;
	m_rbReady = 0;
	m_needSync = 1;
}

void PrivateReadSource::finish_resync()
{
// 	printf("sync finished\n");
	m_needSync = 0;
	m_bufferUnderRunDetected = 0;
	m_rbReady = 1;
	m_syncInProgress = false;
}

void PrivateReadSource::sync(audio_sample_t* framebuffer)
{
	PENTER;
	if (!m_needSync) {
		return;
	}
	
	if (!m_syncInProgress) {
		rb_seek_to_file_position(m_syncPos);
		m_syncInProgress = true;
	}
	
	process_ringbuffer(framebuffer);
	
	if (m_buffer->write_space() == 0) {
		finish_resync();
	}
	
//         PWARN("Resyncing ringbuffer finished");
}


void PrivateReadSource::set_audio_clip( AudioClip * clip )
{
	Q_ASSERT(!m_clip);
	m_clip = clip;
}

Peak* PrivateReadSource::get_peak( )
{
	return m_peak;
}

void PrivateReadSource::prepare_buffer( )
{
	PENTER;

	float size = config().get_property("Hardware", "PreBufferSize", 1.0).toDouble();

	if (m_isCompressedFile) {
		size *= 2;
		if (size > 3.0) {
			size = 3.0;
		}
	}

	m_bufferSize = (int) (size * audiodevice().get_sample_rate());

	if ( ! m_isCompressedFile) {
		m_chunkSize = m_bufferSize / 8;
	} else {
		m_chunkSize = m_bufferSize / 4;
	}

	m_buffer = new RingBufferNPT<float>(m_bufferSize);

	start_resync(m_clip->get_song()->get_working_frame());
}

BufferStatus PrivateReadSource::get_buffer_status()
{
	BufferStatus status;
	int freespace = m_buffer->write_space();

	if (m_rbFileReadPos >= m_length) {
		status.fillStatus =  100;
		freespace = 0;
	} else {
		status.fillStatus = (int) (((float)freespace / m_bufferSize) * 100);
	}
	
	status.bufferUnderRun = m_bufferUnderRunDetected;
	status.needSync = m_needSync;

	if ( ! m_isCompressedFile) {
		status.priority = (int) (freespace / m_chunkSize);
	} else {
		status.priority = (int) (freespace / m_chunkSize);
	}
	
	return status;
}

void PrivateReadSource::set_active(bool active)
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
