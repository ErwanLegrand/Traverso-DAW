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
 
    $Id: JackDriver.cpp,v 1.15 2007/06/14 11:51:31 r_sijrier Exp $
*/

#include "JackDriver.h"

#include "AudioDevice.h"
#include "AudioChannel.h"

#include <Information.h>

#include <jack/jack.h>

#if defined (ALSA_SUPPORT)
#include "AlsaDriver.h"
#endif

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
                jack_client_close (client);
	}
}

int JackDriver::_read( nframes_t nframes )
{
        int portNumber = 0;
	for (int i=0; i<captureChannels.size(); ++i) {
		AudioChannel* chan = captureChannels.at(i);
                if (!chan->has_data()) {
                        portNumber++;
                        continue;
                }
                memcpy (chan->get_data(), jack_port_get_buffer (inputPorts[portNumber], nframes), sizeof (jack_default_audio_sample_t) * nframes);
                portNumber++;
        }
        return 1;
}

int JackDriver::_write( nframes_t nframes )
{
        int portNumber = 0;
	for (int i=0; i<playbackChannels.size(); ++i) {
		AudioChannel* chan = playbackChannels.at(i);
		
/*		if (!chan->has_data()) {
			portNumber++;
			continue;
		}*/
                memcpy ( jack_port_get_buffer (outputPorts[portNumber], nframes), chan->get_data(), sizeof (jack_default_audio_sample_t) * nframes);
                chan->silence_buffer(nframes);
                portNumber++;
        }
        return 1;
}

int JackDriver::setup(bool capture, bool playback, const QString& )
{
	PENTER;
        const char **inputports;
        const char **outputports;
        const char *client_name = "Traverso";
        // 	const char *server_name = NULL;
        int inputPortCount = 0;
        int outputPortCount = 0;
        client = 0;
        AudioChannel* channel;
        char buf[32];
        int port_flags;
        capture_frame_latency = playback_frame_latency =0;


        printf("Connecting to the Jack server...\n");

        if ( (client = jack_client_new (client_name)) == NULL) {
		info().warning(tr("Jack Driver: Couldn't connect to the jack server, is jack running?"));
                return -1;
        }


        /*********** INPUT PORTS STUFF *************/
        /******************************************/

        //Get all the input ports of Jack
        if ((inputports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == 0) {
                inputPorts =  0;
        }

        if (inputports) {
                for (inputPortCount = 0; inputports[inputPortCount]; ++inputPortCount)
                        ;
                free (inputports);
        }


        inputPorts = (jack_port_t **) malloc (sizeof (jack_port_t *) * inputPortCount);

        for (int i = 0; i < inputPortCount; i++) {
                char name[64];

                sprintf (name, "input_%d", i+1);

                if ((inputPorts[i] = jack_port_register (client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0)) == 0) {
                        fprintf (stderr, "cannot register input port \"%s\"!\n", name);
                        jack_client_close (client);
                        return -1;
                }
        }


        /*********** OUTPUT PORTS STUFF *************/
        /******************************************/

        //Get all the input ports of Jack
        if ((outputports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == 0) {
                outputPorts =  0;
        }

        if (outputports) {
                for (outputPortCount = 0; outputports[outputPortCount]; ++outputPortCount)
                        ;
                free (outputports);
        }


        outputPorts = (jack_port_t **) malloc (sizeof (jack_port_t *) * outputPortCount);

        for (int i = 0; i < outputPortCount; i++) {
                char name[64];

                sprintf (name, "output_%d", i+1);

                if ((outputPorts[i] = jack_port_register (client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0)) == 0) {
                        fprintf (stderr, "cannot register output port \"%s\"!\n", name);
                        jack_client_close (client);
                        return -1;
                }
        }


        port_flags = PortIsInput|PortIsPhysical|PortIsTerminal;
        for (int chn = 0; chn < inputPortCount; chn++) {

                snprintf (buf, sizeof(buf) - 1, "capture_%d", chn+1);

		channel = device->register_capture_channel(buf, JACK_DEFAULT_AUDIO_TYPE, port_flags, frames_per_cycle, chn);
		channel->set_latency( frames_per_cycle + capture_frame_latency );
		captureChannels.append(channel);
        }


        port_flags = PortIsOutput|PortIsPhysical|PortIsTerminal;
        for (int chn = 0; chn < outputPortCount; chn++) {

                snprintf (buf, sizeof(buf) - 1, "playback_%d", chn+1);

		channel = device->register_playback_channel(buf, JACK_DEFAULT_AUDIO_TYPE, port_flags, frames_per_cycle, chn);
		channel->set_latency( frames_per_cycle + capture_frame_latency );
		playbackChannels.append(channel);
        }


        device->set_buffer_size( jack_get_buffer_size(client) );
        device->set_sample_rate (jack_get_sample_rate(client));

        jack_set_process_callback (client, _process_callback, this);
        jack_set_xrun_callback (client, _xrun_callback, this);
        jack_set_buffer_size_callback (client, _bufsize_callback, this);
	jack_on_shutdown(client, _on_jack_shutdown_callback, this);


	info().information(tr("Jack Driver: Connected successfully to the jack server!"));
        
	return 1;
}

int JackDriver::attach( )
{
	PENTER;
        return 1;
}

int JackDriver::start( )
{
	PENTER;
        if (jack_activate (client)) {
		//if jack_active() != 0, something went wrong!
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
        device->run_cycle( nframes, 0.0);
        return 0;
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
        return jack_cpu_load(client);
}

void JackDriver::_on_jack_shutdown_callback( void * arg )
{
	JackDriver* driver  = static_cast<JackDriver *> (arg);
	driver->m_running = -1;
}


//eof
