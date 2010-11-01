/*
    Copyright (C) 2005-2007 Remon Sijrier 
 
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

#include "Information.h"
#include "Utils.h"
#include "AudioDevice.h"

#include "Debugger.h"


Information& info()
{
        static Information information;
        return information;
}

void Information::information( const QString & mes )
{
        InfoStruct s;
        s.message = mes;
        s.type = INFO;
	PMESG("Information::information %s", QS_C(mes));
	emit message(s);
}

void Information::warning( const QString & mes )
{
        InfoStruct s;
        s.message = mes;
        s.type = WARNING;
	PWARN("Information::warning %s", QS_C(mes));
        emit message(s);
}


void Information::critical( const QString & mes )
{
        InfoStruct s;
        s.message = mes;
        s.type = CRITICAL;
	PERROR("Information::critical %s", QS_C(mes));
	emit message(s);
}

void Information::audiodevice_message(QString message, int severity)
{
	switch(severity) {
        case AudioDevice::INFO: information(message);
		break;
        case AudioDevice::WARNING: warning(message);
		break;
        case AudioDevice::CRITICAL: critical(message);
		break;
        default: ;// do nothing;
	}
}

Information::Information()
{
	connect(&audiodevice(), SIGNAL(message(QString, int)),
		 this, SLOT(audiodevice_message(QString, int)));
}

