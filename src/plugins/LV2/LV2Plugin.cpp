/*Copyright (C) 2006-2007 Remon Sijrier

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



#include "LV2Plugin.h"

#include <PluginManager.h>
#include <AudioBus.h>
#include <AudioDevice.h>
#include <Utils.h>

#include <Debugger.h>

#define UC_(x) (const unsigned char* ) x.toAscii().data()



LV2Plugin::LV2Plugin(bool slave)
	: Plugin()
	, m_instance(0)
	, m_slv2plugin(0)
	, m_slave(0)
{
	m_isSlave = slave;
}


LV2Plugin::LV2Plugin(char* pluginUri)
	: Plugin()
	, m_pluginUri((char*) pluginUri)
	, m_instance(0)
	, m_slv2plugin(0)
	, m_slave(0)
	, m_isSlave(false)
{
}


LV2Plugin::~LV2Plugin()
{
	/* Deactivate and free plugin instance */
	if (m_instance) {
		slv2_instance_deactivate(m_instance);
		slv2_instance_free(m_instance);
	}
}


QDomNode LV2Plugin::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("Plugin");
	node.setAttribute("type", "LV2Plugin");
	node.setAttribute("uri", m_pluginUri);
	node.setAttribute("bypassed", is_bypassed());
	
	QDomNode controlPortsNode = doc.createElement("ControlPorts");
	
	foreach(LV2ControlPort* port, m_controlPorts) {
		controlPortsNode.appendChild(port->get_state(doc));
	}
	
	QDomNode audioInputPortsNode = doc.createElement("AudioInputPorts");
	foreach(AudioInputPort* port, m_audioInputPorts) {
		audioInputPortsNode.appendChild(port->get_state(doc));
	}
	
	QDomNode audioOutputPortsNode = doc.createElement("AudioOutputPorts");
	foreach(AudioOutputPort* port, m_audioOutputPorts) {
		audioOutputPortsNode.appendChild(port->get_state(doc));
	}
	
	node.appendChild(controlPortsNode);
	node.appendChild(audioInputPortsNode);
	node.appendChild(audioOutputPortsNode);
	
	return node;
}


int LV2Plugin::set_state(const QDomNode & node )
{
	QDomElement e = node.toElement();
	
	m_pluginUri = e.attribute( "uri", "");
	m_bypass = e.attribute( "bypassed", "0").toInt();
	
	if (create_instance() < 0) {
		return -1;
	}
	
	/* Create control ports */
	QDomElement controlPortsNode = node.firstChildElement("ControlPorts");
	if (!controlPortsNode.isNull()) {
		QDomNode portNode = controlPortsNode.firstChild();
		
		while (!portNode.isNull()) {
			
			LV2ControlPort* port = new LV2ControlPort(this, portNode);
			m_controlPorts.append(port);
			
			portNode = portNode.nextSibling();
		}
	}
		
	// Create audio input ports 	
	QDomElement audioInputPortsNode = node.firstChildElement("AudioInputPorts");
	if (!audioInputPortsNode.isNull()) {
		QDomNode portNode = audioInputPortsNode.firstChild();
		
		while (!portNode.isNull()) {
			AudioInputPort* port = new AudioInputPort(this);
			port->set_state(portNode);
			m_audioInputPorts.append(port);
			
			portNode = portNode.nextSibling();
		}
	}
	
	// Create audio ouput ports
	QDomElement audioOutputPortsNode = node.firstChildElement("AudioOutputPorts");
	if (!audioOutputPortsNode.isNull()) {
		QDomNode portNode = audioOutputPortsNode.firstChild();
		
		while (!portNode.isNull()) {
			AudioOutputPort* port = new AudioOutputPort(this);
			port->set_state(portNode);
			m_audioOutputPorts.append(port);
			
			portNode = portNode.nextSibling();
		}
	}
	
	/* Activate the plugin instance */
	slv2_instance_activate(m_instance);
	
	// The default is 2 channels in - out, if there is only 1 in - out, duplicate
	// this plugin, and use it on the second channel
	if (!m_isSlave && m_audioInputPorts.size() == 1 && m_audioOutputPorts.size() == 1) {
		m_slave = create_copy();
	}
	
	return 1;
}


