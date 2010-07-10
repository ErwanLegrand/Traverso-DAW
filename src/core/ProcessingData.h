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

#ifndef PROCESSING_DATA_H
#define PROCESSING_DATA_H

#include "ContextItem.h"
#include "APILinkedList.h"
#include "GainEnvelope.h"
#include "defines.h"

class AudioBus;
class AudioClip;
class Plugin;
class PluginChain;
class TSession;


class ProcessingData : public ContextItem, public APILinkedListNode
{
        Q_OBJECT

        Q_CLASSINFO("mute", tr("Mute"))

public:
        ProcessingData (TSession* session=0);
        virtual ~ProcessingData () {}
	
        Command* add_plugin(Plugin* plugin);
        Command* remove_plugin(Plugin* plugin);

        PluginChain* get_plugin_chain() const {return m_pluginChain;}
        TSession* get_session() const {return m_session;}
        QString get_name() const {return m_name;}
        float get_pan() const {return m_pan;}

        void set_muted(bool muted);
        virtual void set_name(const QString& name);
        void set_pan(float pan);

        bool is_muted() const {return m_isMuted;}
        virtual bool is_smaller_then(APILinkedListNode* node) = 0;



protected:

        AudioBus*       m_processBus;
        TSession*       m_session;
        GainEnvelope*   m_fader;
        PluginChain*    m_pluginChain;
        QString		m_name;
        bool            m_isMuted;
        float           m_pan;



public slots:
	float get_gain() const;
        void set_gain(float gain);
        Command* mute();


signals:
        void audibleStateChanged();
        void stateChanged();
        void muteChanged(bool isMuted);
        void panChanged();
};

inline float ProcessingData::get_gain( ) const
{
	return m_fader->get_gain();
}


#endif
