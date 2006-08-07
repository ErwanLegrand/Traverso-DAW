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

$Id: ReadSource.cpp,v 1.9 2006/08/07 19:16:23 r_sijrier Exp $
*/

#include "ReadSource.h"

#include "Peak.h"
#include "RingBuffer.h"
#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"

#include <QFile>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


// This constructor is called for existing (recorded/imported) audio sources
ReadSource::ReadSource(const QDomNode node)
		: AudioSource(node), refcount(0)
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

ReadSource::ReadSource(uint chan, QString dir, QString name)
		: AudioSource(chan, dir, name), refcount(0)
{
}

ReadSource::ReadSource( AudioSource * source )
		: AudioSource(source->get_channel(), source->get_dir(), source->get_name()), refcount(0)
{
	m_peak = source->get_peak();
	m_peak->set_audiosource(this);
	source->set_peak(0);
	init();
}

ReadSource::~ReadSource()
{
	PENTERDES;
	if (readbuffer)
		delete [] readbuffer;
}


int ReadSource::init( )
{
	PENTER2;
	
	Q_ASSERT(sf == 0);
	
	rbFileReadPos = 0;
	rbRelativeFileReadPos = 0;
	rbReady = true;
	needSync = false;
	readbuffer = 0;
	readbuffersize = 0;
	seekPos = -1;
	sharedReadSource = (ReadSource*) 0;
	m_clip = 0;

	/* although libsndfile says we don't need to set this,
	valgrind and source code shows us that we do.
	Really? Look it up !
	*/
	memset (&sfinfo, 0, sizeof(sfinfo));


	if ((sf = sf_open (m_filename.toAscii().data(), SFM_READ, &sfinfo)) == 0) {
		PERROR("Couldn't open soundfile (%s)", m_filename.toAscii().data());
		return -1;
	}

	if (channelNumber >= (uint)sfinfo.channels) {
		PERROR("ReadAudioSource: file only contains %d channels; %d is invalid as a channel number", sfinfo.channels, channelNumber);
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

	m_length = sfinfo.frames;
	return 1;
}


int ReadSource::file_read (audio_sample_t* dst, nframes_t start, nframes_t cnt) const
{
// 	PWARN("file_read");
	// this equals checking if init() is called!
	Q_ASSERT(sf);
	
	if (start >= m_length) {
		return 0;
	}
	
	seekPos = start;
	
	if (sharedReadSource) {
		if (sharedReadSource->get_seek_position() == (int) start) {
			return sharedReadSource->shared_file_read(dst, start, cnt, channelNumber);
		}
	}


	if (sf_seek (sf, (off_t) start, SEEK_SET) < 0) {
		char errbuf[256];
		sf_error_str (0, errbuf, sizeof (errbuf) - 1);
		PERROR("ReadAudioSource: could not seek to frame %d within %s (%s)", start, m_filename.toAscii().data(), errbuf);
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
	ptr = readbuffer + channelNumber;
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

int ReadSource::shared_file_read( audio_sample_t * dst,  nframes_t /*start*/, nframes_t /*cnt*/, uint channelNumber ) const
{
// 	PWARN("Entering shared_file_read");
	float *ptr;
	ptr = readbuffer + channelNumber;
	
	if (!readbuffer)
		return 0;

	for (int32_t n = 0; n < nread; ++n) {
		dst[n] = *ptr;
		ptr += sfinfo.channels;
	}
	
	return nread;
}


int ReadSource::rb_read(audio_sample_t* dst, nframes_t start, nframes_t cnt)
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


int ReadSource::rb_file_read( audio_sample_t * dst, nframes_t cnt )
{
	int readFrames = file_read( dst, rbFileReadPos, cnt);
	rbFileReadPos += readFrames;

	return readFrames;
}


void ReadSource::rb_seek_to_file_position( nframes_t position )
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

int ReadSource::process_ringbuffer( audio_sample_t * framebuffer )
{
	if (rbFileReadPos >= m_length) {
		return 0;
	}
	
	nframes_t writeSpace = m_buffer->write_space() / sizeof(audio_sample_t);

	int toRead = ((int)(writeSpace / 16384)) * 16384;
	
	if (toRead > 65536) {
		toRead = 65536;
	}
	
	if (toRead == 0) {
		if ( (m_length - rbFileReadPos) <= 16384) {
			toRead = m_length - rbFileReadPos;
		} else {
			return 0;
		}
	}
	
	nframes_t toWrite = rb_file_read(framebuffer, toRead);

	m_buffer->write((char*)framebuffer, toWrite * sizeof(audio_sample_t));
	
	return 0;
}

void ReadSource::start_resync( nframes_t position )
{
	syncPos = position;
	rbReady = false;
	needSync = true;
//         PWARN("Resyncing ringbuffer start");
}

void ReadSource::sync( )
{
	rb_seek_to_file_position(syncPos);
	needSync = false;
//         PWARN("Resyncing ringbuffer finished");
}

void ReadSource::set_rb_ready( bool ready )
{
	rbReady = ready;
}

void ReadSource::set_active( )
{
	PENTER2;
	active = true;
}

void ReadSource::set_inactive( )
{
	PENTER2;
	active = false;
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
	Q_ASSERT(!m_clip);
	m_clip = clip;
}

//eof

