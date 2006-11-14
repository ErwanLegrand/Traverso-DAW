/*
Copyright (C) 2005-2006 Remon Sijrier 

(December 2005) Ported to C++ for Traverso by Remon Sijrier
Copyright (C) 2001 Paul Davis 

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

$Id: AlsaDriver.cpp,v 1.8 2006/11/14 14:32:12 r_sijrier Exp $
*/


#include "AlsaDriver.h"
#include "AudioDevice.h"
#include "AudioChannel.h"
#include <Utils.h>

#include <pthread.h>

#include <math.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/types.h>
#include <regex.h>

#include <string.h>
#include <sys/time.h>
#include <time.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

#undef DEBUG_WAKEUP

/* Delay (in process calls) before Traverso will report an xrun */
#define XRUN_REPORT_DELAY 0


AlsaDriver::AlsaDriver(AudioDevice* dev, int rate, nframes_t bufferSize)
		: Driver(dev, rate, bufferSize)
{
	read = MakeDelegate(this, &AlsaDriver::_read);
	write = MakeDelegate(this, &AlsaDriver::_write);
	run_cycle = RunCycleCallback(this, &AlsaDriver::_run_cycle);
}

AlsaDriver::~AlsaDriver()
{
	PENTERDES;
	if (capture_handle) {
		snd_pcm_close (capture_handle);
	}

	if (playback_handle) {
		snd_pcm_close (playback_handle);
	}

	if (capture_hw_params) {
		snd_pcm_hw_params_free (capture_hw_params);
	}

	if (playback_hw_params) {
		snd_pcm_hw_params_free (playback_hw_params);
	}

	if (capture_sw_params) {
		snd_pcm_sw_params_free (capture_sw_params);
	}

	if (playback_sw_params) {
		snd_pcm_sw_params_free (playback_sw_params);
	}

	if (pfd) {
		free (pfd);
	}

	release_channel_dependent_memory ();
}


