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

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

JackDriver::JackDriver( AudioDevice * dev , int rate, nframes_t bufferSize)
                : Driver(dev, rate, bufferSize)
{
        read = MakeDelegate(this, &JackDriver::_read);
        write = MakeDelegate(this, &JackDriver::_write);
        run_cycle = RunCycleCallback(this, &JackDriver::_run_cycle);
	m_running = false;
}

JackDriver::~JackDriver( )
{
	PENTER;
	if (m_running == 0) {
                jack_client_close (m_jack_client);
	}
}

int JackDriver::_read( nframes_t nframes )
{
        for (int i=0; i<m_captureChannels.size(); ++i) {
                AudioChannel* chan = m_captureChannels.at(i);
                if (!chan->has_data()) {
                        continue;
                }
                memcpy (chan->get_data(), jack_port_get_buffer (m_inputPorts[i], nframes), sizeof (jack_default_audio_sample_t) * nframes);
        }
        return 1;
}

int JackDriver::_write( nframes_t nframes )
{
        for (int i=0; i<m_playbackChannels.size(); ++i) {
                AudioChannel* chan = m_playbackChannels.at(i);
		
/*		if (!chan->has_data()) {
			continue;
		}*/
                memcpy ( jack_port_get_buffer (m_outputPorts[i], nframes), chan->get_data(), sizeof (jack_default_audio_sample_t) * nframes);
                chan->silence_buffer(nframes);
        }
        return 1;
}

int JackDriver::setup(bool capture, bool playback, const QString& )
{
	PENTER;
	
	Q_UNUSED(capture);
	Q_UNUSED(playback);

        const char **inputports;
        const char **outputports;
        const char *client_name = "Traverso";
        m_jack_client = 0;
        capture_frame_latency = playback_frame_latency =0;


        printf("Connecting to the Jack server...\n");

        if ( (m_jack_client = jack_client_new (client_name)) == NULL) {
		device->message(tr("Jack Driver: Couldn't connect to the jack server, is jack running?"), AudioDevice::WARNING);
                return -1;
        }


        //Get all the input ports of Jack
        inputports = jack_get_ports (m_jack_client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);

        if (inputports) {
                for (int i = 0; inputports[i]; i++) {
                        char name[64];
                        sprintf (name, "input_%d", i+1);
                        add_capture_channel(name);
                }

                free (inputports);
        }


        //Get all the output ports of Jack
        outputports = jack_get_ports (m_jack_client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);

        if (outputports) {
                for (int i = 0; outputports[i]; i++) {
                        char name[64];
                        sprintf (name, "output_%d", i+1);
                        add_playback_channel(name);
                }

                free (outputports);
        }


	device->message(tr("Jack Driver: Connected successfully to the jack server!"), AudioDevice::INFO);
        
	return 1;
}


AudioChannel* JackDriver::add_capture_channel(const QByteArray& chanName)
{
        char buf[32];

        int channelNumber = m_captureChannels.size();
        snprintf (buf, sizeof(buf) - 1, "capture_%d", channelNumber+1);

        AudioChannel* chan = new AudioChannel(buf, channelNumber);
        chan->set_latency( frames_per_cycle + capture_frame_latency );


        jack_port_t* port = jack_port_register (m_jack_client, chanName, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

        if (port == 0) {
                fprintf (stderr, "cannot register input port \"%s\"!\n", chanName.data());
                return 0;
        }


        m_captureChannels.append(chan);
        m_inputPorts.append(port);

        return chan;
}

AudioChannel* JackDriver::add_playback_channel(const QByteArray& chanName)
{
        char buf[32];

        int channelNumber = m_playbackChannels.size();
        snprintf (buf, sizeof(buf) - 1, "playback_%d", channelNumber+1);
        AudioChannel* chan = new AudioChannel(buf, channelNumber);
        chan->set_latency( frames_per_cycle + capture_frame_latency );

        jack_port_t* port = jack_port_register (m_jack_client, chanName, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        if (port == 0) {
                fprintf (stderr, "cannot register output port \"%s\"!\n", chanName.data());
                delete chan;
                return 0;
        }

        m_playbackChannels.append(chan);
        m_outputPorts.append(port);

        return chan;
}


int JackDriver::delete_capture_channel(QString name)
{
        AudioChannel* chan = get_capture_channel_by_name(name);

        if (!chan) {
                // No capture channel by this name is known, do nothing
                return 0;
        }

        int index = m_captureChannels.indexOf(chan);
        m_captureChannels.removeAt(index);

        jack_port_t* port = m_outputPorts.at(index);
        jack_port_disconnect(m_jack_client, port);

        m_outputPorts.remove(index);

        return 1;
}


int JackDriver::delete_playback_channel(QString name)
{
        AudioChannel* chan = get_playback_channel_by_name(name);

        if (!chan) {
                // No playback channel by this name is known, do nothing
                return 0;
        }

        int index = m_playbackChannels.indexOf(chan);
        m_playbackChannels.removeAt(index);

        jack_port_t* port = m_inputPorts.at(index);
        jack_port_disconnect(m_jack_client, port);

        m_inputPorts.remove(index);

        return 1;
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
	
	m_running = 1;
	return 1;
}

int JackDriver::stop( )
{
	PENTER;
	m_running = 0;
	return 1;
}

int JackDriver::process_callback (nframes_t nframes)
{
	jack_position_t pos;
        jack_transport_state_t state = jack_transport_query (m_jack_client, &pos);
	
	transport_state_t tranportstate;
        tranportstate.transport = state;
	tranportstate.location = TimeRef(pos.frame, audiodevice().get_sample_rate());
	tranportstate.realtime = true;
	
	device->transport_control(tranportstate);
	
	device->run_cycle( nframes, 0.0);
        return 0;
}

int JackDriver::jack_sync_callback (jack_transport_state_t state, jack_position_t* pos)
{
        transport_state_t transportstate;
        transportstate.transport = state;
        transportstate.location = TimeRef(pos->frame, audiodevice().get_sample_rate());
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

//eof

