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

$Id: Client.h,v 1.7 2007/11/19 11:18:54 r_sijrier Exp $
*/

#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QObject>
#include "APILinkedList.h"

#include "defines.h"

class AudioDeviceClient : public QObject, public APILinkedListNode
{
	Q_OBJECT

public:
        AudioDeviceClient(const QString& name);
        ~AudioDeviceClient();

	void set_process_callback(ProcessCallback call);
	void set_transport_control_callback(TransportControlCallback call);
	bool is_smaller_then(APILinkedListNode* ) {return false;}

	
	ProcessCallback process;
	TransportControlCallback transport_control;
	
	QString		m_name;

};

#endif

//eof
