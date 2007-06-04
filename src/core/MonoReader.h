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

$Id: MonoReader.h,v 1.2 2007/06/04 18:22:52 r_sijrier Exp $
*/

#ifndef PRIVATE_READSOURCE_H
#define PRIVATE_READSOURCE_H

#include "AudioSource.h"
#include "RingBufferNPT.h"
#include "defines.h"
#include "sndfile.h"


class AudioClip;
class Peak;
class ReadSource;
class QString;
struct BufferStatus;

class MonoReader : public AudioSource
{
public :
	int rb_read(audio_sample_t* dst, nframes_t start, nframes_t cnt);
	int rb_file_read(audio_sample_t* dst, nframes_t cnt, audio_sample_t* readbuffer);
	void rb_seek_to_file_position(nframes_t position);

	void process_ringbuffer(audio_sample_t* framebuffer, audio_sample_t* readbuffer, bool seeking=false);
	BufferStatus* get_buffer_status();

	int file_read(audio_sample_t* dst, nframes_t start, nframes_t cnt, audio_sample_t* readbuffer) const;

	int init();
	int ref();
	
	void sync(audio_sample_t* framebuffer, audio_sample_t* readbuffer);
	void set_audio_clip(AudioClip* clip);
	void prepare_buffer();

	Peak* get_peak();
	size_t is_active() const;

private:
	MonoReader(ReadSource* source, int sourceChannelCount, int channelNumber, const QString& fileName);
	~MonoReader();
	
	ReadSource*	m_source;
	RingBufferNPT<float>*	m_buffer;
	Peak* 		m_peak;
	SNDFILE*	m_sf;
	SF_INFO 	m_sfinfo;
	int 		m_sourceChannelCount;
	int		m_channelNumber;
	uint		m_length;
	uint		m_bufferSize;
	int		m_chunkSize;
	
	nframes_t	m_rbFileReadPos;
	nframes_t	m_rbRelativeFileReadPos;
	volatile size_t	m_syncPos;
	volatile size_t	m_rbReady;
	volatile size_t	m_needSync;
	volatile size_t	m_active;
	volatile size_t	m_wasActivated;
	volatile size_t	m_bufferUnderRunDetected;
	bool		m_syncInProgress;
	bool		m_isCompressedFile;
	int		m_prio;
	
	AudioClip*	m_clip;
	BufferStatus*	m_bufferstatus;
	QString		m_fileName;
	

	void start_resync(nframes_t position);
	void finish_resync();
	void set_active(bool active);

	void recover_from_buffer_underrun(nframes_t position);
	
	friend class ReadSource;
	
};

inline size_t MonoReader::is_active() const
{
	return m_active;
}


#endif
