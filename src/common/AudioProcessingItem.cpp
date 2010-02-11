/*
Copyright (C) 2010 Remon Sijrier

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


#include "AudioProcessingItem.h"

#include "AddRemove.h"
#include "AudioBus.h"
#include "AudioChannel.h"
#include "Mixer.h"
#include "PluginChain.h"

AudioProcessingItem::AudioProcessingItem(Sheet *sheet)
        : m_sheet(sheet)
{
        if (m_sheet) {
                m_pluginChain = new PluginChain(this, m_sheet);
        } else {
                m_pluginChain = new PluginChain(this);
        }
        m_fader = m_pluginChain->get_fader();

}

void AudioProcessingItem::set_input_bus(AudioBus *bus)
{
        THREAD_SAVE_INVOKE_AND_EMIT_SIGNAL(this, bus, private_set_input_bus(AudioBus*), busConfigurationChanged());
}

void AudioProcessingItem::set_output_bus(AudioBus *bus)
{
        THREAD_SAVE_INVOKE_AND_EMIT_SIGNAL(this, bus, private_set_output_bus(AudioBus*), busConfigurationChanged());
}

void AudioProcessingItem::private_set_input_bus(AudioBus* bus)
{
        m_inputBus = bus;
}

void AudioProcessingItem::private_set_output_bus(AudioBus* bus)
{
        m_outputBus = bus;
}

void AudioProcessingItem::send_to_output_buses(nframes_t nframes)
{
        if (!m_outputBus) {
                return;
        }

        AudioChannel* sender;
        AudioChannel* receiver;

        for (int i=0; i<m_processBus->get_channel_count(); i++) {
                sender = m_processBus->get_channel(i);
                receiver = m_outputBus->get_channel(i);
                if (sender && receiver) {
                        Mixer::mix_buffers_with_gain(receiver->get_buffer(nframes), sender->get_buffer(nframes), nframes, get_gain());
                        Mixer::mix_buffers_with_gain(receiver->get_buffer(nframes), sender->get_buffer(nframes), nframes, get_gain());
                }

        }

}



