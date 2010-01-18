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

$Id: AudioBus.cpp,v 1.11 2008/01/21 16:22:15 r_sijrier Exp $
*/

#include "AudioBus.h"
#include "AudioChannel.h"
#include "Mixer.h"
#include "AudioDevice.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
 * \class AudioBus
 * A convenience class to wrap (the likley 2) AudioChannels in the well known Bus concept.
 * 
 */


/**
 * Constructs an AudioBus instance with name \a name 
 * @param name The name of the AudioBus
 * @return a new AudioBus instance
 */
AudioBus::AudioBus(const QString& name)
		: QObject()
{
	PENTERCONS;
	
	init(name);
}

/**
 * Constructs an AudioBus instance with name \a name and channel \a channels
 *
 * This is a convenience constructor, which populates the AudioBus with \a channels AudioChannels
 * The buffer size of the AudioChannels is the same as the current AudioDevice::get_buffer_size() 
 * @param name The name of the AudioBus
 * @param channels The number of AudioChannels to add to this AudioBus
 * @return a new AudioBus instance
 */
AudioBus::AudioBus( const QString& name, int channels )
{
	PENTERCONS;
	
	init(name);

	for(int channelNumber=0; channelNumber<channels; ++channelNumber) {
		AudioChannel* chan = new AudioChannel(name, "", 0,  channelNumber);
		chan->set_buffer_size(audiodevice().get_buffer_size());
		add_channel(chan);
	}
}

void AudioBus::init(const QString& name )
{
	channelCount = m_monitors = 0;
	m_name = name;
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(resize_buffer()), Qt::DirectConnection);
}


AudioBus::~ AudioBus( )
{
}


/**
 * Add's AudioChannel \a chan to this AudioBus channel list
 *
 * This function is used by the AudioDrivers, use the convenience constructor AudioBus( QString name, int channels )
 * if you want to quickly create an AudioBus with a certain amount of AudioChannels!
 * @param chan The AudioChannel to add.
 */
void AudioBus::add_channel(AudioChannel* chan)
{
	Q_ASSERT(chan);
	channels.append(chan);
	channelCount++;
}


/**
 * Resizes all the AudioChannel buffers to the new size.
 *
 * WARNING: This is not thread save! 
 *
 * @param size The new buffer size 
 */
void AudioBus::set_buffer_size( nframes_t size )
{
	for (int i=0; i<channels.size(); ++i) {
		channels.at(i)->set_buffer_size(size);
	}
}


void AudioBus::resize_buffer( )
{
	set_buffer_size(audiodevice().get_buffer_size());
}


/**
 * If set to true, all the data going through the AudioChannels in this AudioBus
 * will be monitored for their highest peak value.
 * Get the peak value with AudioChannel::get_peak_value()
 * @param monitor 
 */
void AudioBus::set_monitor_peaks( bool monitor )
{
	if (monitor) {
		m_monitors++;
	} else {
		m_monitors--;
	}
	
	for (int i=0; i<channels.size(); ++i) {
		channels.at(i)->set_monitor_peaks(m_monitors);
	}
	
	if (m_monitors > 0) {
		emit monitoringPeaksStarted();
	}
	if (m_monitors == 0) {
		emit monitoringPeaksStopped();
	}
}

void AudioBus::reset_monitor_peaks()
{
	m_monitors = 0;
	emit monitoringPeaksStopped();
}

//eof

