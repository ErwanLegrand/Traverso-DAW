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

#ifndef PULSE_AUDIO_DRIVER_H
#define PULSE_AUDIO_DRIVER_H

#include "Driver.h"
#include "defines.h"
#include <pulse/pulseaudio.h>

class PulseAudioDriver : public Driver
{
public:
	PulseAudioDriver(AudioDevice* dev, int rate, nframes_t bufferSize);
	~PulseAudioDriver();

	int  process_callback (nframes_t nframes);
	int _read(nframes_t nframes);
	int _write(nframes_t nframes);
	int _run_cycle();
	int setup(bool capture=true, bool playback=true, const QString& cardDevice="hw:0");
	int attach();
	int start();
	int stop();

	QString get_device_name();
	QString get_device_longname();

	float get_cpu_load();

	void update_config();

private:
	/** PulseAudio playback stream object */
	pa_mainloop* mainloop;
	pa_context *context;
	pa_stream *stream;
	pa_mainloop_api *mainloop_api;
	pa_volume_t volume;
	pa_sample_spec sample_spec;
	pa_channel_map channel_map;	
	int channel_map_set;
		
	static void context_state_callback(pa_context *c, void *userdata);
	static void stream_state_callback(pa_stream *s, void *userdata);
	static void stream_write_callback(pa_stream *s, size_t length, void *userdata);
};


#endif

//eof

 
