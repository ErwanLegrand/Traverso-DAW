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

#ifndef READSOURCE_H
#define READSOURCE_H

#include "AudioSource.h"
#include "RingBufferNPT.h"
#include "defines.h"

class AbstractAudioReader;
class AudioClip;
struct BufferStatus;

class ReadSource : public AudioSource
{
	Q_OBJECT
public :
	ReadSource(const QDomNode node);
	ReadSource(const QString& dir, const QString& name);
	ReadSource(const QString& dir, const QString& name, int channelCount);
	ReadSource();  // For creating a 0-channel, silent ReadSource
	~ReadSource();
	
	enum ReadSourceError {
		COULD_NOT_OPEN_FILE = -1,
  		INVALID_CHANNEL_COUNT = -2,
    		ZERO_CHANNELS = -3,
      		CHANNELCOUNT_FILECOUNT_MISMATCH = -4
	};
	
	ReadSource* deep_copy();

	int rb_read(audio_sample_t** dest, nframes_t start, nframes_t cnt);
	void rb_seek_to_file_position(nframes_t position);
	
	int file_read(audio_sample_t** dst, nframes_t start, nframes_t cnt, audio_sample_t* readbuffer) const;

	int init();
	int get_error() const {return m_error;}
	int set_file(const QString& filename);
	void set_active(bool active);
	
	void set_audio_clip(AudioClip* clip);
	nframes_t get_nframes() const;
	
	void sync(audio_sample_t** framebuffer, audio_sample_t* readbuffer);
	void process_ringbuffer(audio_sample_t** framebuffer, audio_sample_t* readbuffer, bool seeking=false);
	void prepare_buffer();
	size_t is_active() const;
	BufferStatus* get_buffer_status();
	
public slots:
	void output_rate_changed();
	
private:
	QList<RingBufferNPT<float>*> m_buffers;
	AbstractAudioReader*	m_audioReader;
	AudioClip* 		m_clip;
	int			m_refcount;
	int			m_error;
	bool			m_silent;
	uint			m_bufferSize;
	int			m_chunkSize;
	nframes_t		m_rbFileReadPos;
	nframes_t		m_rbRelativeFileReadPos;
	volatile size_t		m_syncPos;
	volatile size_t		m_rbReady;
	volatile size_t		m_needSync;
	volatile size_t		m_active;
	volatile size_t		m_wasActivated;
	volatile size_t		m_bufferUnderRunDetected;
	bool			m_syncInProgress;
	
	BufferStatus*	m_bufferstatus;
	
	int ref() { return m_refcount++;}
	
	void private_init();
	void start_resync(nframes_t position);
	void finish_resync();
	void recover_from_buffer_underrun(nframes_t position);
	int rb_file_read(audio_sample_t** dst, nframes_t cnt, audio_sample_t* readbuffer);

	friend class ResourcesManager;
	
signals:
	void stateChanged();
};


inline size_t ReadSource::is_active() const
{
	return m_active;
}

#endif
