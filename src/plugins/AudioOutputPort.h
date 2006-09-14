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

$Id: AudioOutputPort.h,v 1.2 2006/09/14 10:45:44 r_sijrier Exp $
*/


#ifndef AUDIO_OUTPUT_PORT_H
#define AUDIO_OUTPUT_PORT_H

#include "PluginPort.h"

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

 
 
 
