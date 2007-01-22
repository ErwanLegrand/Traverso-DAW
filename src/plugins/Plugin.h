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

$Id: Plugin.h,v 1.5 2007/01/22 20:12:58 r_sijrier Exp $
*/


#ifndef PLUGIN_H
#define PLUGIN_H

#include "ContextItem.h"
#include <QString>
#include <QDomNode>

#include "defines.h"

class AudioBus;
class PluginChain;

class Plugin : public ContextItem
{
	Q_OBJECT
	Q_CLASSINFO("toggle_bypass", tr("Bypass: On/Off"))
	
public:
	Plugin();
	~Plugin(){};

	virtual int init() = 0;
	virtual	QDomNode get_state(QDomDocument doc) = 0;
	virtual int set_state(const QDomNode & node ) = 0;
	virtual void process(AudioBus* bus, unsigned long nframes) = 0;
	virtual QString get_name() = 0;
	
	bool is_bypassed() const {return m_bypass;}
	

protected:
	bool	m_bypass;
	
signals:
	void bypassChanged();
	
public slots:
	Command* toggle_bypass();
};


class PluginPort : public QObject
{

public:
	PluginPort(QObject* parent, int index) : QObject(parent), m_index(index) {};
	PluginPort(QObject* parent) : QObject(parent) {};
	~PluginPort(){};

	virtual QDomNode get_state(QDomDocument doc);
	virtual int set_state( const QDomNode & node ) = 0;
	
	int get_index() const {return m_index;}

protected:
	int	m_index;
}; 


class AudioInputPort : public PluginPort
{

public:
	AudioInputPort(QObject* parent, int index);
	AudioInputPort(QObject* parent) : PluginPort(parent) {};
	~AudioInputPort(){};

	QDomNode get_state(QDomDocument doc);
	int set_state( const QDomNode & node );

}; 


class AudioOutputPort : public PluginPort
{

public:
	AudioOutputPort(QObject* parent, int index);
	AudioOutputPort(QObject* parent) : PluginPort(parent) {};
	~AudioOutputPort(){};

	QDomNode get_state(QDomDocument doc);
	int set_state( const QDomNode & node );

}; 


#endif

//eof
