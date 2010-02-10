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

class PluginChain;
class AudioBus;
class Sheet;

class AudioProcessingItem : public ContextItem, public APILinkedListNode
{
        Q_OBJECT
public:
        AudioProcessingItem (Sheet* sheet=0) : m_sheet(sheet) {}
	~AudioProcessingItem () {}
	
        void set_input_bus(AudioBus* bus);
        void set_output_bus(AudioBus* bus);

	bool is_muted() const {return m_isMuted;}
	virtual bool is_smaller_then(APILinkedListNode* node) = 0;

protected:
        AudioBus*       m_inputBus;
        AudioBus*       m_outputBus;
        Sheet*          m_sheet;
        GainEnvelope*   m_fader;
        PluginChain*    m_pluginChain;
        bool            m_isMuted;
        
public slots:
	float get_gain() const;

private slots:
        void private_set_input_bus(AudioBus*);
        void private_set_output_bus(AudioBus*);

signals:
        void busConfigurationChanged();

};

inline float AudioProcessingItem::get_gain( ) const
{
	return m_fader->get_gain();
}


#endif
