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

$Id: AudioBus.cpp,v 1.4 2006/06/26 23:58:13 r_sijrier Exp $
*/

#include "AudioBus.h"
#include "AudioChannel.h"
#include "Mixer.h"
#include "AudioDevice.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AudioBus::AudioBus(QString name)
		: QObject()
{
	PENTERCONS;
	
	init(name);
}

AudioBus::AudioBus( QString name, int channels )
{
	PENTERCONS;
	
	init(name);

	for(int channelNumber=0; channelNumber<channels; ++channelNumber) {
		AudioChannel* chan = new AudioChannel(name, "", 0,  channelNumber);
		chan->set_buffer_size(audiodevice().get_buffer_size());
		add_channel(chan);
	}
}

void AudioBus::init( QString name )
{
	channelCount = 0;
	m_name = name;
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(resize_buffer()), Qt::DirectConnection);
}


AudioBus::~ AudioBus( )
{
	while( ! channels.isEmpty())
		delete channels.takeFirst();
}


void AudioBus::add_channel(AudioChannel* chan)
{
	channels.append(chan);
	channelCount++;
}


void AudioBus::set_buffer_size( nframes_t size )
{
	foreach(AudioChannel* chan, channels)
		chan->set_buffer_size(size);
}


void AudioBus::resize_buffer( )
{
	set_buffer_size(audiodevice().get_buffer_size());
}


AudioChannel * AudioBus::get_channel( int channelNumber )
{
	return channels.at(channelNumber);
}

void AudioBus::set_monitor_peaks( bool monitor )
{
	foreach(AudioChannel* chan, channels) {
		chan->set_monitor_peaks(monitor);
	}
}

//eof
