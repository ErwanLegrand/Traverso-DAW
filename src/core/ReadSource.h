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

$Id: ReadSource.h,v 1.12 2006/10/04 19:26:12 r_sijrier Exp $
*/

#ifndef READSOURCE_H
#define READSOURCE_H

#include "AudioSource.h"
#include "PrivateReadSource.h"

class AudioClip;
class Peak;

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
	void process_ringbuffer(audio_sample_t* framebuffer);

	int file_read(int channel, audio_sample_t* dst, nframes_t start, nframes_t cnt) const;

	int init();
	int ref();
	void set_active();
	void set_inactive();
	void prepare_buffer();
	
	bool need_sync() const;
	void sync(audio_sample_t* framebuffer);

	void set_audio_clip(AudioClip* clip);
	Peak* get_peak(int channel);
	nframes_t get_nframes() const;
	
	bool is_active() const {return m_active;}
	
	int get_processable_buffer_space() const;

private:
	QList<PrivateReadSource*> m_sources;
	int		refcount;
	bool		m_active;
	
	int add_private_source(int channel, int channelNumber, const QString& fileName);
	
	friend class PrivateReadSource;
};


inline bool ReadSource::need_sync() const
{
	for (int i=0; i<m_sources.size(); ++i) {
		if (m_sources.at(i)->need_sync()) {
			return true;
		}
	}
	return false;
}

inline int ReadSource::get_processable_buffer_space( ) const
{
	if (m_sources.at(0)->rbFileReadPos >= m_length) {
		return 0;
	}
	return m_sources.at(0)->m_buffer->write_space();
}

inline void ReadSource::process_ringbuffer( audio_sample_t * framebuffer )
{
	for (int i=0; i<m_sources.size(); ++i) {
		m_sources.at(i)->process_ringbuffer(framebuffer);
	}
}

#endif
