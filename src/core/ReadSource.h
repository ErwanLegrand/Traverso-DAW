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

#include <QDomDocument>


class AbstractAudioReader;
class AudioClip;
struct BufferStatus;
class DecodeBuffer;

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
	
	int set_state( const QDomNode& node );
	QDomNode get_state(QDomDocument doc);

	int rb_read(audio_sample_t** dest, TimeRef& start, nframes_t cnt);
	void rb_seek_to_file_position(TimeRef& position);
	
	int file_read(DecodeBuffer* buffer, TimeRef& start, nframes_t cnt) const;
	int file_read(DecodeBuffer* buffer, nframes_t start, nframes_t cnt);

	int init();
	int get_error() const {return m_error;}
	int set_file(const QString& filename);
	void set_active(bool active);
	
	void set_audio_clip(AudioClip* clip);
	const nframes_t get_nframes() const;
	int get_file_rate() const;
	const TimeRef& get_length() const {return m_length;}
	
	void sync(DecodeBuffer* buffer);
	void process_ringbuffer(DecodeBuffer* buffer, bool seeking=false);
	void prepare_buffer();
	size_t is_active() const;
	BufferStatus* get_buffer_status();
	
	void set_output_rate(int rate);
	
private:
	AbstractAudioReader*	m_audioReader;
	AudioClip* 		m_clip;
	int			m_refcount;
	int			m_error;
	bool			m_silent;
	TimeRef			m_rbFileReadPos;
	TimeRef			m_rbRelativeFileReadPos;
	TimeRef			m_syncPos;
	volatile size_t		m_rbReady;
	volatile size_t		m_needSync;
	volatile size_t		m_active;
	volatile size_t		m_wasActivated;
	volatile size_t		m_bufferUnderRunDetected;
	bool			m_syncInProgress;
	
	mutable TimeRef		m_length;
	QString		m_decodertype;
	
	BufferStatus*	m_bufferstatus;
	
	int ref() { return m_refcount++;}
	
	void private_init();
	void start_resync(TimeRef& position);
	void finish_resync();
	int rb_file_read(DecodeBuffer* buffer, nframes_t cnt);

	friend class ResourcesManager;
	
signals:
	void stateChanged();
};


inline size_t ReadSource::is_active() const
{
	return m_active;
}

#endif
