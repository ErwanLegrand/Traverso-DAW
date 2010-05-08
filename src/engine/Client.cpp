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

$Id: Client.cpp,v 1.5 2007/06/21 14:31:10 r_sijrier Exp $
*/


#include "Client.h"

#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/**
 * \class Client
 * \brief The Client class is used to include an Object's process callback function 
 *	into the audio processing chain of the AudioDevice's Audio Thread.
 
 * Use the set_process_callback( ProcessCallback call) to set the Objects callback function.
 * It's now save to add the Client to the AudioDevice, which will be done in a Thread save way,
 * without using any mutual exclusion mechanisms.
 * \sa AudioDevice::add_client(Client* client)
 * 
 * @param name The name of the Client, this should be a unique name
 */
AudioDeviceClient::AudioDeviceClient(const QString& name )
{
	PENTERCONS;
	
	m_name = name;
}

AudioDeviceClient::~ AudioDeviceClient( )
{
	PENTERDES;
}

/**
 * Set this Client's process callback delegate to \a call.
 *
 * The ProcessCallback is of type FastDelegate. 
 *
 * Use the convenience function MakeDelegate(this, &MyApp::process); to create a 
 * ProcessCallback delegate. See the AudioDevice for a code example
 *
 * @param call The FastDelegate \a call to use as the callback function 
 */
void AudioDeviceClient::set_process_callback( ProcessCallback call)
{
	process = call;
}

void AudioDeviceClient::set_transport_control_callback(TransportControlCallback callback)
{
	transport_control = callback;
}

//eof

