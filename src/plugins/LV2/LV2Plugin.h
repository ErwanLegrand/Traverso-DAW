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


#ifndef LV2_PLUGIN_H
#define LV2_PLUGIN_H

#include <slv2/slv2.h>
#include <QList>
#include <QString>
#include <QObject>

#include "Plugin.h"

class AudioBus;
class LV2ControlPort;
class AudioInputPort;
class AudioOutputPort;

class LV2Plugin : public Plugin
{
	Q_OBJECT
	Q_CLASSINFO("toggle_bypass", tr("Bypass: On/Off"))

public:
	LV2Plugin(bool slave=false);
	LV2Plugin(char* pluginUri);
	~LV2Plugin();

	void process(AudioBus* bus, unsigned long nframes);

	SLV2Instance  get_instance() const {return m_instance; }
	SLV2Plugin get_slv2_plugin() const {return m_slv2plugin; }
	LV2ControlPort* get_control_port_by_index(int index) const;
	LV2Plugin* create_copy();
	LV2Plugin* get_slave() const {return m_slave;}

	QList<LV2ControlPort* > get_control_ports() const { return m_controlPorts; }

	QDomNode get_state(QDomDocument doc);
	QString get_name();
	
	int init();
	int set_state(const QDomNode & node );

private:
	QString		m_pluginUri;
	SLV2Instance  	m_instance;
	SLV2Plugin    	m_slv2plugin;
	int		m_portcount;
	LV2Plugin* 	m_slave;
	bool 		m_isSlave;
	
	QList<LV2ControlPort* > 	m_controlPorts;
	QList<AudioInputPort* >		m_audioInputPorts;
	QList<AudioOutputPort* >	m_audioOutputPorts;

	LV2ControlPort* create_port(int portIndex);

	int create_instance();

public slots:
	Command* toggle_bypass();
};


class LV2ControlPort : public PluginPort
{
	Q_OBJECT

public:
	LV2ControlPort(LV2Plugin* plugin, int index, float value);
	LV2ControlPort(LV2Plugin* plugin, const QDomNode node);
	~LV2ControlPort(){};

	float get_control_value() {return m_controlValue; }
	float get_min_control_value();
	float get_max_control_value();
	QDomNode get_state(QDomDocument doc);

	QString get_description();

private:
	LV2Plugin*	m_lv2plugin;
	float		m_controlValue;

	int set_state( const QDomNode & node );

public slots:
	void set_control_value(float value);

}; 


#endif
