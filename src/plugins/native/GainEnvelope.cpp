/*Copyright (C) 2007 Remon Sijrier

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

#include "GainEnvelope.h"

#include "Song.h"
#include "Curve.h"
#include "Mixer.h"
#include "AudioBus.h"

GainEnvelope::GainEnvelope(Song* song)
	: Plugin(song)
{
	PluginControlPort* port = new PluginControlPort(this, 0, 1.0);
	port->set_index(0);
	m_controlPorts.append(port);
	if (song) {
		set_song(song);
	}
}

QDomNode GainEnvelope::get_state(QDomDocument doc)
{
	QDomElement node = Plugin::get_state(doc).toElement();
	node.setAttribute("type", "GainEnvelope");
	node.setAttribute("gain", m_gain);
	
	return node;
}

int GainEnvelope::set_state(const QDomNode & node)
{
	foreach(PluginControlPort* port, m_controlPorts) {
		delete port;
	}
	m_controlPorts.clear();
	
	Plugin::set_state(node);
	
	QDomElement controlPortsNode = node.firstChildElement("ControlPorts");
	if (!controlPortsNode.isNull()) {
		QDomNode portNode = controlPortsNode.firstChild();
		
		while (!portNode.isNull()) {
			
			PluginControlPort* port = new PluginControlPort(this, portNode);
			m_controlPorts.append(port);
			
			portNode = portNode.nextSibling();
		}
	}
	
	QDomElement e = node.toElement();
	m_gain = e.attribute("gain", "1.0").toFloat();
	
	return 1;
}

void GainEnvelope::process_gain(audio_sample_t* buffer, nframes_t pos, nframes_t nframes)
{
	PluginControlPort* port = m_controlPorts.at(0);
	
	Mixer::apply_gain_to_buffer(buffer, nframes, port->get_control_value());
	
	if (port->use_automation()) {
		port->get_curve()->process(buffer, pos, nframes);
	}
}

QString GainEnvelope::get_name()
{
	return "Gain Envelope";
}

void GainEnvelope::set_song(Song * song)
{
	m_song = song;
	set_history_stack(m_song->get_history_stack());
	if (get_curve()) {
		get_curve()->set_song(song);
	}
}

void GainEnvelope::process(AudioBus * bus, unsigned long nframes)
{
	Q_UNUSED(bus)
	Q_UNUSED(nframes)
}

Curve * GainEnvelope::get_curve() const
{
	return m_controlPorts.at(0)->get_curve();
}

