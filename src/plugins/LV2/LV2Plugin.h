/*
Copyright (C) 2006 Remon Sijrier

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

$Id: LV2Plugin.h,v 1.2 2006/07/31 13:59:20 r_sijrier Exp $
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

public:
	LV2Plugin();
	LV2Plugin(char* pluginUri);
	~LV2Plugin();

	void process(AudioBus* bus, unsigned long nframes);

	SLV2Instance*  get_instance() const {return m_instance; }
	SLV2Plugin* get_slv2_plugin() const {return m_plugin; }

	QList<LV2ControlPort* > get_control_ports() const { return m_controlPorts; }

	QDomNode get_state(QDomDocument doc);
	QString get_name();
	
	int init();
	int set_state(const QDomNode & node );

private:
	QString		m_pluginUri;
	SLV2Instance*  	m_instance;    /**< Plugin "instance" (loaded shared lib) */
	SLV2Plugin*    	m_plugin;      /**< Plugin "class" (actually just a few strings) */
	
	QList<LV2ControlPort* > 	m_controlPorts;
	QList<AudioInputPort* >		m_audioInputPorts;
	QList<AudioOutputPort* >	m_audioOutputPorts;

	LV2ControlPort* create_port(int portIndex);

	int create_instance();
};


#endif