int AlsaDriver::setup(bool capture, bool playback)
{
	unsigned long user_nperiods = 2;
	char *playback_pcm_name = "hw:0";
	char *capture_pcm_name = "hw:0";
	int soft_mode = false;
	int monitor = false;
	int shorts_first = false;

	/* duplex is the default */
	if (!capture && !playback) {
		capture = true;
		playback = true;
	}


	int err;
	playback_handle = (snd_pcm_t*) 0;
	capture_handle = (snd_pcm_t*) 0;
	ctl_handle = 0;
	capture_and_playback_not_synced = false;
	max_nchannels = 0;
	user_nchannels = 0;
	playback_nchannels = 0;
	capture_nchannels = 0;
	playback_sample_bytes = (shorts_first ? 2:4);
	capture_sample_bytes = (shorts_first ? 2:4);
	capture_frame_latency = 0;
	playback_frame_latency = 0;
	channels_done = 0;
	channels_not_done = 0;

	playback_addr = 0;
	capture_addr = 0;

	playback_hw_params = 0;
	capture_hw_params = 0;
	playback_sw_params = 0;
	capture_sw_params = 0;


	silent = 0;
	all_monitor_in = false;
	with_monitor_ports = monitor;

	input_monitor_mask = 0;   /* XXX is it? */


	pfd = 0;
	playback_nfds = 0;
	capture_nfds = 0;

	dither = None;
	soft_mode = soft_mode;

	pthread_mutex_init (&clock_sync_lock, 0);

	poll_late = 0;
	xrun_count = 0;
	process_count = 0;

	alsa_name_playback = strdup (playback_pcm_name);
	alsa_name_capture = strdup (capture_pcm_name);


	printf ("creating alsa driver ... %s|%s|%d|%lu|%d|%d|%d|%s|%s\n",
		playback ? playback_pcm_name : "-",
		capture ? capture_pcm_name : "-",
		frames_per_cycle, user_nperiods, frame_rate,
		(int)capture_nchannels, (int)playback_nchannels,
		soft_mode ? "soft-mode":"-",
		shorts_first ? "16bit":"32bit");


	if (playback) {
		if (snd_pcm_open (&playback_handle, alsa_name_playback, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) {
			switch (errno) {
			case EBUSY:
				printf ("ALSA Driver: The playback device \"%s\" is already in use. Please stop the"
					" application using it and run Traverso again\n",  playback_pcm_name);
				return -1;
				break;

			case EPERM:
				printf ("ALSA Driver: You do not have permission to open the audio device \"%s\" for playback\n", playback_pcm_name);
				return -1;
				break;
			default:
				PERROR ("snd_pcm_open(playback_handle, ..) failed with unknown error type");
			}

			playback_handle = 0;
		}

		if (playback_handle) {
			snd_pcm_nonblock (playback_handle, 0);
		}
	}

	if (capture) {
		if (snd_pcm_open (&capture_handle, alsa_name_capture, SND_PCM_STREAM_CAPTURE,  SND_PCM_NONBLOCK) < 0) {
			switch (errno) {
			case EBUSY:
				PWARN ("ALSA Driver: The capture device \"%s\" is already in use. Please stop the"
				" application using it and run Traverso again", capture_pcm_name);
				return -1;
				break;

			case EPERM:
				PWARN ("ALSA Driver: You do not have permission to open the audio device \"%s\" for capture", capture_pcm_name);
				return -1;
				break;
			default:
				PERROR ("snd_pcm_open(capture_handle, ...) failed with unknown error type");
			}

			capture_handle = 0;
		}

		if (capture_handle) {
			snd_pcm_nonblock (capture_handle, 0);
		}
	}

	if (playback_handle == 0) {
		if (playback) {

			/* they asked for playback, but we can't do it */

			PWARN ("ALSA: Cannot open PCM device %s for playback. Falling back to capture-only mode", "alsa_pcm");

			if (capture_handle == 0) {
				/* can't do anything */
				return -1;
			}
			playback = false;
		}
	}

	if (capture_handle == 0) {
		if (capture) {

			/* they asked for capture, but we can't do it */

			PWARN ("ALSA: Cannot open PCM device %s for capture. Falling back to playback-only mode", "alsa_pcm");

			if (playback_handle == 0) {
				/* can't do anything */
				return -1;
			}
			capture = false;
		}
	}


	if (playback_handle) {
		if ((err = snd_pcm_hw_params_malloc (&playback_hw_params)) < 0) {
			PWARN ("ALSA: could not allocate playback hw params structure");
			return -1;
		}

		if ((err = snd_pcm_sw_params_malloc (&playback_sw_params)) < 0) {
			PWARN ("ALSA: could not allocate playback sw params structure");
			return -1;
		}
	}

	if (capture_handle) {
		if ((err = snd_pcm_hw_params_malloc (&capture_hw_params)) < 0) {
			PWARN ("ALSA: could not allocate capture hw params structure");
			return -1;
		}

		if ((err = snd_pcm_sw_params_malloc (&capture_sw_params)) < 0) {
			PWARN ("ALSA: could not allocate capture sw params structure");
			return -1;
		}
	}

	if (set_parameters (frames_per_cycle, user_nperiods, frame_rate)) {
		return -1;
	}

	capture_and_playback_not_synced = false;

	if (capture_handle && playback_handle) {
		if (snd_pcm_link (capture_handle, playback_handle) != 0) {
			capture_and_playback_not_synced = true;
		}
	}


	return 1;
}



void AlsaDriver::release_channel_dependent_memory ()
{
	bitset_destroy (&channels_done);
	bitset_destroy (&channels_not_done);

	if (playback_addr) {
		free(playback_addr);
		playback_addr = 0;
	}

	if (capture_addr) {
		free(capture_addr);
		capture_addr = 0;
	}

	if (silent) {
		free(silent);
		silent = 0;
	}

	/*	if (dither_state) {
			delete dither_state;
			dither_state = 0;
		}*/
}


void AlsaDriver::setup_io_function_pointers()
{
	switch (playback_sample_bytes) {
	case 2:
		if (playback_interleaved) {
			channel_copy = memcpy_interleave_d16_s16;
		} else {
			channel_copy = memcpy_fake;
		}

		switch (dither) {
		case Rectangular:
			fprintf (stderr,"Rectangular dithering at 16 bits\n");
			write_via_copy = sample_move_dither_rect_d16_sS;
			break;

		case Triangular:
			printf("Triangular dithering at 16 bits\n");
			write_via_copy = sample_move_dither_tri_d16_sS;
			break;

		case Shaped:
			printf("Noise-shaped dithering at 16 bits\n");
			write_via_copy = sample_move_dither_shaped_d16_sS;
			break;

		default:
			write_via_copy = sample_move_d16_sS;
			break;
		}
		break;

	case 3:
		if (playback_interleaved) {
			channel_copy = memcpy_interleave_d24_s24;
		} else {
			channel_copy = memcpy_fake;
		}

		switch (dither) {
		case Rectangular:
			printf("Rectangular dithering at 24 bits\n");
			write_via_copy = sample_move_dither_rect_d24_sS;
			break;

		case Triangular:
			printf("Triangular dithering at 24 bits\n");
			write_via_copy = sample_move_dither_tri_d24_sS;
			break;

		case Shaped:
			printf("Noise-shaped dithering at 24 bits\n");
			write_via_copy = sample_move_dither_shaped_d24_sS;
			break;

		default:
			write_via_copy = sample_move_d24_sS;
			break;
		}
		break;

	case 4:
		if (playback_interleaved) {
			channel_copy = memcpy_interleave_d32_s32;
		} else {
			channel_copy = memcpy_fake;
		}

		switch (dither) {
		case Rectangular:
			printf("Rectangular dithering at 32 bits\n");
			write_via_copy = sample_move_dither_rect_d32u24_sS;
			break;

		case Triangular:
			printf("Triangular dithering at 32 bits\n");
			write_via_copy = sample_move_dither_tri_d32u24_sS;
			break;

		case Shaped:
			printf("Noise-shaped dithering at 32 bits\n");
			write_via_copy = sample_move_dither_shaped_d32u24_sS;
			break;

		default:
			write_via_copy = sample_move_d32u24_sS;
			break;
		}
		break;
	}

	switch (capture_sample_bytes) {
	case 2:
		read_via_copy = sample_move_dS_s16;
		break;
	case 3:
		read_via_copy = sample_move_dS_s24;
		break;
	case 4:
		read_via_copy = sample_move_dS_s32u24;
		break;
	}
}

int AlsaDriver::configure_stream(char *device_name,
				const char *stream_name,
				snd_pcm_t *handle,
				snd_pcm_hw_params_t *hw_params,
				snd_pcm_sw_params_t *sw_params,
				unsigned int *nperiodsp,
				unsigned long *nchns,
				unsigned long sample_width)
{
	int err, format;
	snd_pcm_uframes_t stop_th;
	static struct {
		char Name[16];
		snd_pcm_format_t format;
		int bitdepth;
	}
	formats[] = {
			{"32bit", SND_PCM_FORMAT_S32, 32},
			{"24bit", SND_PCM_FORMAT_S24_3, 24},
			{"16bit", SND_PCM_FORMAT_S16, 16},
		};
#define NOFORMATS (sizeof(formats)/sizeof(formats[0]))

	if ((err = snd_pcm_hw_params_any (handle, hw_params)) < 0)  {
		PERROR ("ALSA: no playback configurations available (%s)", snd_strerror (err));
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_periods_integer (handle, hw_params))  < 0) {
		qDebug ("ALSA: cannot restrict period size to integral value.");
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_access (handle, hw_params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED)) < 0) {
		if ((err = snd_pcm_hw_params_set_access ( handle, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED)) < 0) {
			qDebug ("ALSA: mmap-based access is not possible for the %s "
				"stream of this audio interface", stream_name);
			return -1;
		}
	}

	format = (sample_width == 4) ? 0 : (NOFORMATS - 1);
	while (1) {
		if ((err = snd_pcm_hw_params_set_format ( handle, hw_params, formats[format].format)) < 0) {
			int failed_format = format;
			if (( (sample_width == 4) ? (format++ < (int)NOFORMATS - 1) : (format-- > 0))) {
				fprintf (stderr,"Note: audio device %s doesn't support a %s sample format"
					" so Traverso will try a %s format instead\n", device_name,
					formats[failed_format].Name,
					formats[format].Name);
			} else {
				PERROR ("ALSA Driver: Sorry. The audio interface \"%s\" doesn't support any of the"
					" hardware sample formats that Traverso's alsa-driver can use.", device_name);
				return -1;
			}
		} else {
			device->set_bit_depth(formats[format].bitdepth);
			break;
		}
	}

	if ( (err = snd_pcm_hw_params_set_rate_near (handle, hw_params, &frame_rate, NULL)) < 0) {
		PERROR ("ALSA: cannot set sample/frame rate to % for %s", (double)frame_rate, stream_name);
		return -1;
	}

	if (!*nchns) {
		/*if not user-specified, try to find the maximum number of channels */
		unsigned int channels_max ;
		err = snd_pcm_hw_params_get_channels_max (hw_params, &channels_max);
		*nchns = channels_max;

		if (*nchns > 1024) {

			/* the hapless user is an unwitting victim of
			the "default" ALSA PCM device, which can
			support up to 16 million channels. since
			they can't be bothered to set up a proper
			default device, limit the number of
			channels for them to a sane default.
			*/

			PERROR (
				"ALSA Driver: You appear to be using the ALSA software \"plug\" layer, probably\n"
				"a result of using the \"default\" ALSA device. This is less\n"
				"efficient than it could be. Consider using a hardware device\n"
				"instead rather than using the plug layer. Usually the name of the\n"
				"hardware device that corresponds to the first sound card is hw:0\n"
			);
			*nchns = 2;
		}
	}

	if ((err = snd_pcm_hw_params_set_channels (handle, hw_params, *nchns)) < 0) {
		qWarning ("ALSA: cannot set channel count to %lu for %s", *nchns, stream_name);
		return -1;
	}
	int frperscycle = frames_per_cycle;
	if ((err = snd_pcm_hw_params_set_period_size (handle, hw_params, frames_per_cycle, 0)) < 0) {
		qWarning ("ALSA: cannot set period size to %d frames for %s", frperscycle, stream_name);
		return -1;
	}

	*nperiodsp = user_nperiods;
	snd_pcm_hw_params_set_periods_min (handle, hw_params, nperiodsp, NULL);
	if (*nperiodsp < user_nperiods)
		*nperiodsp = user_nperiods;

	if (snd_pcm_hw_params_set_periods_near (handle, hw_params, nperiodsp, NULL) < 0) {
		PERROR ("ALSA: cannot set number of periods to %u for %s", *nperiodsp, stream_name);
		return -1;
	}

	if (*nperiodsp < user_nperiods) {
		PERROR ("ALSA: got smaller periods %u than %u for %s",
			*nperiodsp, (unsigned int) user_nperiods,
			stream_name);
		return -1;
	}

	if (!is_power_of_two(frames_per_cycle)) {
		PERROR("Traverso: frames must be a power of two (64, 512, 1024, ...)");
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_buffer_size (handle, hw_params,  *nperiodsp * frames_per_cycle)) < 0) {
		PERROR ("ALSA: cannot set buffer length to %d for %s", *nperiodsp * frames_per_cycle, stream_name);
		return -1;
	}

	if ((err = snd_pcm_hw_params (handle, hw_params)) < 0) {
		PERROR ("ALSA: cannot set hardware parameters for %s", stream_name);
		return -1;
	}

	snd_pcm_sw_params_current (handle, sw_params);

	if ((err = snd_pcm_sw_params_set_start_threshold (handle, sw_params, 0U)) < 0) {
		PERROR ("ALSA: cannot set start mode for %s", stream_name);
		return -1;
	}

	stop_th = *nperiodsp * frames_per_cycle;
	if (soft_mode) {
		stop_th = (snd_pcm_uframes_t)-1;
	}

	if ((err = snd_pcm_sw_params_set_stop_threshold (handle, sw_params, stop_th)) < 0) {
		PERROR ("ALSA: cannot set stop mode for %s", stream_name);
		return -1;
	}

	if ((err = snd_pcm_sw_params_set_silence_threshold (handle, sw_params, 0)) < 0) {
		PERROR ("ALSA: cannot set silence threshold for %s", stream_name);
		return -1;
	}

