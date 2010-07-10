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

#include "TBusTrack.h"

#include "AudioBus.h"
#include "PluginChain.h"
#include "Utils.h"

TBusTrack::TBusTrack(Sheet* sheet, const QString& name, int channelCount)
        : Track(sheet)
{
        QObject::tr("SubGroup");
        m_height = 60;
        m_id = create_id();
        m_name = name;
        m_channelCount = channelCount;
        m_fader->set_gain(1.0);

        create_process_bus();

        m_processBus->set_id(m_id);
        m_processBus->set_name(m_name);
}

TBusTrack::TBusTrack(Sheet *sheet, QDomNode /*node*/)
        : Track(sheet)
{
}

TBusTrack::~TBusTrack()
{
        delete m_processBus;
}

QDomNode TBusTrack::get_state( QDomDocument doc, bool istemplate)
{
        QDomElement node = doc.createElement("SubGroup");
        Track::get_state(doc, node, istemplate);

        node.setAttribute("channelcount", m_channelCount);

        return node;
}

int TBusTrack::set_state( const QDomNode & node )
{
        QDomElement e = node.toElement();

        Track::set_state(node);

        bool ok;
        m_channelCount = e.attribute("channelcount", "2").toInt(&ok);


        create_process_bus();

        m_processBus->set_id(m_id);
        m_processBus->set_name(m_name);

        return 1;
}

void TBusTrack::create_process_bus()
{
        // avoid creating the bus twice!
        if (m_processBus) {
                return;
        }
        m_type = SUBGROUP;
        BusConfig busConfig;
        busConfig.name = m_name;
        busConfig.channelcount = m_channelCount;
        busConfig.type = "output";
        busConfig.isInternalBus = true;
        busConfig.id = m_id;
        m_processBus = new AudioBus(busConfig);
}

int TBusTrack::process(nframes_t nframes)
{
        if (m_isMuted || (get_gain() == 0.0f) ) {
                return 0;
        }

        process_pre_sends(nframes);

        m_pluginChain->process_pre_fader(m_processBus, nframes);

        m_fader->process(m_processBus, nframes);

        float panFactor;

        if ( (m_processBus->get_channel_count() >= 1) && (m_pan > 0) )  {
                panFactor = 1 - m_pan;
                Mixer::apply_gain_to_buffer(m_processBus->get_buffer(0, nframes), nframes, panFactor);
        }

        if ( (m_processBus->get_channel_count() >= 2) && (m_pan < 0) )  {
                panFactor = 1 + m_pan;
                Mixer::apply_gain_to_buffer(m_processBus->get_buffer(1, nframes), nframes, panFactor);
        }


        m_pluginChain->process_post_fader(m_processBus, nframes);

        m_processBus->process_monitoring(m_vumonitors);

        process_post_sends(nframes);

        m_processBus->silence_buffers(nframes);

        return 1;
}
