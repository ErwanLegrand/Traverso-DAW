/*
Copyright (C) 2006-2009 Remon Sijrier

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

#include <cstdlib>
#include <slv2/slv2.h>
#include <redland.h>
#include <QList>
#include <QString>
#include <QObject>

#include "Plugin.h"

class AudioBus;
class LV2ControlPort;
class AudioInputPort;
class AudioOutputPort;
class TSession;

class LV2Plugin : public Plugin
{
	Q_OBJECT

public:
        LV2Plugin(TSession* session, bool slave=false);
        LV2Plugin(TSession* session, char* pluginUri);
	~LV2Plugin();

	void process(AudioBus* bus, unsigned long nframes);

	SLV2Instance  get_instance() const {return m_instance; }
	SLV2Plugin get_slv2_plugin() const {return m_plugin; }
	LV2Plugin* create_copy();

	QDomNode get_state(QDomDocument doc);
	QString get_name();
	
	int init();
	int set_state(const QDomNode & node );
	
	static PluginInfo get_plugin_info(SLV2Plugin plugin);

private:
	QString		m_pluginUri;
	SLV2Plugin     m_plugin;   /**< Plugin "class" (actually just a few strings) */
	SLV2Instance   m_instance;      /**< Plugin "instance" (loaded shared lib) */
	uint32_t       m_num_ports;     /**< Size of the two following arrays: */
	struct Port*   m_ports;         /**< Port array of size num_ports */
	SLV2Value      m_input_class;   /**< Input port class (URI) */
	SLV2Value      m_output_class;  /**< Output port class (URI) */
	SLV2Value      m_control_class; /**< Control port class (URI) */
	SLV2Value      m_audio_class;   /**< Audio port class (URI) */
	SLV2Value      m_event_class;   /**< Event port class (URI) */
	SLV2Value      optional;        /**< lv2:connectionOptional port property */
	bool 		m_isSlave;
	
	LV2ControlPort* create_port(int portIndex, float defaultValue);

	int create_instance();

public slots:
	TCommand* toggle_bypass();
};


class LV2ControlPort : public PluginControlPort
{

public:
	LV2ControlPort(LV2Plugin* plugin, int index, float value);
	LV2ControlPort(LV2Plugin* plugin, const QDomNode node);
	~LV2ControlPort(){};

	float get_min_control_value();
	float get_max_control_value();
	float get_default_value();
	
	QDomNode get_state(QDomDocument doc);

	QString get_description();
	QString get_symbol();

private:
	LV2Plugin*	m_lv2plugin;
	
	void init();
	QStringList get_hints();
};


#endif