#if 0
	fprintf (stderr, "set silence size to %lu * %lu = %lu\n",
		frames_per_cycle, *nperiodsp,
		frames_per_cycle * *nperiodsp);

	if ((err = snd_pcm_sw_params_set_silence_size (
				handle, sw_params,
				frames_per_cycle * *nperiodsp)) < 0) {
		PERROR ("ALSA: cannot set silence size for %s",
			stream_name);
		return -1;
	}
#endif

	if (handle == playback_handle)
		err = snd_pcm_sw_params_set_avail_min (handle, sw_params, frames_per_cycle * (*nperiodsp - user_nperiods + 1));
	else
		err = snd_pcm_sw_params_set_avail_min (handle, sw_params, frames_per_cycle);

	if (err < 0) {
		PERROR ("ALSA: cannot set avail min for %s", stream_name);
		return -1;
	}

	if ((err = snd_pcm_sw_params (handle, sw_params)) < 0) {
		PERROR ("ALSA: cannot set software parameters for %s\n",
			stream_name);
		return -1;
	}

	return 0;
}

int  AlsaDriver::set_parameters (nframes_t frames_per_interupt,
				nframes_t nperiods,
				nframes_t rate)
{
	int dir;
	snd_pcm_uframes_t p_period_size = 0;
	snd_pcm_uframes_t c_period_size = 0;
	channel_t chn;
	unsigned int pr = 0;
	unsigned int cr = 0;
	int err;

	frame_rate = rate;
	frames_per_cycle = frames_per_interupt;
	user_nperiods = nperiods;

	fprintf (stderr, "configuring for %d Hz, period = %ld frames, buffer = %ld periods\n", rate, (long)frames_per_cycle, user_nperiods);

	if (capture_handle) {
		if (configure_stream (
					alsa_name_capture,
					"capture",
					capture_handle,
					capture_hw_params,
					capture_sw_params,
					&capture_nperiods,
					&capture_nchannels,
					capture_sample_bytes)) {
			PERROR ("ALSA: cannot configure capture channel");
			return -1;
		}
	}

	if (playback_handle) {
		if (configure_stream (
					alsa_name_playback,
					"playback",
					playback_handle,
					playback_hw_params,
					playback_sw_params,
					&playback_nperiods,
					&playback_nchannels,
					playback_sample_bytes)) {
			PERROR ("ALSA: cannot configure playback channel");
			return -1;
		}
	}

	/* check the rate, since thats rather important */

	if (playback_handle) {
		snd_pcm_hw_params_get_rate (playback_hw_params, &pr, &dir);
	}

	if (capture_handle) {
		snd_pcm_hw_params_get_rate (capture_hw_params, &cr, &dir);
	}

	if (capture_handle && playback_handle) {
		if (cr != pr) {
			PERROR ("ALSA Driver: playback and capture sample rates do not match (%d vs. %d)", pr, cr);
		}

		/* only change if *both* capture and playback rates
		* don't match requested certain hardware actually
		* still works properly in full-duplex with slightly
		* different rate values between adc and dac
		*/
		if (cr != frame_rate && pr != frame_rate) {
			PERROR ("ALSA Driver: sample rate in use (%d Hz) does not match requested rate (%d Hz)", cr, frame_rate);
			frame_rate = cr;
		}

	} else if (capture_handle && cr != frame_rate) {
		PERROR ("ALSA Driver: capture sample rate in use (%d Hz) does not match requested rate (%d Hz)", cr, frame_rate);
		frame_rate = cr;
	} else if (playback_handle && pr != frame_rate) {
		PERROR ("ALSA Driver: playback sample rate in use (%d Hz) does not match requested rate (%d Hz)", pr, frame_rate);
		frame_rate = pr;
	}


	/* check the fragment size, since thats non-negotiable */

	if (playback_handle) {
		snd_pcm_access_t access;

		err = snd_pcm_hw_params_get_period_size (playback_hw_params, &p_period_size, &dir);
		err = snd_pcm_hw_params_get_format (playback_hw_params,	&playback_sample_format);
		err = snd_pcm_hw_params_get_access (playback_hw_params, &access);
		playback_interleaved = (access == SND_PCM_ACCESS_MMAP_INTERLEAVED);

		if (p_period_size != frames_per_cycle) {
			PERROR ("alsa_pcm: requested an interrupt every %ld frames but got %ld frames for playback", (long)frames_per_cycle, p_period_size);
			return -1;
		}
	}

	if (capture_handle) {
		snd_pcm_access_t access;

		err = snd_pcm_hw_params_get_period_size (capture_hw_params, &c_period_size, &dir);
		err = snd_pcm_hw_params_get_format (capture_hw_params, &(capture_sample_format));
		err = snd_pcm_hw_params_get_access (capture_hw_params, &access);
		capture_interleaved = (access == SND_PCM_ACCESS_MMAP_INTERLEAVED);


		if (c_period_size != frames_per_cycle) {
			PERROR ("alsa_pcm: requested an interrupt every %ld frames but got %ld frames for capture", (long)frames_per_cycle, p_period_size);
			return -1;
		}
	}

	playback_sample_bytes =	snd_pcm_format_physical_width(playback_sample_format)	/ 8;
	capture_sample_bytes =	snd_pcm_format_physical_width(capture_sample_format)	/ 8;

	if (playback_handle) {
		switch (playback_sample_format) {
		case SND_PCM_FORMAT_S32_LE:
		case SND_PCM_FORMAT_S24_3:
		case SND_PCM_FORMAT_S16_LE:
		case SND_PCM_FORMAT_S32_BE:
		case SND_PCM_FORMAT_S16_BE:
			break;

		default:
			PERROR ("ALSA Driver: programming error: unhandled format type for playback");
			return -1;
		}
	}

	if (capture_handle) {
		switch (capture_sample_format) {
		case SND_PCM_FORMAT_S32_LE:
		case SND_PCM_FORMAT_S24_3:
		case SND_PCM_FORMAT_S16_LE:
		case SND_PCM_FORMAT_S32_BE:
		case SND_PCM_FORMAT_S16_BE:
			break;

		default:
			PERROR ("ALSA Driver: programming error: unhandled format type for capture");
			return -1;
		}
	}

	if (playback_interleaved) {
		const snd_pcm_channel_area_t *my_areas;
		snd_pcm_uframes_t offset, frames;
		if (snd_pcm_mmap_begin(playback_handle, &my_areas, &offset, &frames) < 0) {
			PERROR ("ALSA: %s: mmap areas info error", alsa_name_playback);
			return -1;
		}
		playback_interleave_skip = my_areas[0].step / 8;
		interleave_unit = snd_pcm_format_physical_width(playback_sample_format) / 8;
	} else {
		interleave_unit = 0;  /* NOT USED */
		playback_interleave_skip = snd_pcm_format_physical_width(playback_sample_format) / 8;
	}

	if (capture_interleaved) {
		const snd_pcm_channel_area_t *my_areas;
		snd_pcm_uframes_t offset, frames;
		if (snd_pcm_mmap_begin(capture_handle, &my_areas, &offset, &frames) < 0) {
			PERROR ("ALSA: %s: mmap areas info error", alsa_name_capture);
			return -1;
		}
		capture_interleave_skip = my_areas[0].step / 8;
	} else {
		capture_interleave_skip = snd_pcm_format_physical_width(capture_sample_format) / 8;
	}

	if (playback_nchannels > capture_nchannels) {
		max_nchannels = playback_nchannels;
		user_nchannels = capture_nchannels;
	} else {
		max_nchannels = capture_nchannels;
		user_nchannels = playback_nchannels;
	}

	setup_io_function_pointers ();

	/* Allocate and initialize structures that rely on the
	channels counts.

	Set up the bit pattern that is used to record which
	channels require action on every cycle. any bits that are
	not set after the engine's process() call indicate channels
	that potentially need to be silenced.
	*/

	bitset_create (&channels_done, max_nchannels);
	bitset_create (&channels_not_done, max_nchannels);

	if (playback_handle) {
		playback_addr =  (char**) malloc (sizeof (char *) * playback_nchannels);
		memset (playback_addr, 0, sizeof (char *) * playback_nchannels);
		silent = (unsigned long *) malloc (sizeof (unsigned long) * playback_nchannels);

		for (chn = 0; chn < playback_nchannels; chn++) {
			silent[chn] = 0;
		}

		for (chn = 0; chn < playback_nchannels; chn++) {
			bitset_add (channels_done, chn);
		}

		dither_state = (dither_state_t *) calloc ( playback_nchannels, sizeof (dither_state_t));
	}

	if (capture_handle) {
		capture_addr = (char **) malloc (sizeof (char *) * capture_nchannels);
		memset (capture_addr, 0, sizeof (char *) * capture_nchannels);
	}

	period_usecs = (trav_time_t) floor ((((float) frames_per_cycle) / frame_rate) * 1000000.0f);
	poll_timeout = (int) floor (1.5f * period_usecs);

	device->set_buffer_size(frames_per_cycle);

	return 0;
}

