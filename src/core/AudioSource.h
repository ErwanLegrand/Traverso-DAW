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
 
    $Id: AudioSource.h,v 1.4 2006/09/07 09:36:52 r_sijrier Exp $
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
        AudioSource(uint channelNumber, const QString& dir, const QString& name);
        AudioSource(const QDomNode node);
        AudioSource() {}
        ~AudioSource();

	virtual int process_ringbuffer(audio_sample_t* framebuffer) = 0;
	
        void set_name(const QString& name);
        void set_dir(const QString& name);
        void set_original_bit_depth(uint bitDepth);
        void set_created_by_song(int id);
        void set_sample_rate(int rate);
        int set_state( const QDomNode& node );
        void prepare_buffer();
        
        Peak* get_peak() const;
        QDomNode get_state(QDomDocument doc);
        QString get_filename() const;
        QString get_dir() const;
        QString get_name() const;
        qint64 get_id() const;
        nframes_t get_nframes() const;
        int get_rate();
        int get_channel_count();
        int get_channel();
        int get_clips_count();
        bool is_active() const {return active;}


protected:
        RingBuffer*	m_buffer;
        Peak* 		m_peak;
        SNDFILE*		sf;
        SF_INFO 		sfinfo;
        uint 			channelNumber;
        QString 		m_dir;
        QString 		m_name;
        QString		m_filename;
        nframes_t	seekPos;
        nframes_t	m_length;
        uint 			m_rate;
        uint			m_creationTime;
        uint			m_originalBitDepth;
        int			createdBySong;
        qint64		m_id;
        bool			active;

private:
        void create_id();
};

#endif



