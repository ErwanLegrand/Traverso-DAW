/*
Copyright (C) 2005-2006 Remon Sijrier 

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

$Id: AudioSource.h,v 1.7 2006/09/18 18:30:14 r_sijrier Exp $
*/

#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <sndfile.h>
#include "defines.h"

#include <QObject>
#include <QList>
#include <QDomDocument>

class QString;
class Peak;
class Song;
class RingBuffer;

/// The base class for AudioSources like ReadSource and WriteSource
class AudioSource : public QObject
{
public :
	AudioSource(const QString& dir, const QString& name);
	AudioSource(const QDomNode node);
	AudioSource() {}
	~AudioSource();
	
	enum BufferProcessPrio {
		NormalPrio, 
		MediumPrio,
		HighPrio
	};

	virtual int process_ringbuffer(audio_sample_t* framebuffer) = 0;
	virtual void set_buffer_process_prio(BufferProcessPrio prio);
	
        static bool greater(const AudioSource* left, const AudioSource* right )
        {
                return left->get_buffer_process_prio() > right->get_buffer_process_prio();
        }
	
	void set_name(const QString& name);
	void set_dir(const QString& name);
	void set_original_bit_depth(uint bitDepth);
	void set_created_by_song(int id);
	void set_sample_rate(int rate);
	int set_state( const QDomNode& node );
	int get_buffer_process_prio() const;
	
	
	QDomNode get_state(QDomDocument doc);
	QString get_filename() const;
	QString get_dir() const;
	QString get_name() const;
	qint64 get_id() const;
	int get_rate() const;
	uint get_channel_count() const;
	int get_bit_depth() const;
	
	void set_channel_count(uint count);

protected:
	uint		m_channelCount;
	uint		m_fileCount;
	int		m_origSongId;
	QString 	m_dir;
	qint64		m_id;
	QString 	m_name;
	uint		m_origBitDepth;
	QString		m_fileName;
	nframes_t	m_length;
	uint 		m_rate;
	BufferProcessPrio m_bufferProcessPrio;
	int		m_preBufferSize;
};

inline uint AudioSource::get_channel_count( ) const
{
	return m_channelCount;
}

#endif