int AlsaDriver::reset_parameters (nframes_t frames_per_cycle,
				nframes_t user_nperiods,
				nframes_t rate)
{
	/* XXX unregister old ports ? */
	release_channel_dependent_memory ();
	return set_parameters (frames_per_cycle, user_nperiods, rate);
}

int AlsaDriver::get_channel_addresses (snd_pcm_uframes_t *capture_avail,
				snd_pcm_uframes_t *playback_avail,
				snd_pcm_uframes_t *capture_offset,
				snd_pcm_uframes_t *playback_offset)
{
	int err;
	channel_t chn;

	if (capture_avail) {
		if ((err = snd_pcm_mmap_begin (capture_handle, &capture_areas, capture_offset, capture_avail)) < 0) {
			PERROR ("ALSA: %s: mmap areas info error", alsa_name_capture);
			return -1;
		}

		for (chn = 0; chn < capture_nchannels; chn++) {
			const snd_pcm_channel_area_t *a = &capture_areas[chn];
			capture_addr[chn] = (char *) a->addr + ((a->first + a->step * *capture_offset) / 8);
		}
	}

	if (playback_avail) {
		if ((err = snd_pcm_mmap_begin (playback_handle, &playback_areas, playback_offset, playback_avail)) < 0) {
			PERROR ("ALSA: %s: mmap areas info error ", alsa_name_playback);
			return -1;
		}

		for (chn = 0; chn < playback_nchannels; chn++) {
			const snd_pcm_channel_area_t *a = &playback_areas[chn];
			playback_addr[chn] = (char *) a->addr + ((a->first + a->step * *playback_offset) / 8);
		}
	}

	return 0;
}

