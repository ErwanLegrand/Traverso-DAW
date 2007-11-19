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

#include "APILinkedList.h"

class GainEnvelope;
class PluginChain;

class AudioProcessingItem : public APILinkedListNode
{
public:
	AudioProcessingItem () {}
	~AudioProcessingItem () {}
	
	bool is_muted() const {return m_isMuted;}
	virtual bool is_smaller_then(APILinkedListNode* node) = 0;

protected:
	GainEnvelope* m_fader;
	PluginChain* m_pluginChain;
	bool m_isMuted;
};

#endif
