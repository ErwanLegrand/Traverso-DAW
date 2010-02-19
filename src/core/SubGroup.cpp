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
#include "Utils.h"

SubGroup::SubGroup(Sheet* sheet, const QString& name, int channelCount)
        : Track(sheet)
{
        QObject::tr("SubGroup");
        m_sortIndex = -1;
        m_height = 60;
        m_id = create_id();
        m_name = name;
        m_channelCount = channelCount;
        m_isSolo = mutedBySolo = m_isMuted = false;
        m_busOutName = "Playback 1";
        m_fader->set_gain(1.0);

        init();
}

SubGroup::SubGroup(Sheet *sheet, QDomNode /*node*/)
        : Track(sheet)
{
}

SubGroup::~SubGroup()
{
        delete m_processBus;
}

QDomNode SubGroup::get_state( QDomDocument doc, bool /*istemplate*/)
{
        QDomElement node = doc.createElement("SubGroup");
        Track::get_state(node);

        node.setAttribute("channelcount", m_channelCount);

        return node;
}

int SubGroup::set_state( const QDomNode & node )
{
        QDomElement e = node.toElement();

        Track::set_state(node);

        set_output_bus(e.attribute("OutBus", "Playback 1"));

        bool ok;
        m_channelCount = e.attribute("channelcount", "2").toInt(&ok);

        init();

        return 1;
}

void SubGroup::init()
{
        m_processBus = new AudioBus(m_name, m_channelCount, ChannelIsOutput);
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

void SubGroup::set_height(int h)
{
        m_height = h;
        if (m_height > 60) {
                m_height = 60;
        }
        emit heightChanged();
}
