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

$Id: LV2Plugin.cpp,v 1.3 2006/09/18 18:40:18 r_sijrier Exp $

slv2 url: http://codeson.net/svn/libslv2/
*/



#include "LV2Plugin.h"
#include "LV2ControlPort.h"
#include "AudioInputPort.h"
#include "AudioOutputPort.h"

#include <PluginManager.h>
#include <AudioBus.h>
#include <AudioDevice.h>
#include <Debugger.h>

#define C_(x) x.toAscii().data()
#define UC_(x) (const unsigned char* ) x.toAscii().data()



LV2Plugin::LV2Plugin()
	: Plugin(), m_instance(0), m_plugin(0)
{
}


LV2Plugin::LV2Plugin(char* pluginUri)
	: Plugin(), m_pluginUri((char*) pluginUri), m_instance(0), m_plugin(0)
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
	
	return 1;
}


int LV2Plugin::init()
{
	if (create_instance() < 0) {
		return -1;
	}

	/* Create ports */
	int numPorts  = slv2_plugin_get_num_ports(m_plugin);
	printf("numports is %d\n", (int) numPorts);

	for (int i=0; i < numPorts; ++i) {
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
		PERROR("Plugin %s has no audio input ports set!!", C_(get_name()));
		return -1;
	}

	if (m_audioOutputPorts.size() == 0) {
		PERROR("Plugin %s has no audio output ports set!!", C_(get_name()));
		return -1;
	}
	
	return 1;
}


int LV2Plugin::create_instance()
{
	printf("URI:\t%s\n", C_(m_pluginUri));

	m_plugin = slv2_list_get_plugin_by_uri(PluginManager::instance()->get_slv2_plugin_list(), C_(m_pluginUri));

	
	if (! m_plugin) {
		fprintf(stderr, "Failed to find plugin %s.\n", C_(m_pluginUri));
		return -1;
	}

	/* Get the plugin's name */
	char* name = slv2_plugin_get_name(m_plugin);

	printf("Name:\t%s\n", name);

	/* Instantiate the plugin */
	int samplerate = audiodevice().get_sample_rate();
	m_instance = slv2_plugin_instantiate(m_plugin, samplerate, NULL);

	if (! m_instance) {
		printf("Failed to instantiate plugin.\n");
		return -1;
	} else {
		printf("Succesfully instantiated plugin.\n\n");
	}
	
	return 1;
}


void LV2Plugin::process(AudioBus* bus, unsigned long nframes)
{
	if ( is_bypassed() ) {
		return;
	}
			
	/* Connect the AudioBus Channel buffers to the plugin instance */
// 	for (int index = 0; index < (int)bus->get_channel_count(); ++index) {
	
		slv2_instance_connect_port(m_instance, 1, bus->get_buffer(0, (nframes_t)nframes));
		slv2_instance_connect_port(m_instance, 2, bus->get_buffer(0, (nframes_t)nframes));
// 	}

	/* Run plugin for this cycle */
	slv2_instance_run(m_instance, nframes);
		
		slv2_instance_connect_port(m_instance, 1, bus->get_buffer(1, (nframes_t)nframes));
		slv2_instance_connect_port(m_instance, 2, bus->get_buffer(1, (nframes_t)nframes));
	
	slv2_instance_run(m_instance, nframes);
}


LV2ControlPort* LV2Plugin::create_port(int  portIndex)
{
	LV2ControlPort* port = (LV2ControlPort*) 0;

	/* Make sure this is a float port */
	char* type = (char*) slv2_port_get_data_type(m_plugin, portIndex);

	if (strcmp(type, SLV2_DATA_TYPE_FLOAT)) {
		PERROR("Unrecognized data type.");
		return port;
	}

	free(type);

	/* Get the port symbol (label) for console printing */
	char* symbol = (char*) slv2_port_get_symbol(m_plugin, portIndex);

	/* Get the 'class' of the port (control input, audio output, etc) */
	enum SLV2PortClass portClass = slv2_port_get_class(m_plugin, portIndex);

	/* Create the port based on it's 'class' */
	switch (portClass) {
		case SLV2_CONTROL_RATE_INPUT:
			port = new LV2ControlPort(this, portIndex);
			printf("Set %s to ", symbol);
			break;
		case SLV2_CONTROL_RATE_OUTPUT:
			port = new LV2ControlPort(this, portIndex);
			printf("Set %s to ", symbol);
			break;
		case SLV2_AUDIO_RATE_INPUT:
			m_audioInputPorts.append(new AudioInputPort(this, portIndex));
			break;
		case SLV2_AUDIO_RATE_OUTPUT:
			m_audioOutputPorts.append(new AudioOutputPort(this, portIndex));
			break;
		default:
			PERROR("ERROR: Unknown port type!");
	}

	free(symbol);

	return port;
}

QString LV2Plugin::get_name( )
{
	return QString( (char*) slv2_plugin_get_name(m_plugin));
}

//eof
