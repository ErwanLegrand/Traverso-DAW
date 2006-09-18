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

$Id: ReadSource.h,v 1.10 2006/09/18 18:30:14 r_sijrier Exp $
*/

#ifndef READSOURCE_H
#define READSOURCE_H

#include "AudioSource.h"

class AudioClip;
class PrivateReadSource;

class ReadSource : public AudioSource
{
public :
	ReadSource(const QDomNode node);
	ReadSource(const QString& dir, const QString& name);
	ReadSource(const QString& dir, const QString& name, int channelCount, int fileCount);
	~ReadSource();
	
	ReadSource* deep_copy();

	int rb_read(int channel, audio_sample_t* dst, nframes_t start, nframes_t cnt);
	void rb_seek_to_file_position(nframes_t position);
	void set_buffer_process_prio(BufferProcessPrio prio);
	int process_ringbuffer(audio_sample_t* framebuffer);

	int file_read(int channel, audio_sample_t* dst, nframes_t start, nframes_t cnt) const;

	int init();
	int ref();
	void set_active();
	void set_inactive();
	void prepare_buffer();
	
	bool need_sync();
	void sync();

	void set_audio_clip(AudioClip* clip);
	Peak* get_peak(int channel);
	nframes_t get_nframes() const;
	
	bool is_active() const {return m_active;}

private:
	QList<PrivateReadSource*> m_sources;
	int		refcount;
	bool		m_active;
	
	friend class PrivateReadSource;
};


#endif
