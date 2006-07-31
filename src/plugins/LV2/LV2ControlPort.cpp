/*Copyright (C) 2006 Remon Sijrier

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

$Id: LV2ControlPort.cpp,v 1.1 2006/07/31 13:24:46 r_sijrier Exp $

slv2 url: http://codeson.net/svn/libslv2/
*/



#include "LV2ControlPort.h"
#include "LV2Plugin.h"


LV2ControlPort::LV2ControlPort(LV2Plugin* plugin, int index)
	: PluginPort(index), m_plugin(plugin)
{
	m_controlValue = slv2_port_get_default_value(m_plugin->get_slv2_plugin(), m_index);
	slv2_instance_connect_port(m_plugin->get_instance(), m_index, &m_controlValue);

	printf(" %f\n", m_controlValue);
}

LV2ControlPort::LV2ControlPort( LV2Plugin * plugin, const QDomNode node )
	: PluginPort(), m_plugin(plugin)
{
	set_state(node);
	
	slv2_instance_connect_port(m_plugin->get_instance(), m_index, &m_controlValue);
}


QDomNode LV2ControlPort::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("ControlPort");
	node.setAttribute("index", (int) m_index);
	node.setAttribute("value", m_controlValue);
	
	return node;
}


int LV2ControlPort::set_state( const QDomNode & node )
{
	QDomElement e = node.toElement();
	m_index = (unsigned long) e.attribute( "index", "").toInt();
	m_controlValue = e.attribute( "value", "").toFloat();
	
	return 1;
}


float LV2ControlPort::get_min_control_value()
{
	return slv2_port_get_minimum_value (m_plugin->get_slv2_plugin(), m_index);
}

float LV2ControlPort::get_max_control_value()
{
	return slv2_port_get_maximum_value (m_plugin->get_slv2_plugin(), m_index);

}

QString LV2ControlPort::get_description()
{
	SLV2Property prop = slv2_port_get_property(m_plugin->get_slv2_plugin(), m_index, (const uchar*) "lv2:symbol");

        if (prop && prop->num_values == 1) {
		return QString((char*)prop->values[0]);
	}
	
	return QString("no desc. avail.");
}


/******* SLOTS ********/

void LV2ControlPort::set_control_value(double value)
{
	m_controlValue = value;
}

//eof
