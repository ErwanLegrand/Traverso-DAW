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

$Id: AudioOutputPort.cpp,v 1.3 2006/09/14 10:45:44 r_sijrier Exp $
*/


#include "AudioOutputPort.h" 

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
 
//eof

