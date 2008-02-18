/*
    Copyright (C) 2008 Remon Sijrier 
 
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

#include "PulseAudioDriver.h"

#include <pulse/error.h>

#include "AudioDevice.h"
#include "AudioChannel.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PulseAudioDriver::PulseAudioDriver( AudioDevice * dev , int rate, nframes_t bufferSize)
	: Driver(dev, rate, bufferSize)
{
	read = MakeDelegate(this, &PulseAudioDriver::_read);
	write = MakeDelegate(this, &PulseAudioDriver::_write);
	run_cycle = RunCycleCallback(this, &PulseAudioDriver::_run_cycle);
	
	mainloop = NULL;
	context = NULL;
	stream = NULL;
	mainloop_api = NULL;
	volume = PA_VOLUME_NORM;
	channel_map_set = 0;
}

PulseAudioDriver::~PulseAudioDriver( )
{
	PENTER;
	if (stream)
		pa_stream_unref(stream);

	if (context)
		pa_context_unref(context);

	if (mainloop) {
		pa_signal_done();
		pa_mainloop_free(mainloop);
	}
}

int PulseAudioDriver::_read( nframes_t nframes )
{
	return 1;
}

int PulseAudioDriver::_write( nframes_t nframes )
{
	return 1;
}

int PulseAudioDriver::setup(bool capture, bool playback, const QString& )
{
	PENTER;
	
	sample_spec.rate = frame_rate;
	sample_spec.channels = 2;
	sample_spec.format = PA_SAMPLE_FLOAT32NE;
	
	assert(pa_sample_spec_valid(&sample_spec));
	
	if (channel_map_set && channel_map.channels != sample_spec.channels) {
		fprintf(stderr, "Channel map doesn't match file.\n");
		return -1;
	}
	
	/* Set up a new main loop */
	if (!(mainloop = pa_mainloop_new())) {
		fprintf(stderr, "pa_mainloop_new() failed.\n");
		return -1;
	}

	mainloop_api = pa_mainloop_get_api(mainloop);

	int r = pa_signal_init(mainloop_api);
	assert(r == 0);

	/* Create a new connection context */
	if (!(context = pa_context_new(mainloop_api, "Traverso"))) {
		fprintf(stderr, "pa_context_new() failed.\n");
		return -1;
	}

	pa_context_set_state_callback(context, context_state_callback, this);

	/* Connect the context */
	pa_context_connect(context, "", (pa_context_flags_t)0, NULL);

	int ret;
	/* Run the main loop */
// 	if (pa_mainloop_run(mainloop, &ret) < 0) {
// 		fprintf(stderr, "pa_mainloop_run() failed.\n");
// 		return -1;
// 	}


	AudioChannel* audiochannel;
	int port_flags;
	char buf[32];
	
	// TODO use the found maxchannel count for the playback stream, instead of assuming 2 !!
	for (int chn = 0; chn < 2; chn++) {

		snprintf (buf, sizeof(buf) - 1, "playback_%d", chn+1);

		audiochannel = device->register_playback_channel(buf, "32 bit float audio", port_flags, frames_per_cycle, chn);
		audiochannel->set_latency( frames_per_cycle + capture_frame_latency );
		playbackChannels.append(audiochannel);
	}

	// TODO use the found maxchannel count for the capture stream, instead of assuming 0 !!
	for (int chn = 0; chn < 2; chn++) {

		snprintf (buf, sizeof(buf) - 1, "capture_%d", chn+1);

		audiochannel = device->register_capture_channel(buf, "32 bit float audio", port_flags, frames_per_cycle, chn);
		audiochannel->set_latency( frames_per_cycle + capture_frame_latency );
		captureChannels.append(audiochannel);
	}

	return 1;
}

int PulseAudioDriver::attach( )
{
	PENTER;
	return 1;
}

int PulseAudioDriver::start( )
{
	PENTER;
	return 1;
}

int PulseAudioDriver::stop( )
{
	PENTER;
	return 1;
}

int PulseAudioDriver::process_callback (nframes_t nframes)
{
	device->run_cycle( nframes, 0.0);
	return 0;
}

QString PulseAudioDriver::get_device_name()
{
	return "Pulse";
}

QString PulseAudioDriver::get_device_longname()
{
	return "Pulse";
}

int PulseAudioDriver::_run_cycle()
{
	return device->run_cycle(frames_per_cycle, 0);
}

void PulseAudioDriver::context_state_callback(pa_context * c, void * userdata)
{
}

void PulseAudioDriver::stream_state_callback(pa_stream * s, void * userdata)
{
}

void PulseAudioDriver::stream_write_callback(pa_stream * s, size_t length, void * userdata)
{
}

