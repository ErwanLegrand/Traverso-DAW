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

$Id: AudioInputPort.cpp,v 1.1 2006/07/31 13:24:46 r_sijrier Exp $

slv2 url: http://codeson.net/svn/libslv2/
*/


#include "AudioInputPort.h" 


AudioInputPort::AudioInputPort(int index)
	: PluginPort(index)
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
