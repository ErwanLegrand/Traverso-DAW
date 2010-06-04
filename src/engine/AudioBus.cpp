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
#include "Utils.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
 * \class AudioBus
 * A convenience class to wrap (the likely 2) AudioChannels in the Bus concept.
 * 
 */


/**
 * Constructs an AudioBus instance with name \a name and channel \a channels
 *
 * This is a convenience constructor, which populates the AudioBus with \a channels AudioChannels
 * The buffer size of the AudioChannels is the same as the current AudioDevice::get_buffer_size()
 * @param name The name of the AudioBus
 * @param channels The number of AudioChannels to add to this AudioBus
 * @return a new AudioBus instance
 */
AudioBus::AudioBus(const BusConfig& config)
{
        m_isMonitoring = true;

        m_channelCount = 0;
        m_name = config.name;
        if (config.type == "input") {
                m_type = ChannelIsInput;
        } else {
                m_type = ChannelIsOutput;
        }

        if (config.bustype == "hardware") {
                m_busType = BusIsHardware;
        } else if (config.bustype == "software") {
                m_busType = BusIsSoftware;
        } else {
                Q_ASSERT("bustype != hardware or software");
        }

        m_id = config.id;

        // id was never created if it == -1, so create a unique one now!
        if (m_id == -1) {
                m_id = create_id();
        }

        m_isInternalBus = config.isInternalBus;

        // This bus is an internal Bus, not a Bus used to wrap AudioChannel's from a Driver
        // so we have to create AudioChannel's to make the Bus useful for internal routing.
        if (m_isInternalBus) {
                for(int channelNumber=0; channelNumber < config.channelcount; ++channelNumber) {
                        AudioChannel* chan = audiodevice().create_channel(m_name, channelNumber, m_type);
                        add_channel(chan);
                }
        }
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
        m_channels.append(chan);
        m_channelCount++;
}

void AudioBus::add_channel(const QString &channel)
{
        m_channelNames.append(channel);
}

QStringList AudioBus::get_channel_names() const
{
        if (m_busType == BusIsHardware) {
                return m_channelNames;
        }

        QStringList list;
        foreach(AudioChannel* channel, m_channels) {
                list.append(channel->get_name());
        }

        return list;
}
void AudioBus::audiodevice_params_changed()
{
        m_channels.clear();
        m_channelCount = 0;

        AudioChannel* channel;

        foreach(QString channelName, m_channelNames) {
                if (is_input()) {
                        channel = audiodevice().get_capture_channel_by_name(channelName);
                } else {
                        channel = audiodevice().get_playback_channel_by_name(channelName);
                }

                if (channel) {
                        add_channel(channel);
                } else {
                        printf("channel not found %s\n", channelName.toAscii().data());
                }
        }
}

/**
 * If set to true, all the data going through the AudioChannels in this AudioBus
 * will be monitored for their highest peak value.
 * Get the peak value with AudioChannel::get_peak_value()
 * @param monitor 
 */
void AudioBus::set_monitoring( bool monitor )
{
        m_isMonitoring = monitor;
	
	for (int i=0; i<m_channels.size(); ++i) {
                m_channels.at(i)->set_monitoring(monitor);
	}
}

//eof

