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


#include "ProcessingData.h"

#include "AddRemove.h"
#include "AudioBus.h"
#include "AudioChannel.h"
#include "AudioDevice.h"
#include "AudioClip.h"
#include "AudioClipManager.h"
#include "Mixer.h"
#include "PluginChain.h"
#include "Sheet.h"
#include "SubGroup.h"

#include "Debugger.h"

ProcessingData::ProcessingData(Sheet *sheet)
        : m_sheet(sheet)
{
        if (m_sheet) {
                m_pluginChain = new PluginChain(this, m_sheet);
                set_history_stack(m_sheet->get_history_stack());
        } else {
                m_pluginChain = new PluginChain(this);
        }
        m_fader = m_pluginChain->get_fader();

        connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(audiodevice_params_changed()), Qt::DirectConnection);
}

void ProcessingData::set_input_bus(AudioBus *bus)
{
        THREAD_SAVE_INVOKE_AND_EMIT_SIGNAL(this, bus, private_set_input_bus(AudioBus*), busConfigurationChanged());
}

void ProcessingData::set_output_bus(AudioBus *bus)
{
        THREAD_SAVE_INVOKE_AND_EMIT_SIGNAL(this, bus, private_set_output_bus(AudioBus*), busConfigurationChanged());
}

void ProcessingData::private_set_input_bus(AudioBus* bus)
{
        m_inputBus = bus;
}

void ProcessingData::set_input_bus(const QString &name)
{
//        bool wasArmed=isArmed;
//        if (isArmed)
//                disarm();
//        m_busInName = bus;
//        if (wasArmed) {
//                arm();
//        }

        m_busInName = name;

        AudioBus* inBus = audiodevice().get_capture_bus(m_busInName);
        if (inBus) {
                set_input_bus(inBus);
        }
}

void ProcessingData::set_output_bus(const QString &name)
{
        m_busOutName = name;

        AudioBus* outBus = 0;
        if (m_busOutName == "Master Out") {
                outBus = m_sheet->m_masterSubGroup->get_process_bus();

        } else {
                outBus = audiodevice().get_playback_bus(m_busOutName);
        }

        set_output_bus(outBus);
}

void ProcessingData::private_set_output_bus(AudioBus* bus)
{
        m_outputBus = bus;
}

void ProcessingData::send_to_output_buses(nframes_t nframes, bool applyFaderGain)
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
                        if (applyFaderGain) {
                                Mixer::mix_buffers_with_gain(receiver->get_buffer(nframes), sender->get_buffer(nframes), nframes, get_gain());
                        } else {
                                Mixer::mix_buffers_no_gain(receiver->get_buffer(nframes), sender->get_buffer(nframes), nframes);
                        }
                }

        }

}

void ProcessingData::audiodevice_params_changed()
{
        AudioBus* bus = audiodevice().get_playback_bus(m_busOutName);
        if (bus) {
                m_outputBus = bus;
        }
}

void ProcessingData::set_name( const QString & name )
{
        m_name = name;
        emit stateChanged();
}


void ProcessingData::set_pan(float pan)
{
        if ( pan < -1.0 )
                m_pan=-1.0;
        else
                if ( pan > 1.0 )
                        m_pan=1.0;
                else
                        m_pan=pan;
        emit panChanged();
}



void ProcessingData::set_muted( bool muted )
{
        m_isMuted = muted;
        emit muteChanged(m_isMuted);
        emit audibleStateChanged();
}

Command* ProcessingData::mute()
{
        PENTER;
        set_muted(!m_isMuted);

        return (Command*) 0;
}

Command* ProcessingData::add_plugin( Plugin * plugin )
{
        return m_pluginChain->add_plugin(plugin);
}

Command* ProcessingData::remove_plugin( Plugin * plugin )
{
        return m_pluginChain->remove_plugin(plugin);
}

void ProcessingData::rescan_busses()
{
    QStringList ibus = audiodevice().get_capture_buses_names();
    QStringList obus = audiodevice().get_playback_buses_names();

    // in the worst case, i.e. if no busses are available at all,
    // use the default ones also used in the track's constructor.
    QString fallbackCapture = "Capture 1";
    QString fallbackPlayback = "Master Out";

    // in the less worse case, if at least one bus is available,
    // use it as a fallback
    if (ibus.size()) {
        fallbackCapture = ibus.at(0);
    }

    if (obus.size()) {
        fallbackPlayback = obus.at(0);
    }

    // now let's look for the bus we actually want to connect to
    if (!ibus.contains(m_busInName)) {
        m_busInName = fallbackCapture;
    }

    if (!obus.contains(m_busOutName)) {
        m_busOutName = fallbackPlayback;
    }

    set_input_bus(m_busInName);
    set_output_bus(m_busOutName);
}

void ProcessingData::set_gain(float gain)
{
        if (gain < 0.0)
                gain = 0.0;
        if (gain > 2.0)
                gain = 2.0;
        m_fader->set_gain(gain);
        emit stateChanged();
}