int AlsaDriver::start()
{
	int err;
	snd_pcm_uframes_t poffset, pavail;
	channel_t chn;

	poll_last = 0;
	poll_next = 0;

	if (playback_handle) {
		if ((err = snd_pcm_prepare (playback_handle)) < 0) {
			PERROR ("ALSA: prepare error for playback on \"%s\" (%s)", alsa_name_playback, snd_strerror(err));
			return -1;
		}
	}

	if ((capture_handle && capture_and_playback_not_synced)  || !playback_handle) {
		if ((err = snd_pcm_prepare (capture_handle)) < 0) {
			PERROR ("ALSA: prepare error for capture on \"%s\" (%s)", alsa_name_capture,  snd_strerror(err));
			return -1;
		}
	}


	if (playback_handle) {
		playback_nfds = snd_pcm_poll_descriptors_count (playback_handle);
	} else {
		playback_nfds = 0;
	}

	if (capture_handle) {
		capture_nfds = snd_pcm_poll_descriptors_count (capture_handle);
	} else {
		capture_nfds = 0;
	}

	if (pfd) {
		free (pfd);
	}

	pfd = (struct pollfd *)	malloc (sizeof (struct pollfd) * (playback_nfds + capture_nfds + 2));

	if (playback_handle) {
		/* fill playback buffer with zeroes, and mark
		all fragments as having data.
		*/

		pavail = snd_pcm_avail_update (playback_handle);

		if (pavail !=  frames_per_cycle * playback_nperiods) {
			PERROR ("ALSA: full buffer not available at start");
			return -1;
		}

		if (get_channel_addresses (0, &pavail, 0, &poffset)) {
			return -1;
		}

		/* XXX this is cheating. ALSA offers no guarantee that
		we can access the entire buffer at any one time. It
		works on most hardware tested so far, however, buts
		its a liability in the long run. I think that
		alsa-lib may have a better function for doing this
		here, where the goal is to silence the entire
		buffer.
		*/

		for (chn = 0; chn < playback_nchannels; chn++) {
			silence_on_channel (chn, user_nperiods * frames_per_cycle);
		}

		snd_pcm_mmap_commit (playback_handle, poffset, user_nperiods * frames_per_cycle);

		if ((err = snd_pcm_start (playback_handle)) < 0) {
			PERROR ("ALSA: could not start playback (%s)", snd_strerror (err));
			return -1;
		}
	}

	if ((capture_handle && capture_and_playback_not_synced)  || !playback_handle) {
		if ((err = snd_pcm_start (capture_handle)) < 0) {
			PERROR ("ALSA: could not start capture (%s)", snd_strerror (err));
			return -1;
		}
	}

	return 0;
}

