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
 
    $Id: AudioSource.cpp,v 1.2 2006/05/17 21:57:21 r_sijrier Exp $
*/

#include <QDateTime>
#include <QMutex>

#include "AudioSource.h"
#include "Song.h"
#include "Peak.h"
#include "Export.h"
#include "RingBuffer.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

// QMutex mutex;


// This constructor is called during file import
AudioSource::AudioSource(uint chanNumber, QString dir, QString name)
                : sf(0), channelNumber(chanNumber), m_dir(dir), m_name(name)
{
        PENTERCONS;
        m_filename = m_dir + "/" + m_name;
        create_id();
        private_init();
}


// This constructor is called for existing (recorded/imported) audio sources
AudioSource::AudioSource(const QDomNode node)
	: sf(0)
{
        set_state(node);
        private_init();
}


AudioSource::~AudioSource()
{
        PENTERDES;
        if (m_peak)
                delete m_peak;
        if (m_buffer)
                delete m_buffer;
        if (sf)
                sf_close (sf);
}


void AudioSource::private_init( )
{
        m_buffer = new RingBuffer(131072);
        m_buffer->mlock_buffer();
        m_buffer->reset();
        m_peak = new Peak(this);
        active = true;
}


QDomNode AudioSource::get_state( QDomDocument doc )
{
        QDomElement node = doc.createElement("Source");
        node.setAttribute("dir", m_dir);
        node.setAttribute("name", m_name);
        node.setAttribute("bitdepth", m_originalBitDepth);
        node.setAttribute("createdBySong", createdBySong);
        node.setAttribute("id", m_id);
        node.setAttribute("channelNumber", channelNumber);

        return node;
}


int AudioSource::set_state( const QDomNode & node )
{
        QDomElement e = node.toElement();
        m_id = e.attribute( "id", "" ).toLongLong();
        set_dir( e.attribute( "dir", "" ) );
        set_name( e.attribute( "name", "" ) );
        m_originalBitDepth = e.attribute( "bitdepth", "" ).toInt();
        createdBySong = e.attribute( "createdBySong", "" ).toInt();
        channelNumber = e.attribute( "channelNumber", "" ).toInt();
        return 1;
}


int AudioSource::rebuild_peaks()
{
        PENTER;
        if (m_peak)
                delete m_peak;
        m_peak = new Peak(this);
        return 1;
}


int AudioSource::get_clips_count()
{
        //         return m_song->get_clips_count_for_audio(this);
        return 0;
}


void AudioSource::set_name(QString name)
{
        m_name = name;
        m_filename = m_dir + "/" + m_name;
}


void AudioSource::set_dir(QString dir)
{
        m_dir = dir;
        m_filename = m_dir + "/" + m_name;
}


int AudioSource::get_rate( )
{
        return sfinfo.samplerate;
}

int AudioSource::get_channel_count( )
{
        return sfinfo.channels;
}

int AudioSource::get_channel( )
{
        return channelNumber;
}

void AudioSource::create_id( )
{
        int r = rand();
        QDateTime time = QDateTime::currentDateTime();
        uint timeValue = time.toTime_t();
        m_id = timeValue;
        m_id *= 1000000000;
        m_id += r;
}


nframes_t AudioSource::get_nframes( ) const
{
        return sfinfo.frames;
}

void AudioSource::set_peak( Peak * peak )
{
        m_peak = peak;
}

void AudioSource::set_original_bit_depth( uint bitDepth )
{
        m_originalBitDepth = bitDepth;
}

void AudioSource::set_created_by_song( int id )
{
        createdBySong = id;
}

void AudioSource::set_sample_rate( int rate )
{
        m_rate = rate;
}

Peak * AudioSource::get_peak( ) const
{
        return m_peak;
}

QString AudioSource::get_filename( ) const
{
        return m_filename;
}

QString AudioSource::get_dir( ) const
{
        return m_dir;
}

QString AudioSource::get_name( ) const
{
        return m_name;
}

qint64 AudioSource::get_id( ) const
{
        return m_id;
}

// eof
