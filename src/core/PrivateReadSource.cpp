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

$Id: PrivateReadSource.cpp,v 1.1 2006/09/13 12:51:28 r_sijrier Exp $
*/

#include "PrivateReadSource.h"

#include "Peak.h"
#include "RingBuffer.h"
#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"
#include "ReadSource.h"
#include "Utils.h"

#include <QFile>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



PrivateReadSource::PrivateReadSource(ReadSource* source, uint chan, int channelNumber, const QString& fileName)
	: m_source(source), 
	  m_peak(0), 
	  sf(0), 
	  m_channelCount(chan), 
	  m_channelNumber(channelNumber), 
	  m_fileName(fileName)
{
	PENTERCONS;
}

PrivateReadSource::~PrivateReadSource()
{
	PENTERDES;
	if (readbuffer)
		delete [] readbuffer;
}


int PrivateReadSource::init( )
{
	PENTER;
	
	Q_ASSERT(sf == 0);
	
	rbFileReadPos = 0;
	rbRelativeFileReadPos = 0;
	rbReady = true;
	needSync = false;
	readbuffer = 0;
	readbuffersize = 0;
	seekPos = -1;
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

	if (m_channelCount >= (uint)sfinfo.channels) {
		PERROR("ReadAudioSource: file only contains %d channels; %d is invalid as a channel number", sfinfo.channels, m_channelCount);
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
	
	seekPos = start;
	

	if (sf_seek (sf, (off_t) start, SEEK_SET) < 0) {
		char errbuf[256];
		sf_error_str (0, errbuf, sizeof (errbuf) - 1);
		PERROR("ReadAudioSource: could not seek to frame %d within %s (%s)", start, QS_C(m_fileName), errbuf);
		return 0;
	}
	

	if (sfinfo.channels == 1) {
		nframes_t ret = sf_read_float (sf, dst, cnt);
		m_read_data_count = cnt * sizeof(float);
		return ret;
	}

	float *ptr;
	uint32_t real_cnt = cnt * sfinfo.channels;

	/*	{
			Do we want to have a Lock here during read?? */

	if (readbuffersize < real_cnt) {

		if (readbuffer) {
			delete [] readbuffer;
		}
		readbuffersize = real_cnt;
		readbuffer = new float[readbuffersize];
	}

	nread = sf_read_float (sf, readbuffer, real_cnt);
	ptr = readbuffer + m_channelNumber;
	nread /= sfinfo.channels;

	/* stride through the interleaved data */

	for (int32_t n = 0; n < nread; ++n) {
		dst[n] = *ptr;
		ptr += sfinfo.channels;
	}

	// 	}

	// 	m_read_data_count = cnt * sizeof(float);

	return nread;
}


int PrivateReadSource::rb_read(audio_sample_t* dst, nframes_t start, nframes_t cnt)
{

	if ( ! rbReady ) {
// 		printf("ringbuffer not ready\n");
		return 0;
	}


	if (start != rbRelativeFileReadPos) {
		if ( (start > rbRelativeFileReadPos) && (rbRelativeFileReadPos + (m_buffer->read_space() /  sizeof(audio_sample_t)) ) > start) {
			int advance = start - rbRelativeFileReadPos;
			m_buffer->read_advance( advance * sizeof(audio_sample_t) );
			rbRelativeFileReadPos += advance;
		} else {
// 			printf("starting resync!\n");
			start_resync(start);
			return 0;
		}
	}

	nframes_t readFrames = m_buffer->read((char*)dst, cnt * sizeof(audio_sample_t)) / sizeof(audio_sample_t);

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
	if (rbFileReadPos >= m_source->m_length) {
		return 0;
	}
	
	nframes_t writeSpace = m_buffer->write_space() / sizeof(audio_sample_t);

	int toRead = ((int)(writeSpace / 16384)) * 16384;
	
	if (toRead > 65536) {
		toRead = 65536;
	}
	
	if (toRead == 0) {
		if ( (m_source->m_length - rbFileReadPos) <= 16384) {
			toRead = m_source->m_length - rbFileReadPos;
		} else {
			return 0;
		}
	}
	
	nframes_t toWrite = rb_file_read(framebuffer, toRead);

	m_buffer->write((char*)framebuffer, toWrite * sizeof(audio_sample_t));
	
	return 0;
}

void PrivateReadSource::start_resync( nframes_t position )
{
	syncPos = position;
	rbReady = false;
	needSync = true;
//         PWARN("Resyncing ringbuffer start");
}

void PrivateReadSource::sync( )
{
	rb_seek_to_file_position(syncPos);
	needSync = false;
//         PWARN("Resyncing ringbuffer finished");
}

void PrivateReadSource::set_rb_ready( bool ready )
{
	rbReady = ready;
}


void PrivateReadSource::set_audio_clip( AudioClip * clip )
{
	Q_ASSERT(!m_clip);
	m_clip = clip;
}

bool PrivateReadSource::need_sync( )
{
	return needSync;
}

Peak* PrivateReadSource::get_peak( )
{
	return m_peak;
}

void PrivateReadSource::prepare_buffer( )
{
	PENTER;
	
	m_buffer = new RingBuffer(131072 * sizeof(audio_sample_t));
}

//eof

 
