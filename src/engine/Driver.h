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
 
    $Id: Driver.h,v 1.5 2007/03/19 11:18:57 r_sijrier Exp $
*/

#ifndef DRIVER_H
#define DRIVER_H

#include "defines.h"
#include <memops.h>


#define DEFAULTDRIVERBUFFERSIZE  1024

#include <QList>
#include <QString>
#include <QObject>

class AudioDevice;
class AudioChannel;

class Driver : public QObject
{
public:
        Driver(AudioDevice* dev, int rate, nframes_t bufferSize);
        virtual ~Driver();

        virtual int _run_cycle();
        virtual int _read(nframes_t nframes);
        virtual int _write(nframes_t nframes);
        virtual int _null_cycle(nframes_t nframes);
	virtual int setup(bool capture=true, bool playback=true, const QString& cardDevice="none");
        virtual int attach();
        virtual int detach();
        virtual int start();
        virtual int stop();
        virtual QString get_device_name();
        virtual QString get_device_longname();

        QList<AudioChannel* > get_capture_channels() const {return m_captureChannels;}
        QList<AudioChannel* > get_playback_channels() const {return m_playbackChannels;}

        AudioChannel* get_capture_channel_by_name(const QString& name);
        AudioChannel* get_playback_channel_by_name(const QString& name);

        virtual AudioChannel* add_playback_channel(const QByteArray& chanName);
        virtual AudioChannel* add_capture_channel(const QByteArray& chanName);

        virtual int remove_capture_channel(QString ) {return -1;}
        virtual int remove_playback_channel(QString ) {return -1;}


        ProcessCallback read;
        ProcessCallback write;
        RunCycleCallback run_cycle;


protected:
        AudioDevice* device;
        QList<AudioChannel* >		m_captureChannels;
        QList<AudioChannel* >		m_playbackChannels;
        int             		dither;
        dither_state_t*			dither_state;
        trav_time_t 			period_usecs;
        trav_time_t 			last_wait_ust;
        nframes_t                frame_rate;
        nframes_t                frames_per_cycle;
        nframes_t                capture_frame_latency;
        nframes_t                playback_frame_latency;

};


#endif

//eof

