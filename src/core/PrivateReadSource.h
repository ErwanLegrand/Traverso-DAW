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

$Id: PrivateReadSource.h,v 1.4 2006/10/02 19:04:38 r_sijrier Exp $
*/

#ifndef PRIVATE_READSOURCE_H
#define PRIVATE_READSOURCE_H

#include "RingBufferNPT.h"
#include "defines.h"

#include "sndfile.h"


class AudioClip;
class Peak;
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
	
	size_t need_sync();
	void sync(audio_sample_t* framebuffer);

	void set_audio_clip(AudioClip* clip);
	Peak* get_peak();

	void prepare_buffer();

private:
	PrivateReadSource(ReadSource* source, int sourceChannelCount, int channelNumber, const QString& fileName);
	~PrivateReadSource();
	
	ReadSource*	m_source;
	RingBufferNPT<float>*	m_buffer;
	Peak* 		m_peak;
	SNDFILE*	sf;
	SF_INFO 	sfinfo;
	int 		m_sourceChannelCount;
	int		m_channelNumber;
	
	nframes_t	rbFileReadPos;
	nframes_t	rbRelativeFileReadPos;
	volatile size_t	syncPos;
	volatile size_t	rbReady;
	volatile size_t	needSync;
	bool		syncInProgress;
	int		refcount;
	
	AudioClip*	m_clip;
	QString		m_fileName;
	

	void start_resync(nframes_t position);
	
	friend class ReadSource;
	
};

inline size_t PrivateReadSource::need_sync( )
{
	return needSync;
}

#endif
