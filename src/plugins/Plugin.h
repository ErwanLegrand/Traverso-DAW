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


#ifndef PLUGIN_H
#define PLUGIN_H

#include "ContextItem.h"
#include <QString>
#include <QDomNode>

#include "defines.h"

class AudioBus;
class PluginChain;
class PluginControlPort;
class AudioInputPort;
class AudioOutputPort;
class Curve;
class Song;

class Plugin : public ContextItem
{
	Q_OBJECT
	Q_CLASSINFO("toggle_bypass", tr("Bypass: On/Off"))
	
public:
	Plugin(Song* song = 0);
	virtual ~Plugin(){};

	virtual int init() {return 1;}
	virtual	QDomNode get_state(QDomDocument doc);
	virtual int set_state(const QDomNode & node );
	virtual void process(AudioBus* bus, unsigned long nframes) = 0;
	virtual QString get_name() = 0;
	
	PluginControlPort* get_control_port_by_index(int index) const;
	QList<PluginControlPort* > get_control_ports() const { return m_controlPorts; }
	
	Plugin* get_slave() const {return m_slave;}
	Song* get_song() const {return m_song;}
	bool is_bypassed() const {return m_bypass;}
	
	void automate_port(int index, bool automate);
	

protected:
	Plugin* m_slave;
	Song* m_song;
	QList<PluginControlPort* > 	m_controlPorts;
	QList<AudioInputPort* >		m_audioInputPorts;
	QList<AudioOutputPort* >	m_audioOutputPorts;
	
	bool	m_bypass;
	
	
signals:
	void bypassChanged();
	
public slots:
	Command* toggle_bypass();
};


class PluginPort : public QObject
{

public:
	PluginPort(QObject* parent, int index) : QObject(parent), m_index(index), m_hint(FLOAT_CONTROL) {};
	PluginPort(QObject* parent) : QObject(parent), m_hint(FLOAT_CONTROL) {};
	virtual ~PluginPort(){};

	virtual QDomNode get_state(QDomDocument doc);
	virtual int set_state( const QDomNode & node ) = 0;
	
	enum PortHint {
		FLOAT_CONTROL,
  		INT_CONTROL,
    		LOG_CONTROL
	};
	
	int get_index() const {return m_index;}
	int get_hint() const {return m_hint;}
	
	void set_index(int index) {m_index = index;}

protected:
	int	m_index;
	int	m_hint;
}; 


class PluginControlPort : public PluginPort
{
	Q_OBJECT

public:
	PluginControlPort(Plugin* parent, int index, float value);
	PluginControlPort(Plugin* parent, const QDomNode node);
	virtual ~PluginControlPort(){}

	virtual float get_control_value() {return m_value; }
	virtual float get_min_control_value() {return m_min;}
	virtual float get_max_control_value() {return m_max;}
	virtual float get_default_value() {return m_default;}
	
	void set_min(float min) {m_min = min;}
	void set_max(float max) {m_max = max;}
	void set_default(float def) {m_default = def;}
	void set_use_automation(bool automation);
	
	bool use_automation();
	Curve* get_curve() const {return m_curve;}

	virtual QDomNode get_state(QDomDocument doc);

	virtual QString get_description();
	virtual QString get_symbol();

protected:
	Curve*	m_curve;
	Plugin*	m_plugin;
	float	m_value;
	float 	m_default;
	float	m_min;
	float	m_max;
	bool	m_automation;
	QString m_description;
	
	virtual int set_state( const QDomNode & node );
	
public slots:
	void set_control_value(float value);
};


class AudioInputPort : public PluginPort
{

public:
	AudioInputPort(QObject* parent, int index);
	AudioInputPort(QObject* parent) : PluginPort(parent) {};
	virtual ~AudioInputPort(){};

	QDomNode get_state(QDomDocument doc);
	int set_state( const QDomNode & node );

};


class AudioOutputPort : public PluginPort
{

public:
	AudioOutputPort(QObject* parent, int index);
	AudioOutputPort(QObject* parent) : PluginPort(parent) {};
	virtual ~AudioOutputPort(){};

	QDomNode get_state(QDomDocument doc);
	int set_state( const QDomNode & node );

}; 


#endif

//eof
