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

#include "SubGroup.h"

#include "AudioBus.h"
#include "PluginChain.h"

SubGroup::SubGroup(Sheet* sheet, const QString& name, int channelCount)
        : Track(sheet)
{
        m_processBus = new AudioBus(name, channelCount, ChannelIsOutput);
}


SubGroup::~SubGroup()
{
        delete m_processBus;
}

int SubGroup::process(nframes_t nframes)
{
        if (m_isMuted || (get_gain() == 0.0f) ) {
                return 0;
        }

        m_pluginChain->process_post_fader(m_processBus, nframes);
        send_to_output_buses(nframes);

        return 1;
}