int LV2Plugin::init()
{
	if (create_instance() < 0) {
		return -1;
	}

	/* Create ports */
	m_portcount  = slv2_plugin_get_num_ports(m_slv2plugin);
	PMESG("numports is %d", (int) m_portcount);

	for (int i=0; i < m_portcount; ++i) {
		LV2ControlPort* port = create_port(i);
		if (port) {
			m_controlPorts.append(port);
		} else {
			// Not an audio port, or unrecognized port type
		}
	}

	/* Activate the plugin instance */
	slv2_instance_activate(m_instance);
	
	if (m_audioInputPorts.size() == 0) {
		PERROR("Plugin %s has no audio input ports set!!", QS_C(get_name()));
		return -1;
	}

	if (m_audioOutputPorts.size() == 0) {
		PERROR("Plugin %s has no audio output ports set!!", QS_C(get_name()));
		return -1;
	}
	
	// The default is 2 channels in - out, if there is only 1 in - out, duplicate
	// this plugin, and use it on the second channel
	if (!m_isSlave && m_audioInputPorts.size() == 1 && m_audioOutputPorts.size() == 1) {
		m_slave = create_copy();
	}
	
	return 1;
}


int LV2Plugin::create_instance()
{
// 	printf("URI:\t%s\n", QS_C(m_pluginUri));

	m_slv2plugin = slv2_plugins_get_by_uri(PluginManager::instance()->get_slv2_plugin_list(), QS_C(m_pluginUri));

	
	if (! m_slv2plugin) {
		fprintf(stderr, "Failed to find plugin %s.\n", QS_C(m_pluginUri));
		return -1;
	}

	/* Get the plugin's name */
	char* name = slv2_plugin_get_name(m_slv2plugin);

// 	printf("Name:\t%s\n", name);

	/* Instantiate the plugin */
	int samplerate = audiodevice().get_sample_rate();
	m_instance = slv2_plugin_instantiate(m_slv2plugin, samplerate, NULL);

	if (! m_instance) {
		printf("Failed to instantiate plugin.\n");
		return -1;
	} else {
// 		printf("Succesfully instantiated plugin.\n\n");
	}
	
	return 1;
}


void LV2Plugin::process(AudioBus* bus, unsigned long nframes)
{
	if ( is_bypassed() ) {
		return;
	}
	
	for (int i=0; i<m_audioInputPorts.size(); ++i) {
		AudioInputPort* port = m_audioInputPorts.at(i);
		int index = port->get_index();
		// If we are a slave, then we are meant to operate on the second channel of the Bus!
		if (m_isSlave) i = 1;
		slv2_instance_connect_port(m_instance, index, bus->get_buffer(i, (nframes_t)nframes));
	}
	
	for (int i=0; i<m_audioOutputPorts.size(); ++i) {
		AudioOutputPort* port = m_audioOutputPorts.at(i);
		int index = port->get_index();
		// If we are a slave, then we are meant to operate on the second channel of the Bus!
		if (m_isSlave) i = 1;
		slv2_instance_connect_port(m_instance, index, bus->get_buffer(i, (nframes_t)nframes));
	}
	
	/* Run plugin for this cycle */
	slv2_instance_run(m_instance, nframes);
	
	// If we have a slave, and the bus has 2 channels, process the slave too!
	if (m_slave && bus->get_channel_count() == 2) {
		m_slave->process(bus, nframes);
	}	
	
}


