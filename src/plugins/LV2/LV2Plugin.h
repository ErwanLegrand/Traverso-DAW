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
class Song;

class LV2Plugin : public Plugin
{
	Q_OBJECT
	Q_CLASSINFO("toggle_bypass", tr("Bypass: On/Off"))

public:
	LV2Plugin(Song* song, bool slave=false);
	LV2Plugin(Song* song, char* pluginUri);
	~LV2Plugin();

	void process(AudioBus* bus, unsigned long nframes);

	SLV2Instance  get_instance() const {return m_instance; }
	SLV2Plugin get_slv2_plugin() const {return m_slv2plugin; }
	LV2Plugin* create_copy();

	QDomNode get_state(QDomDocument doc);
	QString get_name();
	
	int init();
	int set_state(const QDomNode & node );
	
	static PluginInfo get_plugin_info(const QString& uri);
	static QString plugin_type(const QString& uri);

private:
	QString		m_pluginUri;
	SLV2Instance  	m_instance;
	SLV2Plugin    	m_slv2plugin;
	bool 		m_isSlave;
	
	LV2ControlPort* create_port(int portIndex);

	int create_instance();

public slots:
	Command* toggle_bypass();
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
