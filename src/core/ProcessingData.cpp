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

#include "AudioClip.h"
#include "AudioClipManager.h"
#include "PluginChain.h"
#include "TSession.h"
#include "limits"
#include "Mixer.h"


#include "Debugger.h"

ProcessingData::ProcessingData(TSession *session)
        : ContextItem(session)
        , m_session(session)
{
        if (m_session) {
                m_pluginChain = new PluginChain(this, m_session);
                set_history_stack(m_session->get_history_stack());
        } else {
                m_pluginChain = new PluginChain(this);
        }

        m_processBus = 0;
        m_isMuted = false;
        m_pan = 0.0f;
        m_fader = m_pluginChain->get_fader();
}


void ProcessingData::set_name( const QString & name )
{
        m_name = name;
        emit stateChanged();
}


void ProcessingData::set_pan(float pan)
{
        if ( pan < -1.0 ) {
                m_pan=-1.0;
        } else {
                if ( pan > 1.0 ) {
                        m_pan=1.0;
                } else {
                        m_pan=pan;
                }
        }

        if (fabs(pan) < std::numeric_limits<float>::epsilon()) {
                m_pan = 0.0f;
        }

        emit panChanged();
}



void ProcessingData::set_muted( bool muted )
{
        m_isMuted = muted;
        emit muteChanged(m_isMuted);
        emit audibleStateChanged();
}

TCommand* ProcessingData::mute()
{
        PENTER;
        set_muted(!m_isMuted);

        return (TCommand*) 0;
}

TCommand* ProcessingData::add_plugin( Plugin * plugin )
{
        return m_pluginChain->add_plugin(plugin);
}

TCommand* ProcessingData::remove_plugin( Plugin * plugin )
{
        return m_pluginChain->remove_plugin(plugin);
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