int AlsaDriver::stop()
{
	int err;
	audio_sample_t* buf;

	/* silence all capture port buffers, because we might
	be entering offline mode.
	*/

	for (int i=0; i<captureChannels.size(); ++i) {
		AudioChannel* chan = captureChannels.at(i);
		buf = chan->get_buffer(frames_per_cycle);
		memset (buf, 0, sizeof (audio_sample_t) * frames_per_cycle);
	}

	if (playback_handle) {
		if ((err = snd_pcm_drop (playback_handle)) < 0) {
			PERROR ("ALSA: channel flush for playback failed (%s)", snd_strerror (err));
			return -1;
		}
	}

	if (!playback_handle || capture_and_playback_not_synced) {
		if (capture_handle) {
			if ((err = snd_pcm_drop (capture_handle)) < 0) {
				PERROR ("ALSA: channel flush for capture failed (%s)", snd_strerror (err));
				return -1;
			}
		}
	}


	return 0;
}

int AlsaDriver::restart()
{
	if (stop())
		return -1;
	return start();
}

int AlsaDriver::xrun_recovery (float *delayed_usecs)
{
	PWARN("xrun");
	snd_pcm_status_t *status;
	int res;

	snd_pcm_status_alloca(&status);

	if (capture_handle) {
		if ((res = snd_pcm_status(capture_handle, status))  < 0) {
			printf ("status error: %s", snd_strerror(res));
		}
	} else {
		if ((res = snd_pcm_status(playback_handle, status)) < 0) {
			printf ("status error: %s", snd_strerror(res));
		}
	}

	if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN && process_count > XRUN_REPORT_DELAY) {
		struct timeval now, diff, tstamp;
		xrun_count++;
		gettimeofday(&now, 0);
		snd_pcm_status_get_trigger_tstamp(status, &tstamp);
		timersub(&now, &tstamp, &diff);
		*delayed_usecs = diff.tv_sec * 1000000.0 + diff.tv_usec;
		printf ("\n**** alsa_pcm: xrun of at least %.3f msecs\n\n", *delayed_usecs / 1000.0);
		device->xrun();
	}

	if (restart()) {
		return -1;
	}

	return 0;
}

void AlsaDriver::silence_untouched_channels (nframes_t nframes)
{
	channel_t chn;
	nframes_t buffer_frames = frames_per_cycle * playback_nperiods;

	for (chn = 0; chn < playback_nchannels; chn++) {
		if (bitset_contains (channels_not_done, chn)) {
			if (silent[chn] < buffer_frames) {
				silence_on_channel_no_mark (chn, nframes);
				silent[chn] += nframes;
			}
		}
	}
}

static int under_gdb = false;

