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

$Id: PrivateReadSource.cpp,v 1.4 2006/10/02 19:04:38 r_sijrier Exp $
*/

#include "PrivateReadSource.h"

#include "Peak.h"
#include "RingBuffer.h"
#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"
#include "ReadSource.h"
#include "Utils.h"
#include "AudioSource.h"
#include "RingBufferNPT.h"
#include "DiskIO.h"

#include <QFile>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



PrivateReadSource::PrivateReadSource(ReadSource* source, int sourceChannelCount, int channelNumber, const QString& fileName)
	: m_source(source),
	  m_buffer(0),
	  m_peak(0),
	  sf(0),
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
	if (sf) {
		if (sf_close (sf)) {
			qWarning("sf_close returned an error!");
		}
	}
}


int PrivateReadSource::init( )
{
	PENTER;
	
	Q_ASSERT(sf == 0);
	
	rbFileReadPos = 0;
	rbRelativeFileReadPos = 0;
	rbReady = 0;
	needSync = 0;
	syncInProgress = 0;
	m_clip = 0;

	/* although libsndfile says we don't need to set this,
	valgrind and source code shows us that we do.
	Really? Look it up !
	*/
	memset (&sfinfo, 0, sizeof(sfinfo));


	if ((sf = sf_open (QS_C(m_fileName), SFM_READ, &sfinfo)) == 0) {
		PERROR("Couldn't open soundfile (%s)", QS_C(m_fileName));
		return -1;
	}

	if (m_sourceChannelCount > sfinfo.channels) {
		PERROR("ReadAudioSource: file only contains %d channels; %d is invalid as a channel number", sfinfo.channels, m_sourceChannelCount);
		sf_close (sf);
		sf = 0;
		return -1;
	}

	if (sfinfo.channels == 0) {
		PERROR("ReadAudioSource: not a valid channel count: %d", sfinfo.channels);
		sf_close (sf);
		sf = 0;
		return -1;
	}

	m_source->m_length = sfinfo.frames;
	m_source->m_rate = sfinfo.samplerate;
	
	m_peak = new Peak(m_source, m_channelNumber);
	
	return 1;
}


int PrivateReadSource::file_read (audio_sample_t* dst, nframes_t start, nframes_t cnt) const
{
// 	PWARN("file_read");
	// this equals checking if init() is called!
	Q_ASSERT(sf);
	
	if (start >= m_source->m_length) {
		return 0;
	}
	
	if (sf_seek (sf, (off_t) start, SEEK_SET) < 0) {
		char errbuf[256];
		sf_error_str (0, errbuf, sizeof (errbuf) - 1);
		PERROR("ReadAudioSource: could not seek to frame %d within %s (%s)", start, QS_C(m_fileName), errbuf);
		return 0;
	}
	

	if (sfinfo.channels == 1) {
		return sf_read_float (sf, dst, cnt);
	}

	float *ptr;
	int real_cnt = cnt * sfinfo.channels;


	audio_sample_t readbuffer[real_cnt];

	int nread = sf_read_float (sf, readbuffer, real_cnt);
	ptr = readbuffer + m_channelNumber;
	nread /= sfinfo.channels;

	/* stride through the interleaved data */

	for (int32_t n = 0; n < nread; ++n) {
		dst[n] = *ptr;
		ptr += sfinfo.channels;
	}


	return nread;
}


int PrivateReadSource::rb_read(audio_sample_t* dst, nframes_t start, nframes_t cnt)
{

	if ( ! rbReady ) {
// 		printf("ringbuffer not ready\n");
		return 0;
	}


	if (start != rbRelativeFileReadPos) {
		int available = m_buffer->read_space();
		if ( (start > rbRelativeFileReadPos) && ((rbRelativeFileReadPos + (available / 0.7)) > start) ) {
			int advance = start - rbRelativeFileReadPos;
			m_buffer->increment_read_ptr(advance);
			rbRelativeFileReadPos += advance;
		} else {
// 			printf("starting resync!\n");
			start_resync(start);
			return 0;
		}
	}

	nframes_t readFrames = m_buffer->read(dst, cnt);

	if (readFrames != cnt) {
		// Hmm, not sure what to do in this case....
	}

	rbRelativeFileReadPos += readFrames;

	return readFrames;
}


int PrivateReadSource::rb_file_read( audio_sample_t * dst, nframes_t cnt )
{
	int readFrames = file_read( dst, rbFileReadPos, cnt);
	rbFileReadPos += readFrames;

	return readFrames;
}


void PrivateReadSource::rb_seek_to_file_position( nframes_t position )
{
	Q_ASSERT(m_clip);
	
	if (rbFileReadPos == position) {
		PMESG("ringbuffer allready at position %d", position);
		return;
	}
	
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
	rbFileReadPos = fileposition;
	rbRelativeFileReadPos = fileposition;
}

int PrivateReadSource::process_ringbuffer( audio_sample_t * framebuffer )
{
	// Do nothing if we passed the lenght of the AudioFile.
	if (rbFileReadPos >= m_source->m_length) {
		printf("returning, rbFileReadPos > m_length! (%d >  %d)\n", rbFileReadPos, m_source->m_length);
		return 0;
	}
	
	// Calculate the number of samples we can write into the buffer
	int writeSpace = m_buffer->write_space();
	
	// calculate the 'chunk' size 
	int chunkSize = m_source->diskio->get_chunk_size();
	
	// The amount of chunks which can be 'read'
	int chunkCount = (int)(writeSpace / chunkSize);
	
	
	int toRead;
	
	// Calculate how many chuncks should be read in.
	// In case the buffer nears emptyness read in a bit more
	// else just read in chunkSize.
	if (chunkCount >= 1) {
		if (syncInProgress) {
			toRead = chunkSize * 2;
		} else if (chunkCount < 7){
			toRead = chunkSize;
		} else {
			toRead = chunkSize * 8;
		}
	} else {
		// If we are nearing the end of the source file it could be possible
		// we only need to read the last samples which is smaller in size then 
		// chunksize. If so, set toRead to m_source->m_length - rbFileReasPos
		if ( (int) (m_source->m_length - rbFileReadPos) <= chunkSize) {
			toRead = m_source->m_length - rbFileReadPos;
		} else {
			return 0;
		}
	}
	
	// Read in the samples from source
	nframes_t toWrite = rb_file_read(framebuffer, toRead);

	// and write it to the ringbuffer
	m_buffer->write(framebuffer, toWrite);
	
	return writeSpace;
}

void PrivateReadSource::start_resync( nframes_t position )
{
	syncPos = position;
	rbReady = false;
	needSync = true;
//         PWARN("Resyncing ringbuffer start");
}

void PrivateReadSource::sync(audio_sample_t* framebuffer)
{
	PENTER;
	if (!needSync) {
		return;
	}
	
	if (!syncInProgress) {
		rb_seek_to_file_position(syncPos);
		syncInProgress = true;
	}
	
	int writeSpace = process_ringbuffer(framebuffer);
	
	if (writeSpace == 0) {
		needSync = false;
		rbReady = true;
		syncInProgress = false;
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
	
	m_buffer = new RingBufferNPT<float>(m_source->diskio->get_buffer_size());
	start_resync(m_clip->get_song()->get_working_frame());
}

//eof

 
