/*
Copyright (C) 2005-2006 Remon Sijrier 

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

$Id: Client.cpp,v 1.1 2006/04/20 14:50:44 r_sijrier Exp $
*/


#include "Client.h"

#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Client::Client( QString name )
{
	m_name = name;
	scheduleForDeletion = false;
}

Client::~ Client( )
{
	PENTERDES;
}

void Client::set_process_callback( ProcessCallback call)
{
	process = call;
}

void Client::delete_client( )
{
	scheduleForDeletion = true;
}

bool Client::scheduled_for_deletion( )
{
	return scheduleForDeletion;
}

//eof
