/*
Copyright (C) 2007 Remon Sijrier

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

#ifndef AUDIO_PROCESSING_ITEM_H
#define AUDIO_PROCESSING_ITEM_H

#include "ContextItem.h"
#include "APILinkedList.h"
#include "GainEnvelope.h"
#include "defines.h"

class PluginChain;
class AudioBus;
class Sheet;

class AudioProcessingItem : public ContextItem, public APILinkedListNode
{
        Q_OBJECT
public:
        AudioProcessingItem (Sheet* sheet=0);
	~AudioProcessingItem () {}
	
        void set_input_bus(AudioBus* bus);
        void set_output_bus(AudioBus* bus);

        void set_output_bus_name(const QString& name);

        void send_to_output_buses(nframes_t nframes, bool applyFaderGain=true);

        AudioBus* get_process_bus() {return m_processBus;}
        PluginChain* get_plugin_chain() const {return m_pluginChain;}
        QString get_name() const {return m_name;}
        void set_name(const QString& name);


	bool is_muted() const {return m_isMuted;}
	virtual bool is_smaller_then(APILinkedListNode* node) = 0;

protected:
        AudioBus*       m_inputBus;
        AudioBus*       m_outputBus;
        AudioBus*       m_processBus;
        Sheet*          m_sheet;
        GainEnvelope*   m_fader;
        PluginChain*    m_pluginChain;
        QString         m_busInName;
        QString         m_busOutName;
        QString		m_name;


        bool            m_isMuted;

public slots:
	float get_gain() const;

private slots:
        void audiodevice_params_changed();
        void private_set_input_bus(AudioBus*);
        void private_set_output_bus(AudioBus*);

signals:
        void busConfigurationChanged();
        void stateChanged();
};

inline float AudioProcessingItem::get_gain( ) const
{
	return m_fader->get_gain();
}


#endif
