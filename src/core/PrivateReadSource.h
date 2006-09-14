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

$Id: PrivateReadSource.h,v 1.2 2006/09/14 10:49:39 r_sijrier Exp $
*/

#ifndef PRIVATE_READSOURCE_H
#define PRIVATE_READSOURCE_H

#include "AudioSource.h"

class AudioClip;
class ReadSource;

class PrivateReadSource
{
public :
	int rb_read(audio_sample_t* dst, nframes_t start, nframes_t cnt);
	int rb_file_read(audio_sample_t* dst, nframes_t cnt);
	void rb_seek_to_file_position(nframes_t position);
	int process_ringbuffer(audio_sample_t* framebuffer);

	int file_read(audio_sample_t* dst, nframes_t start, nframes_t cnt) const;

	int init();
	int ref();
	void set_rb_ready(bool ready);
	
	bool need_sync();
	void sync();

	void set_audio_clip(AudioClip* clip);
	Peak* get_peak();

	void prepare_buffer();

private:
	PrivateReadSource(ReadSource* source, uint channel, int channelNumber, const QString& fileName);
	~PrivateReadSource();
	
	ReadSource*	m_source;
	RingBuffer*	m_buffer;
	Peak* 		m_peak;
	SNDFILE*	sf;
	SF_INFO 	sfinfo;
	uint 		m_channelCount;
	int		m_channelNumber;
	
	mutable float*	 	m_readbuffer;
	mutable nframes_t 	readbuffersize;
	mutable int32_t 	nread;
	mutable uint32_t 	m_read_data_count;
	nframes_t		rbFileReadPos;
	nframes_t		rbRelativeFileReadPos;
	nframes_t		syncPos;
	mutable int		seekPos;
	bool			rbReady;
	bool			needSync;
	int			refcount;
	
	AudioClip*		m_clip;
	QString			m_fileName;
	

	void start_resync(nframes_t position);
	
	friend class ReadSource;
	
};

#endif
