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

$Id: LV2ControlPort.h,v 1.1 2006/07/31 13:24:46 r_sijrier Exp $
*/


#ifndef LV2_CONTROL_PORT_H
#define LV2_CONTROL_PORT_H

#include <PluginPort.h>

class LV2Plugin;


class LV2ControlPort : public PluginPort
{
	Q_OBJECT

public:
	LV2ControlPort(LV2Plugin* plugin, int index);
	LV2ControlPort(LV2Plugin* plugin, const QDomNode node);
	~LV2ControlPort(){};

	float get_control_value() {return m_controlValue; }
	float get_min_control_value();
	float get_max_control_value();
	QDomNode get_state(QDomDocument doc);

	QString get_description();

private:
	LV2Plugin*		m_plugin;
	float			m_controlValue;

	int set_state( const QDomNode & node );

public slots:
	void set_control_value(double value);

}; 

#endif

//eof