int AlsaDriver::wait(int extra_fd, int *status, float *delayed_usecs)
{
	snd_pcm_sframes_t avail = 0;
	snd_pcm_sframes_t capture_avail = 0;
	snd_pcm_sframes_t playback_avail = 0;
	int xrun_detected = false;
	int need_capture;
	int need_playback;
	unsigned int i;
	trav_time_t poll_enter;
	trav_time_t poll_ret = 0;

	*status = -1;
	*delayed_usecs = 0;

	need_capture = capture_handle ? 1 : 0;

	if (extra_fd >= 0) {
		need_playback = 0;
	} else {
		need_playback = playback_handle ? 1 : 0;
	}

again:

	while (need_playback || need_capture) {

		unsigned int p_timed_out, c_timed_out;
		unsigned int ci = 0;
		unsigned int nfds;

		nfds = 0;

		if (need_playback) {
			snd_pcm_poll_descriptors (playback_handle, &pfd[0], playback_nfds);
			nfds += playback_nfds;
		}

		if (need_capture) {
			snd_pcm_poll_descriptors (capture_handle, &pfd[nfds], capture_nfds);
			ci = nfds;
			nfds += capture_nfds;
		}

		/* ALSA doesn't set POLLERR in some versions of 0.9.X */

		for (i = 0; i < nfds; i++) {
			pfd[i].events |= POLLERR;
		}

		if (extra_fd >= 0) {
			pfd[nfds].fd = extra_fd;
			pfd[nfds].events =
				POLLIN|POLLERR|POLLHUP|POLLNVAL;
			nfds++;
		}

		poll_enter = get_microseconds ();

		if (poll_enter > poll_next) {
			/*
			* This processing cycle was delayed past the
			* next due interrupt!  Do not account this as
			* a wakeup delay:
			*/
			poll_next = 0;
			poll_late++;
		}

		device->transport_cycle_end(poll_enter);

		if (poll (pfd, nfds, poll_timeout) < 0) {

			if (errno == EINTR) {
				printf ("poll interrupt\n");
				// this happens mostly when run
				// under gdb, or when exiting due to a signal
				if (under_gdb) {
					goto again;
				}
				*status = -2;
				return 0;
			}

			PERROR ("ALSA: poll call failed (%s)",
				strerror (errno));
			*status = -3;
			return 0;

		}

		poll_ret = get_microseconds ();

		if (extra_fd < 0) {
			if (poll_next && poll_ret > poll_next) {
				*delayed_usecs = poll_ret - poll_next;
			}
			poll_last = poll_ret;
			poll_next = poll_ret + period_usecs;
			device->transport_cycle_start (poll_ret);
		}

#ifdef DEBUG_WAKEUP
		fprintf (stderr, "%" PRIu64 ": checked %d fds, %" PRIu64
			" usecs since poll entered\n", poll_ret, nfds,
			poll_ret - poll_enter);
#endif

		/* check to see if it was the extra FD that caused us
		* to return from poll */

		if (extra_fd >= 0) {

			if (pfd[nfds-1].revents == 0) {
				/* we timed out on the extra fd */

				*status = -4;
				return -1;
			}

			/* if POLLIN was the only bit set, we're OK */

			*status = 0;
			return (pfd[nfds-1].revents == POLLIN) ? 0 : -1;
		}

		p_timed_out = 0;

		if (need_playback) {
			for (i = 0; i < playback_nfds; i++) {
				if (pfd[i].revents & POLLERR) {
					PWARN("playback pollerror, xrun_detected is true");
					xrun_detected = true;
				}

				if (pfd[i].revents == 0) {
					p_timed_out++;
#ifdef DEBUG_WAKEUP

					fprintf (stderr, "%" PRIu64
						" playback stream timed out\n",
						poll_ret);
#endif

				}
			}

			if (p_timed_out == 0) {
				need_playback = 0;
#ifdef DEBUG_WAKEUP

				fprintf (stderr, "%" PRIu64
					" playback stream ready\n",
					poll_ret);
#endif

			}
		}

		c_timed_out = 0;

		if (need_capture) {
			for (i = ci; i < nfds; i++) {
				if (pfd[i].revents & POLLERR) {
					PWARN("capture pollerror, xrun_detected is true");
					xrun_detected = true;
				}

				if (pfd[i].revents == 0) {
					c_timed_out++;
#ifdef DEBUG_WAKEUP

					fprintf (stderr, "%" PRIu64
						" capture stream timed out\n",
						poll_ret);
#endif

				}
			}

			if (c_timed_out == 0) {
				need_capture = 0;
#ifdef DEBUG_WAKEUP

				fprintf (stderr, "%" PRIu64
					" capture stream ready\n",
					poll_ret);
#endif

			}
		}

		if ((p_timed_out && (p_timed_out == playback_nfds)) &&
				(c_timed_out && (c_timed_out == capture_nfds))) {
			PERROR ("ALSA: poll time out, polled for %ld usecs", (long)(poll_ret - poll_enter));
			*status = -5;
			return 0;
		}

	}

	if (capture_handle) {
		if ((capture_avail = snd_pcm_avail_update (capture_handle)) < 0) {
			if (capture_avail == -EPIPE) {
				xrun_detected = true;
			} else {
				PERROR ("ALSA Driver: unknown avail_update return value (%ld)", capture_avail);
			}
		}
	} else {
		/* odd, but see min() computation below */
		capture_avail = INT_MAX;
	}

	if (playback_handle) {
		if ((playback_avail = snd_pcm_avail_update (playback_handle)) < 0) {
			if (playback_avail == -EPIPE) {
				xrun_detected = true;
			} else {
				PERROR ("ALSA Driver: unknown avail_update return value (%ld)", playback_avail);
			}
		}
	} else {
		/* odd, but see min() computation below */
		playback_avail = INT_MAX;
	}

	if (xrun_detected) {
		*status = xrun_recovery (delayed_usecs);
		return 0;
	}

	*status = 0;
	last_wait_ust = poll_ret;

	avail = capture_avail < playback_avail ? capture_avail : playback_avail;

#ifdef DEBUG_WAKEUP

	fprintf (stderr, "wakeup complete, avail = %lu, pavail = %lu "
		"cavail = %lu\n",
		avail, playback_avail, capture_avail);
#endif

	/* mark all channels not done for now. read/write will change this */

	bitset_copy (channels_not_done, channels_done);

	/* constrain the available count to the nearest (round down) number of
	periods.
	*/

	return avail - (avail % frames_per_cycle);
}

int AlsaDriver::_null_cycle(nframes_t nframes)
{
	nframes_t nf;
	snd_pcm_uframes_t offset;
	snd_pcm_uframes_t contiguous;
	uint chn;

	if (nframes > frames_per_cycle) {
		return -1;
	}

	if (capture_handle) {
		nf = nframes;
		offset = 0;
		while (nf) {
			contiguous = nf;

			if (snd_pcm_mmap_begin (
						capture_handle,
						&capture_areas,
						(snd_pcm_uframes_t *) &offset,
						(snd_pcm_uframes_t *) &contiguous)) {
				return -1;
			}

			if (snd_pcm_mmap_commit (capture_handle, offset, contiguous) < 0) {
				return -1;
			}

			nf -= contiguous;
		}
	}

	if (playback_handle) {
		nf = nframes;
		offset = 0;
		while (nf) {
			contiguous = nf;

			if (snd_pcm_mmap_begin (
						playback_handle,
						&playback_areas,
						(snd_pcm_uframes_t *) &offset,
						(snd_pcm_uframes_t *) &contiguous)) {
				return -1;
			}

			for (chn = 0; chn < playback_nchannels; chn++) {
				silence_on_channel (chn, contiguous);
			}

			if (snd_pcm_mmap_commit (playback_handle, offset, contiguous) < 0) {
				return -1;
			}

			nf -= contiguous;
		}
	}

	return 0;
}

int AlsaDriver::bufsize(nframes_t nframes)
{
	return reset_parameters (nframes, user_nperiods,frame_rate);
}

int AlsaDriver::_read(nframes_t nframes)
{
	snd_pcm_uframes_t contiguous;
	snd_pcm_uframes_t nread;
	snd_pcm_uframes_t offset;
	nframes_t  orig_nframes;
	audio_sample_t* buf;
	int err;

	if (!capture_handle) {
		return 0;
	}

	if (nframes > frames_per_cycle) {
		return -1;
	}

	nread = 0;
	contiguous = 0;
	orig_nframes = nframes;

	while (nframes) {

		contiguous = nframes;

		if (get_channel_addresses (&contiguous, (snd_pcm_uframes_t *) 0, &offset, 0) < 0) {
			return -1;
		}

		for (int i=0; i<captureChannels.size(); ++i) {
			AudioChannel* channel = captureChannels.at(i);
			
			if (!channel->has_data()) {
				//no-copy optimization
				continue;
			}
			buf = channel->get_data();
			read_from_channel (channel->get_number(), buf + nread, contiguous);
		}

		if ((err = snd_pcm_mmap_commit (capture_handle, offset, contiguous)) < 0) {
			PERROR ("ALSA: could not complete read of %ld frames: error = %d\n", contiguous, err);
			return -1;
		}

		nframes -= contiguous;
		nread += contiguous;
	}

	return 0;
}

