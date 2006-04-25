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

$Id: Driver.cpp,v 1.2 2006/04/25 17:23:06 r_sijrier Exp $
*/

#include "Driver.h"
#include <QThread>
#include "AudioDevice.h"
#include "AudioChannel.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Driver::Driver( AudioDevice* dev , int rate, nframes_t bufferSize)
{
	device = dev;
	frame_rate = rate;
	frames_per_cycle = bufferSize;

	read = MakeDelegate(this, &Driver::_read);
	write = MakeDelegate(this, &Driver::_write);
	run_cycle = RunCycleCallback(this, &Driver::_run_cycle);
}

Driver::~ Driver( )
{
	PENTERDES;
}

int Driver::_run_cycle( )
{
	// * 1000, we want it in millisecond
	// / 2, 2 bytes (16 bit)
	device->transport_cycle_end (get_microseconds());

	usleep(23000);

	device->transport_cycle_start (get_microseconds());

	return device->run_cycle( frames_per_cycle, 0);
}

int Driver::_read( nframes_t  )
{
	return 1;
}

int Driver::_write( nframes_t nframes )
{
	foreach(AudioChannel* chan, playbackChannels) {
		chan->get_data();
		chan->silence_buffer(nframes);
	}

	return 1;
}

int Driver::_null_cycle( nframes_t  )
{
	return 1;
}

int Driver::setup( )
{
	return 1;
}

int Driver::attach( )
{
	int port_flags;
	char buf[32];
	AudioChannel* chan;
	
	device->set_buffer_size (frames_per_cycle);
	device->set_sample_rate (frame_rate);
	
	port_flags = PortIsOutput|PortIsPhysical|PortIsTerminal;
	
	// Create 2 fake capture channels
	for (uint chn=0; chn<2; chn++) {
		snprintf (buf, sizeof(buf) - 1, "capture_%d", chn+1);
		
		chan = device->register_capture_channel(buf, JACK_DEFAULT_AUDIO_TYPE, port_flags, frames_per_cycle, chn);
		chan->set_latency( frames_per_cycle + capture_frame_latency );
		captureChannels.append(chan);
	}

	// Create 2 fake playback channels
	for (uint chn=0; chn<2; chn++) {
		snprintf (buf, sizeof(buf) - 1, "playback_%d", chn+1);
		
		chan = device->register_playback_channel(buf, JACK_DEFAULT_AUDIO_TYPE, port_flags, frames_per_cycle, chn);
		chan->set_latency( frames_per_cycle + capture_frame_latency );
		playbackChannels.append(chan);
	}

	return 1;
}

int Driver::detach( )
{
	return 0;
}

int Driver::start( )
{
	return 1;
}

int Driver::stop( )
{
	return 1;
}

QString Driver::get_device_name( )
{
	return "Null Audio Device";
}

QString Driver::get_device_longname( )
{
	return "Null Audio Device";
}


//eof

