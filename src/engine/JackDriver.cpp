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
 
    $Id: JackDriver.cpp,v 1.24 2007/12/07 13:21:49 r_sijrier Exp $
*/

#include "JackDriver.h"

#include <jack/jack.h>

#if defined (ALSA_SUPPORT)
#include "AlsaDriver.h"
#endif

#include "AudioDevice.h"
#include "AudioChannel.h"
#include "Tsar.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

JackDriver::JackDriver( AudioDevice * dev , int rate, nframes_t bufferSize)
                : TAudioDriver(dev, rate, bufferSize)
{
        read = MakeDelegate(this, &JackDriver::_read);
        write = MakeDelegate(this, &JackDriver::_write);
        run_cycle = RunCycleCallback(this, &JackDriver::_run_cycle);
	m_running = false;

        connect(this, SIGNAL(pcpairRemoved(PortChannelPair*)), this, SLOT(cleanup_removed_port_channel_pair(PortChannelPair*)));
}

JackDriver::~JackDriver( )
{
	PENTER;
        Q_ASSERT(!is_running());

        jack_client_close (m_jack_client);
}

int JackDriver::_read( nframes_t nframes )
{
        for (int i=0; i<m_inputs.size(); i++) {
                PortChannelPair* pcpair = m_inputs.at(i);

                if (pcpair->unregister) {
                        m_inputs.removeAll(pcpair);
                        RT_THREAD_EMIT(this, pcpair, pcpairRemoved(PortChannelPair*))
                        continue;
                }

                pcpair->channel->read_from_hardware_port((audio_sample_t*)jack_port_get_buffer (pcpair->jackport, nframes), nframes);
        }
        return 1;
}

int JackDriver::_write( nframes_t nframes )
{
        for (int i=0; i<m_outputs.size(); i++) {
                PortChannelPair* pcpair = m_outputs.at(i);

                if (pcpair->unregister) {
                        m_outputs.removeAll(pcpair);
                        RT_THREAD_EMIT(this, pcpair, pcpairRemoved(PortChannelPair*))
                        continue;
                }

                memcpy (jack_port_get_buffer (pcpair->jackport, nframes), pcpair->channel->get_buffer(nframes), sizeof (jack_default_audio_sample_t) * nframes);
                pcpair->channel->silence_buffer(nframes);
        }
        return 1;
}

int JackDriver::setup(QList<AudioChannel* > channels)
{
	PENTER;
	
        const char *client_name = "Traverso";
        m_jack_client = 0;
        capture_frame_latency = playback_frame_latency =0 ;


        printf("Connecting to the Jack server...\n");

        if ( (m_jack_client = jack_client_open(client_name, JackNoStartServer, NULL)) == NULL) {
                device->driverSetupMessage(tr("Couldn't connect to the jack server, is jack running?"), AudioDevice::DRIVER_SETUP_FAILURE);
                return -1;
        }

        foreach(AudioChannel* channel, channels) {
                add_channel(channel);
        }

	return 1;
}


