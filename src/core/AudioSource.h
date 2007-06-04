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

$Id: AudioSource.h,v 1.17 2007/06/04 18:22:52 r_sijrier Exp $
*/

#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include "defines.h"

#include <QObject>
#include <QDomDocument>

class QString;

/// The base class for AudioSources like ReadSource and WriteSource
class AudioSource : public QObject
{
public :
	AudioSource(const QString& dir, const QString& name);
	AudioSource(const QDomNode node);
	AudioSource(){};
	~AudioSource();
	
	virtual void process_ringbuffer(audio_sample_t* framebuffer, bool seeking=false);
	virtual void process_ringbuffer(audio_sample_t* framebuffer, audio_sample_t* readbuffer, bool seeking=false);

	void set_name(const QString& name);
	void set_dir(const QString& name);
	void set_original_bit_depth(uint bitDepth);
	void set_created_by_song(qint64 id);
	void set_sample_rate(int rate);
	int set_state( const QDomNode& node );
	
	
	QDomNode get_state(QDomDocument doc);
	QString get_filename() const;
	QString get_dir() const;
	QString get_name() const;
	QString get_short_name() const;
	qint64 get_id() const;
	qint64 get_orig_song_id() const {return m_origSongId;}
	int get_rate() const;
	uint get_channel_count() const;
	int get_bit_depth() const;
	
protected:
	uint		m_channelCount;
	uint		m_fileCount;
	qint64		m_origSongId;
	QString 	m_dir;
	qint64		m_id;
	QString 	m_name;
	QString		m_shortName;
	uint		m_origBitDepth;
	QString		m_fileName;
	nframes_t	m_length;
	uint 		m_rate;
	int		m_wasRecording;
};


inline uint AudioSource::get_channel_count( ) const {return m_channelCount;}

inline void AudioSource::process_ringbuffer(audio_sample_t*, bool) {}
inline void AudioSource::process_ringbuffer(audio_sample_t* , audio_sample_t* , bool ) {}
inline qint64 AudioSource::get_id( ) const {return m_id;}

#endif
