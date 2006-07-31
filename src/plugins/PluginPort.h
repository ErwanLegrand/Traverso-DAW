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

$Id: PluginPort.h,v 1.1 2006/07/31 13:24:46 r_sijrier Exp $
*/


#ifndef PLUGIN_PORT_H
#define PLUGIN_PORT_H

#include <QString>
#include <QDomNode>
#include <QObject>


class PluginPort : public QObject
{

public:
	PluginPort(int index) : m_index(index) {};
	PluginPort(){};
	~PluginPort(){};

	virtual QDomNode get_state(QDomDocument doc) = 0;
	virtual int set_state( const QDomNode & node ) = 0;
	
	int get_index() const {return m_index;}

protected:
	int	m_index;


}; 

#endif

//eof

 
