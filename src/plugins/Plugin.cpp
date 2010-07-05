/*
Copyright (C) 2006-2007 Remon Sijrier

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

#include "Plugin.h"

#include "AddRemove.h"
#include "Curve.h"
#include "TSession.h"
#include "Sheet.h"

#include "Debugger.h"

Plugin::Plugin(TSession* session)
	: m_slave(0)
        , m_session(session)
{
	m_bypass = false;
}

QDomNode Plugin::get_state(QDomDocument doc)
{
	QDomElement node = doc.createElement("Plugin");
	
	node.setAttribute("bypassed", is_bypassed());
	
	QDomNode controlPortsNode = doc.createElement("ControlPorts");
	foreach(PluginControlPort* port, m_controlPorts) {
		controlPortsNode.appendChild(port->get_state(doc));
	}
	
	if (m_audioInputPorts.size() > 0) {
		QDomNode audioInputPortsNode = doc.createElement("AudioInputPorts");
		foreach(AudioInputPort* port, m_audioInputPorts) {
			audioInputPortsNode.appendChild(port->get_state(doc));
		}
		node.appendChild(audioInputPortsNode);
	}
	
	if (m_audioOutputPorts.size() > 0) {
		QDomNode audioOutputPortsNode = doc.createElement("AudioOutputPorts");
		foreach(AudioOutputPort* port, m_audioOutputPorts) {
			audioOutputPortsNode.appendChild(port->get_state(doc));
		}
		node.appendChild(audioOutputPortsNode);
	}
	
	node.appendChild(controlPortsNode);
	
	return node;
}

int Plugin::set_state(const QDomNode & node)
{
	QDomElement e = node.toElement();
	
	m_bypass = e.attribute( "bypassed", "0").toInt();

	return 1;
}

Command* Plugin::toggle_bypass( )
{
	m_bypass = ! m_bypass;
	
	emit bypassChanged();
	
	return (Command*) 0;
}

PluginControlPort* Plugin::get_control_port_by_index(int index) const
{
	foreach(PluginControlPort* port, m_controlPorts) {
		if (port->get_index() == index) {
			return port;
		}
	}
	return 0;
}

void Plugin::automate_port(int index, bool automate)
{
	PluginControlPort* port = get_control_port_by_index(index);
	if (!port) {
		PERROR("ControlPort with index %d does not exist", index);
		return;
	}
	port->set_use_automation(automate);
}


QDomNode PluginPort::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("ControlPort");
	node.setAttribute("index", (int) m_index);
	
	return node;
} 

int PluginPort::set_state( const QDomNode & node )
{
	Q_UNUSED(node);
	return 1;
}

AudioInputPort::AudioInputPort(QObject* parent, int index)
	: PluginPort(parent, index)
{
}

QDomNode AudioInputPort::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("AudioInputPort");
	node.setAttribute("index", m_index);
	
	return node;
}

int AudioInputPort::set_state( const QDomNode & node )
{
	QDomElement e = node.toElement();
	
	m_index = e.attribute( "index", "-1").toInt();
	
	return 1;
}

AudioOutputPort::AudioOutputPort(QObject* parent, int index)
	: PluginPort(parent, index)
{
}

QDomNode AudioOutputPort::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("AudioOutputPort");
	node.setAttribute("index", (int) m_index);
	
	return node;
}

int AudioOutputPort::set_state( const QDomNode & node )
{
	QDomElement e = node.toElement();
	
	m_index = e.attribute( "index", "-1").toInt();
	
	return 1;
}

PluginControlPort::PluginControlPort(Plugin* parent, int index, float value)
	: PluginPort(parent, index)
	, m_curve(0)
	, m_plugin(parent)
	, m_value(value)
	, m_automation(false)
{
}

PluginControlPort::PluginControlPort(Plugin* parent, const QDomNode node)
	: PluginPort(parent)
	, m_curve(0)
	, m_plugin(parent)
	, m_automation(false)
{
	set_state(node);
}

QDomNode PluginControlPort::get_state(QDomDocument doc)
{
	QDomElement node = PluginPort::get_state(doc).toElement();
	node.setAttribute("value", m_value);
	node.setAttribute("automation", m_automation);
	if (m_curve) {
		node.appendChild(m_curve->get_state(doc, "PortAutomation"));
	}
	return node;
}

int PluginControlPort::set_state(const QDomNode & node)
{
	QDomElement e = node.toElement();
	m_index = e.attribute("index", "-1").toInt();
	m_value = e.attribute("value", "nan").toFloat();
	m_automation = e.attribute("automation", "0").toInt();
	
	QDomElement curveNode = node.firstChildElement("PortAutomation");
	if (!curveNode.isNull()) {
		m_curve = new Curve(m_plugin, curveNode);
                m_curve->set_sheet(m_plugin->get_session());
	}
		
	return 1;
}

QString PluginControlPort::get_description()
{
	return m_description;
}

QString PluginControlPort::get_symbol()
{
	return "";
}

void PluginControlPort::set_control_value(float value)
{
	m_value = value;
}

void PluginControlPort::set_use_automation(bool automation)
{
	m_automation = automation;
	if (!m_curve) {
		m_curve = new Curve(m_plugin);
		// Add the first default node:
		CurveNode* node = new CurveNode(m_curve, 0.0, 1.0);
		AddRemove* cmd = (AddRemove*)m_curve->add_node(node, false);
		cmd->set_instantanious(true);
		Command::process_command(cmd);
                if (m_plugin->get_session()) {
                        m_curve->set_sheet(m_plugin->get_session());
		}
	}
}

bool PluginControlPort::use_automation()
{
	return m_automation;
}