void JackDriver::add_channel(AudioChannel* channel)
{
        PENTER;
        PortChannelPair* pcpair = new PortChannelPair();

        if (channel->get_type() == ChannelIsInput) {
                pcpair->jackport = jack_port_register (m_jack_client, channel->get_name().toUtf8().data(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        }

        if (channel->get_type() == ChannelIsOutput) {
                pcpair->jackport = jack_port_register (m_jack_client, channel->get_name().toUtf8().data(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        }

        if (pcpair->jackport == 0) {
                printf("JackDriver: cannot register port \"%s\"!\n", channel->get_name().toUtf8().data());
                return;
        }

        pcpair->channel = channel;
        pcpair->channel->set_latency( frames_per_cycle + capture_frame_latency );
        pcpair->name = channel->get_name();


        if (is_running()) {
                THREAD_SAVE_INVOKE(this, pcpair, private_add_port_channel_pair(PortChannelPair*))
        } else {
                private_add_port_channel_pair(pcpair);
        }
}

void JackDriver::private_add_port_channel_pair(PortChannelPair *pair)
{
        if (pair->channel->get_type() == ChannelIsInput) {
                m_inputs.append(pair);
        }
        if (pair->channel->get_type() == ChannelIsOutput) {
                m_outputs.append(pair);
        }
}

void JackDriver::remove_channel(AudioChannel* channel)
{
        PENTER;
        foreach(PortChannelPair* pcpair, m_outputs) {

                if (pcpair->channel == channel) {
                        pcpair->unregister = true;
                        return;
                }
        }

        foreach(PortChannelPair* pcpair, m_inputs) {

                if (pcpair->channel == channel) {
                        pcpair->unregister = true;
                        return;
                }
        }
}


int JackDriver::attach( )
{
	PENTER;

        device->set_buffer_size( jack_get_buffer_size(m_jack_client) );
        device->set_sample_rate (jack_get_sample_rate(m_jack_client));

        jack_set_process_callback (m_jack_client, _process_callback, this);
        jack_set_xrun_callback (m_jack_client, _xrun_callback, this);
        jack_set_buffer_size_callback (m_jack_client, _bufsize_callback, this);
        jack_on_shutdown(m_jack_client, _on_jack_shutdown_callback, this);

        update_config();

        return 1;
}

int JackDriver::start( )
{
	PENTER;
        if (jack_activate (m_jack_client)) {
                //if jack_activate() != 0, something went wrong!
		return -1;
	}
	
        device->driverSetupMessage(tr("Succesfully connected to jack server!"), AudioDevice::DRIVER_SETUP_SUCCESS);

        m_running = 1;
	return 1;
}

int JackDriver::stop( )
{
	PENTER;
        jack_deactivate(m_jack_client);

	m_running = 0;

	return 1;
}

int JackDriver::process_callback (nframes_t nframes)
{
	jack_position_t pos;
        jack_transport_state_t state = jack_transport_query (m_jack_client, &pos);
	
        transport_state_t transportstate;
        transportstate.transport = state;
        transportstate.location = TimeRef(pos.frame, audiodevice().get_sample_rate());
        transportstate.realtime = true;
        transportstate.isSlave = true;

        device->transport_control(transportstate);
	
	device->run_cycle( nframes, 0.0);
        return 0;
}

// NOTE:  note that in jack2 they (process and sync callback) occur asynchronously in 2 different threads
//        How to handle that properly in Travers? The Tsar RT event buffer assumes only one RT thread.
int JackDriver::jack_sync_callback (jack_transport_state_t state, jack_position_t* pos)
{
        transport_state_t transportstate;
        transportstate.transport = state;
        printf("jack state is %d\n", state);
        transportstate.location = TimeRef(pos->frame, audiodevice().get_sample_rate());
        printf("jack transport callback, location is %lld\n", transportstate.location.universal_frame());
        transportstate.isSlave = true;
        transportstate.realtime = true;
	
        return device->transport_control(transportstate);
}


// Is there a way to get the device name from Jack? Can't find it :-(
// Since Jack uses ALSA, we ask it from ALSA directly :-)
QString JackDriver::get_device_name( )
{
#if defined (ALSA_SUPPORT)
        return AlsaDriver::alsa_device_name(false);
#endif
	return "AudioDevice";
}

QString JackDriver::get_device_longname( )
{
#if defined (ALSA_SUPPORT)
        return AlsaDriver::alsa_device_name(true);
#endif
	return "AudioDevice";
}

int JackDriver::_xrun_callback( void * arg )
{
        JackDriver* driver  = static_cast<JackDriver *> (arg);
	if (driver->m_running) {
        	driver->device->xrun();
	}
        return 0;
}

int JackDriver::_process_callback (nframes_t nframes, void *arg)
{
	JackDriver* driver  = static_cast<JackDriver *> (arg);
	if (!driver->m_running) {
		return 0;
	}
	
	return driver->process_callback (nframes);
}

int JackDriver::_bufsize_callback( nframes_t nframes, void * arg )
{
        JackDriver* driver  = static_cast<JackDriver *> (arg);
        driver->device->set_buffer_size( nframes );

        emit driver->device->driverParamsChanged();
        return 0;
}

float JackDriver::get_cpu_load( )
{
        return jack_cpu_load(m_jack_client);
}

void JackDriver::_on_jack_shutdown_callback( void * arg )
{
	JackDriver* driver  = static_cast<JackDriver *> (arg);
	driver->m_running = -1;
}

int JackDriver::_jack_sync_callback (jack_transport_state_t state, jack_position_t* pos, void* arg)
{
	return static_cast<JackDriver*> (arg)->jack_sync_callback (state, pos);
}

void JackDriver::update_config()
{
	m_isSlave = device->get_driver_property("jackslave", false).toBool();
		
	if (m_isSlave) {
                jack_set_sync_callback (m_jack_client, _jack_sync_callback, this);
	} else {
                jack_set_sync_callback(m_jack_client, NULL, this);
	}
}

void JackDriver::cleanup_removed_port_channel_pair(PortChannelPair* pcpair)
{
        PENTER;
        jack_port_unregister(m_jack_client, pcpair->jackport);

        device->delete_channel(pcpair->channel);
        delete pcpair;
}


//eof

