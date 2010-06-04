/*
    Copyright (C) 2005-2010 Remon Sijrier
 
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
 
    $Id: JackDriver.h,v 1.9 2007/10/20 17:38:19 r_sijrier Exp $
*/

#ifndef JACKDRIVER_H
#define JACKDRIVER_H

#include "Driver.h"
#include "defines.h"
#include <jack/jack.h>
#include <QObject>
#include <QVector>

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
        int setup(QList<AudioChannel* > channels);
        int attach();
        int start();
        int stop();

        QString get_device_name();
        QString get_device_longname();

        void add_channel(AudioChannel* channel);
        void remove_channel(AudioChannel* channel);

        float get_cpu_load();
	
        size_t is_running() const {return m_running == 1;}
        jack_client_t* get_client() const {return m_jack_client;}
	bool is_slave() const {return m_isSlave;}
	void update_config();

private:
        struct PortChannelPair {
                PortChannelPair() {
                        jackport = 0;
                        channel = 0;
                        unregister = false;
                }

                jack_port_t*    jackport;
                AudioChannel*   channel;
                QString         name;
                bool            unregister;
        };

        volatile size_t         m_running;
        jack_client_t*          m_jack_client;
        QList<PortChannelPair*> m_inputs;
        QList<PortChannelPair*> m_outputs;

        bool                    m_isSlave;

	int  jack_sync_callback (jack_transport_state_t, jack_position_t*);

        static int _xrun_callback(void *arg);
        static int  _process_callback (nframes_t nframes, void *arg);
        static int _bufsize_callback(jack_nframes_t nframes, void *arg);
	static void _on_jack_shutdown_callback(void* arg);
	static int  _jack_sync_callback (jack_transport_state_t, jack_position_t*, void *arg);	

private slots:
        void private_add_port_channel_pair(PortChannelPair* pair);
        void cleanup_removed_port_channel_pair(PortChannelPair* pair);

signals:
	void jackShutDown();
        void pcpairRemoved(PortChannelPair*);
	
};


#endif

//eof

