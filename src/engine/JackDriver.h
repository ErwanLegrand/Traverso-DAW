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
 
    $Id: JackDriver.h,v 1.7 2007/06/21 14:31:11 r_sijrier Exp $
*/

#ifndef JACKDRIVER_H
#define JACKDRIVER_H

#include "Driver.h"
#include "defines.h"
#include <jack/jack.h>
#include <QObject>

class JackDriver : public Driver
{
	Q_OBJECT
public:
        JackDriver(AudioDevice* dev, int rate, nframes_t bufferSize);
        ~JackDriver();

        int  process_callback (nframes_t nframes);
        int _read(nframes_t nframes);
        int _write(nframes_t nframes);
        int _run_cycle() {return 1;}
        int setup(bool capture=true, bool playback=true, const QString& cardDevice="hw:0");
        int attach();
        int start();
        int stop();

        QString get_device_name();
        QString get_device_longname();

        float get_cpu_load();
	
	size_t is_jack_running() const {return m_running == 1;}
	jack_client_t* get_client() const {return client;}
	void set_jack_slave(bool slave);
	bool is_slave() const {return m_isSlave;}

private:
	volatile size_t	m_running;
        jack_client_t*	client;
        jack_port_t**	inputPorts;
        jack_port_t**	outputPorts;
	bool		m_isSlave;
	
	int  jack_sync_callback (jack_transport_state_t, jack_position_t*);

        static int _xrun_callback(void *arg);
        static int  _process_callback (nframes_t nframes, void *arg);
        static int _bufsize_callback(jack_nframes_t nframes, void *arg);
	static void _on_jack_shutdown_callback(void* arg);
	static int  _jack_sync_callback (jack_transport_state_t, jack_position_t*, void *arg);	
signals:
	void jackShutDown();
	
private slots:
	void update_config();

};


#endif

//eof