LV2ControlPort* LV2Plugin::create_port(int portIndex)
{
	LV2ControlPort* ctrlport = (LV2ControlPort*) 0;

	
	SLV2Port slvport = slv2_plugin_get_port_by_index(m_slv2plugin, portIndex);
	
	/* Get the port symbol (label) for console printing */
	char* symbol = slv2_port_get_symbol(m_slv2plugin, slvport);

	/* Get the 'class' of the port (control input, audio output, etc) */
	SLV2PortClass portClass = slv2_port_get_class(m_slv2plugin, slvport);
	
	/* Create the port based on it's 'class' */
	switch (portClass) {
		case SLV2_CONTROL_INPUT:
			ctrlport = new LV2ControlPort(this, portIndex, slv2_port_get_default_value(m_slv2plugin, slvport));
			break;
		case SLV2_CONTROL_OUTPUT:
			ctrlport = new LV2ControlPort(this, portIndex, 0);
			break;
		case SLV2_AUDIO_INPUT:
			m_audioInputPorts.append(new AudioInputPort(this, portIndex));
			break;
		case SLV2_AUDIO_OUTPUT:
			m_audioOutputPorts.append(new AudioOutputPort(this, portIndex));
			break;
		default:
			PERROR("ERROR: Unknown port type!");
	}

	free(symbol);

	return ctrlport;
}

QString LV2Plugin::get_name( )
{
	return QString(slv2_plugin_get_name(m_slv2plugin));
}




/*********************************************************/
/*		LV2 Control Port			 */
/*********************************************************/


LV2ControlPort::LV2ControlPort(LV2Plugin* plugin, int index, float value)
	: PluginPort(plugin, index)
	, m_lv2plugin(plugin)
	, m_controlValue(value)
{
	slv2_instance_connect_port(m_lv2plugin->get_instance(), m_index, &m_controlValue);
}

LV2ControlPort::LV2ControlPort( LV2Plugin * plugin, const QDomNode node )
	: PluginPort(plugin), m_lv2plugin(plugin)
{
	set_state(node);
	
	slv2_instance_connect_port(m_lv2plugin->get_instance(), m_index, &m_controlValue);
}


QDomNode LV2ControlPort::get_state( QDomDocument doc )
{
	
	QDomElement node = PluginPort::get_state(doc).toElement();
	node.setAttribute("value", m_controlValue);
	
	return node;
}


int LV2ControlPort::set_state( const QDomNode & node )
{
	QDomElement e = node.toElement();
	m_index = e.attribute( "index", "").toInt();
	m_controlValue = e.attribute( "value", "").toFloat();
	
	return 1;
}


float LV2ControlPort::get_min_control_value()
{
	SLV2Port port = slv2_plugin_get_port_by_index(m_lv2plugin->get_slv2_plugin(), m_index);
	return slv2_port_get_minimum_value (m_lv2plugin->get_slv2_plugin(), port);
}

float LV2ControlPort::get_max_control_value()
{
	SLV2Port port = slv2_plugin_get_port_by_index(m_lv2plugin->get_slv2_plugin(), m_index);
	return slv2_port_get_maximum_value (m_lv2plugin->get_slv2_plugin(), port);

}

QString LV2ControlPort::get_description()
{
	SLV2Port port = slv2_plugin_get_port_by_index(m_lv2plugin->get_slv2_plugin(), m_index);
	return QString(slv2_port_get_symbol(m_lv2plugin->get_slv2_plugin(), port));
}

void LV2ControlPort::set_control_value(float value)
{
	m_controlValue = value;
}


LV2ControlPort * LV2Plugin::get_control_port_by_index(int index) const
{
	foreach(LV2ControlPort* port, m_controlPorts) {
		if (port->get_index() == index) {
			return port;
		}
	}
	return 0;
}

LV2Plugin * LV2Plugin::create_copy()
{
	QDomDocument doc("LV2Plugin");
	QDomNode pluginState = get_state(doc);
	LV2Plugin* plug = new LV2Plugin(true);
	plug->set_state(pluginState);
	return plug;
}

Command * LV2Plugin::toggle_bypass()
{
	Plugin::toggle_bypass();
	if (m_slave) {
		m_slave->toggle_bypass();
	}
	return  0;
}

//eof

