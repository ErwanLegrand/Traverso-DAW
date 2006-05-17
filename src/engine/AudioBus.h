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

$Id: AudioBus.h,v 1.2 2006/05/17 22:07:05 r_sijrier Exp $
*/


#ifndef AUDIOBUS_H
#define AUDIOBUS_H

#include <QObject>
#include <QList>
#include <QString>
#include "defines.h"
#include "AudioChannel.h"

class AudioBus : public QObject
{
	Q_OBJECT

public:
	AudioBus(QString name);
	AudioBus(QString name, int channelCount);
	~AudioBus();

	void add_channel(AudioChannel* chan);
	int get_channel_count()
	{
		return channelCount;
	}

	QString get_name()
	{
		return m_name;
	}
	AudioChannel* get_channel(int channelNumber);

	audio_sample_t* get_buffer(int channel, nframes_t nframes)
	{
		return channels.at(channel)->get_buffer(nframes);
	}

	void set_buffer_size(nframes_t size);
	void set_monitor_peaks(bool monitor);
	
	void monitor_peaks()
	{
		foreach(AudioChannel* chan, channels) {
			chan->monitor_peaks();
		}
	}

	void silence_buffers(nframes_t nframes)
	{
		foreach(AudioChannel* chan, channels) {
			chan->silence_buffer(nframes);
		}
	}


private:
	QList<AudioChannel* >	channels;
	QString			deviceName;
	QString			m_name;
	
	int 			channelCount;

	void init(QString name);

public slots:
	void resize_buffer();

};

#endif

//eof
