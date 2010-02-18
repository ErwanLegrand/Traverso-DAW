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
class Sheet;


class ProcessingData : public ContextItem, public APILinkedListNode
{
        Q_OBJECT

        Q_CLASSINFO("mute", tr("Mute"))

public:
        ProcessingData (Sheet* sheet=0);
        ~ProcessingData () {}
	
        void set_input_bus(AudioBus* bus);
        void set_output_bus(AudioBus* bus);

        void set_input_bus(const QString& name);
        void set_output_bus(const QString& name);

        void send_to_output_buses(nframes_t nframes, bool applyFaderGain=true);


        Command* add_plugin(Plugin* plugin);
        Command* remove_plugin(Plugin* plugin);

        AudioBus* get_process_bus() {return m_processBus;}
        PluginChain* get_plugin_chain() const {return m_pluginChain;}
        Sheet* get_sheet() const {return m_sheet;}
        QString get_name() const {return m_name;}
        QString get_bus_in_name() const {return m_busInName;}
        QString get_bus_out_name() const{return m_busOutName;}
        float get_pan() const {return m_pan;}

        void set_muted(bool muted);
        void set_name(const QString& name);
        void set_pan(float pan);

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
        float           m_pan;


public slots:
	float get_gain() const;
        void set_gain(float gain);
        Command* mute();

private slots:
        void audiodevice_params_changed();
        void private_set_input_bus(AudioBus*);
        void private_set_output_bus(AudioBus*);    
        void rescan_busses();


signals:
        void busConfigurationChanged();
        void stateChanged();
        void audibleStateChanged();
        void panChanged();

        void heightChanged();
        void muteChanged(bool isMuted);

};

inline float ProcessingData::get_gain( ) const
{
	return m_fader->get_gain();
}


#endif