int AlsaDriver::_write(nframes_t nframes)
{
	audio_sample_t* buf;
	nframes_t orig_nframes;
	snd_pcm_uframes_t nwritten;
	snd_pcm_uframes_t contiguous;
	snd_pcm_uframes_t offset;
	int err;

	process_count++;

	if (nframes > frames_per_cycle) {
		return -1;
	}

	nwritten = 0;
	contiguous = 0;
	orig_nframes = nframes;

	/* check current input monitor request status */

	input_monitor_mask = 0;

	while (nframes) {

		contiguous = nframes;

		if (get_channel_addresses ((snd_pcm_uframes_t *) 0, &contiguous, 0, &offset) < 0) {
			return -1;
		}

		for (int i=0; i<playbackChannels.size(); ++i) {
			AudioChannel* channel = playbackChannels.at(i);
			if (!channel->has_data()) {
				continue;
			}
			buf = channel->get_data();
			write_to_channel (channel->get_number(), buf + nwritten, contiguous);
			channel->silence_buffer(nframes);
		}


		if (!bitset_empty (channels_not_done)) {
			silence_untouched_channels (contiguous);
		}

		if ((err = snd_pcm_mmap_commit (playback_handle, offset, contiguous)) < 0) {
			PERROR ("ALSA: could not complete playback of %ld frames: error = %d", contiguous, err);
			if (err != EPIPE && err != ESTRPIPE)
				return -1;
		}

		nframes -= contiguous;
		nwritten += contiguous;
	}
	return 0;
}

int AlsaDriver::_run_cycle()
{
	int wait_status;
	float delayed_usecs;
	nframes_t nframes;

	// 	PWARN ("alsa run cycle wait\n");

	nframes = wait (-1, &wait_status, &delayed_usecs);


	if (nframes == 0) {

		/* we detected an xrun and restarted: notify
		* clients about the delay. 
		*/
		device->delay(delayed_usecs);
		return 0;
	}

	if (wait_status == 0)
		return device->run_cycle (nframes, delayed_usecs);

	if (wait_status < 0)
		return -1;		/* driver failed */
	else
		return 0;
}

int AlsaDriver::attach()
{
	char buf[32];
	channel_t chn;
	AudioChannel* chan;
	int port_flags;

	device->set_buffer_size (frames_per_cycle);
	device->set_sample_rate (frame_rate);

	port_flags = PortIsOutput|PortIsPhysical|PortIsTerminal;

	for (chn = 0; chn < capture_nchannels; chn++) {

		snprintf (buf, sizeof(buf) - 1, "capture_%lu", chn+1);

		chan = device->register_capture_channel(buf, JACK_DEFAULT_AUDIO_TYPE, port_flags, frames_per_cycle, chn);
		chan->set_latency( frames_per_cycle + capture_frame_latency );
		captureChannels.append(chan);

	}

	port_flags = PortIsInput|PortIsPhysical|PortIsTerminal;

	for (chn = 0; chn < playback_nchannels; chn++) {

		snprintf (buf, sizeof(buf) - 1, "playback_%lu", chn+1);

		chan = device->register_playback_channel(buf, JACK_DEFAULT_AUDIO_TYPE, port_flags, frames_per_cycle, chn);
		chan->set_latency( frames_per_cycle + capture_frame_latency );
		playbackChannels.append(chan);
	}


	// 	return jack_activate (client);
	return 1;
}

int AlsaDriver::detach ()
{
	return 0;
}



static int
dither_opt (char c, DitherAlgorithm* dither)
{
	switch (c) {
	case '-':
	case 'n':
		*dither = None;
		break;

	case 'r':
		*dither = Rectangular;
		break;

	case 's':
		*dither = Shaped;
		break;

	case 't':
		*dither = Triangular;
		break;

	default:
		fprintf (stderr, "ALSA driver: illegal dithering mode %c\n", c);
		return -1;
	}
	return 0;
}


/* DRIVER "PLUGIN" INTERFACE */

const char driver_client_name[] = "alsa_pcm";

const char* get_descriptor ()
{
	return "";
}

QString AlsaDriver::get_device_name( )
{
	return alsa_device_name(false);
}

QString AlsaDriver::get_device_longname( )
{
	return alsa_device_name(true);
}

QString AlsaDriver::alsa_device_name(bool longname)
{
	snd_ctl_card_info_t *info;
	snd_ctl_t *handle;
	char name[32];
	sprintf(name, "hw:%d", 0);
	int err = 0;

	snd_ctl_card_info_alloca(&info);

	if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
		PERROR("Control open (%i): %s", 0, snd_strerror(err));
	}

	if ((err = snd_ctl_card_info(handle, info)) < 0) {
		PERROR("Control hardware info (%i): %s", 0, snd_strerror(err));
	}

	snd_ctl_close(handle);

	if (err < 0)
		return "Device name unknown";

	/*	int dev = -1;
		if (snd_ctl_pcm_next_device(handle, &dev)<0)
			PERROR("No buses available on soundcard :-(");
		if (dev < 0)
			return "";
		
		snd_pcm_info_set_device(pcminfo, dev);
		snd_pcm_info_set_subdevice(pcminfo, 0);*/

	if (longname)
		return snd_ctl_card_info_get_name(info);

	return snd_ctl_card_info_get_id(info);
}

//eof
