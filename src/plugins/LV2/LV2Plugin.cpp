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



LV2Plugin::LV2Plugin(Song* song, bool slave)
	: Plugin(song)
	, m_instance(0)
	, m_slv2plugin(0)
{
	m_isSlave = slave;
}


LV2Plugin::LV2Plugin(Song* song, char* pluginUri)
	: Plugin(song)
	, m_pluginUri((char*) pluginUri)
	, m_instance(0)
	, m_slv2plugin(0)
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
	QDomElement node = Plugin::get_state(doc).toElement();
	
	node.setAttribute("type", "LV2Plugin");
	node.setAttribute("uri", m_pluginUri);
	
	return node;
}


int LV2Plugin::set_state(const QDomNode & node )
{
	Plugin::set_state(node);
	
	QDomElement e = node.toElement();
	
	m_pluginUri = e.attribute( "uri", "");
	
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
	int portcount  = slv2_plugin_get_num_ports(m_slv2plugin);
	PMESG("numports is %d", portcount);

	for (int i=0; i < portcount; ++i) {
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
	// TODO check if newer versions of slv2 DO NOT REQUIRE THIS CALL
	// TO SUCCESFULLY INSTANTIATE THE PLUGIN !!!!!!!!!!!!!!!!!!
	char* name = slv2_plugin_get_name(m_slv2plugin);
// 	printf("Name:\t%s\n", name);
	Q_UNUSED(name);
	
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

	/* Get the 'direction' of the port (input, output) */
	SLV2PortDirection portDirection = slv2_port_get_direction(m_slv2plugin, slvport);

	/* Get the 'data type' of the port (control, audio) */
	SLV2PortDataType portDataType = slv2_port_get_data_type(m_slv2plugin, slvport);	
	
	/* Create the port based on it's 'direction' and 'data type' */
	switch (portDataType) {
		case SLV2_PORT_DATA_TYPE_CONTROL:
			switch (portDirection) {
			case SLV2_PORT_DIRECTION_INPUT:
					ctrlport = new LV2ControlPort(this, portIndex, slv2_port_get_default_value(m_slv2plugin, slvport));
					break;
			case SLV2_PORT_DIRECTION_OUTPUT:
					ctrlport = new LV2ControlPort(this, portIndex, 0);
					break;
			}
		case SLV2_PORT_DATA_TYPE_AUDIO:
			switch (portDirection) {
			case SLV2_PORT_DIRECTION_INPUT:
					m_audioInputPorts.append(new AudioInputPort(this, portIndex));
					break;
			case SLV2_PORT_DIRECTION_OUTPUT:
					m_audioOutputPorts.append(new AudioOutputPort(this, portIndex));
					break;
			}
		default:
			PERROR("ERROR: Unknown port data type!");
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
	: PluginControlPort(plugin, index, value)
	, m_lv2plugin(plugin)
{
	slv2_instance_connect_port(m_lv2plugin->get_instance(), m_index, &m_value);
	init();
}

LV2ControlPort::LV2ControlPort( LV2Plugin * plugin, const QDomNode node )
	: PluginControlPort(plugin, node)
	, m_lv2plugin(plugin)
{
	slv2_instance_connect_port(m_lv2plugin->get_instance(), m_index, &m_value);
	init();
}

void LV2ControlPort::init()
{
	foreach(QString string, get_hints()) {
		if (string == "http://lv2plug.in/ns/lv2core#logarithmic") {
			m_hint = LOG_CONTROL;
		} else  if (string == "http://lv2plug.in/ns/lv2core#integer") {
			m_hint = INT_CONTROL;
		}
	}
}

QDomNode LV2ControlPort::get_state( QDomDocument doc )
{
	return PluginControlPort::get_state(doc);
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

float LV2ControlPort::get_default_value()
{
	SLV2Port port = slv2_plugin_get_port_by_index(m_lv2plugin->get_slv2_plugin(), m_index);
	return slv2_port_get_default_value (m_lv2plugin->get_slv2_plugin(), port);
}


QString LV2ControlPort::get_description()
{
	SLV2Port port = slv2_plugin_get_port_by_index(m_lv2plugin->get_slv2_plugin(), m_index);
	return QString(slv2_port_get_name(m_lv2plugin->get_slv2_plugin(), port));
}

QString LV2ControlPort::get_symbol()
{
	SLV2Port port = slv2_plugin_get_port_by_index(m_lv2plugin->get_slv2_plugin(), m_index);
	return QString(slv2_port_get_symbol(m_lv2plugin->get_slv2_plugin(), port));
}

QStringList LV2ControlPort::get_hints()
{
	SLV2Port port = slv2_plugin_get_port_by_index(m_lv2plugin->get_slv2_plugin(), m_index);
	SLV2Values values = slv2_port_get_hints(m_lv2plugin->get_slv2_plugin(), port);
	QStringList qslist;
	for (unsigned i=0; i < slv2_values_size(values); ++i) {
		qslist << QString(slv2_value_as_string(slv2_values_get_at(values, i)));
	}
	return qslist;
}

LV2Plugin * LV2Plugin::create_copy()
{
	QDomDocument doc("LV2Plugin");
	QDomNode pluginState = get_state(doc);
	LV2Plugin* plug = new LV2Plugin(m_song, true);
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

PluginInfo LV2Plugin::get_plugin_info(SLV2Plugin plugin)
{
	PluginInfo info;
	info.name = slv2_plugin_get_name(plugin);
	info.uri = slv2_plugin_get_uri(plugin);
	
	
	int portcount  = slv2_plugin_get_num_ports(plugin);

	for (int i=0; i < portcount; ++i) {
		SLV2Port slvport = slv2_plugin_get_port_by_index(plugin, i);
		SLV2PortDirection portDirection = slv2_port_get_direction(plugin, slvport);
		SLV2PortDataType portDataType = slv2_port_get_data_type(plugin, slvport);
		switch (portDataType) {
		case SLV2_PORT_DATA_TYPE_AUDIO:
			switch (portDirection) {
			case SLV2_PORT_DIRECTION_INPUT:
					info.audioPortInCount++;
					continue;
			case SLV2_PORT_DIRECTION_OUTPUT:
					info.audioPortOutCount++;
				continue;
			}
		case SLV2_PORT_DATA_TYPE_CONTROL: break;
		case SLV2_PORT_DATA_TYPE_MIDI: break;
		case SLV2_PORT_DATA_TYPE_OSC: break;
		case SLV2_PORT_DATA_TYPE_UNKNOWN: break;
		}
	}
	
	SLV2Values values =  slv2_plugin_get_value(plugin, SLV2_QNAME, "a");
	for (unsigned i=0; i < slv2_values_size(values); ++i) {
		QString type =  slv2_value_as_string(slv2_values_get_at(values, i));
		if (type.contains("http://lv2plug.in/ns/lv2core#")) {
			info.type = type.remove("http://lv2plug.in/ns/lv2core#");
			break;
		}
	}
	
	return info;
}

QString LV2Plugin::plugin_type(const QString & uri)
{
	SLV2Plugin plugin = slv2_plugins_get_by_uri(PluginManager::instance()->get_slv2_plugin_list(), QS_C(uri));
	
	// TODO WHY THE HACK DO I NEED TO CALL THIS TO BE ABLE TO QUERY PLUGIN RDF DATA ????
	char* name = slv2_plugin_get_name(plugin);
	Q_UNUSED(name);
	SLV2Values values =  slv2_plugin_get_value(plugin, SLV2_QNAME, "a");
	for (unsigned i=0; i < slv2_values_size(values); ++i) {
		QString type =  slv2_value_as_string(slv2_values_get_at(values, i));
		if (type.contains("http://lv2plug.in/ns/lv2core#")) {
			return type.remove("http://lv2plug.in/ns/lv2core#");
		}
	}
	
	return "";
}

//eof

