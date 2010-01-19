/*
Copyright (C) 2005-2010 Remon Sijrier

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

*/

#ifndef ALSADRIVER_H
#define ALSADRIVER_H

#include "Driver.h"

#include <alsa/asoundlib.h>
#include <sys/param.h>
#include <memops.h>
#include "bitset.h"

#include "defines.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define IS_LE 0
#define IS_BE 1
#elif __BYTE_ORDER == __BIG_ENDIAN
#define IS_LE 1
#define IS_BE 0
#endif


class AlsaDriver : public Driver
{
public:
	AlsaDriver(AudioDevice* dev, int rate, nframes_t bufferSize);
	~AlsaDriver();

	int start();
	int stop();
	int _read(nframes_t nframes);
	int _write(nframes_t nframes);
	int _null_cycle(nframes_t nframes);
	int _run_cycle();
	int attach();
	int detach();
	int bufsize(nframes_t nframes);
	int restart();
	int setup(bool capture=true, bool playback=true, const QString& pcmName="hw:0", const QString& dither="None");

	QString get_device_name();
	QString get_device_longname();
	static QString alsa_device_name(bool longname = false, int devicenumber=0);

private:
	void setup_io_function_pointers();
	void silence_untouched_channels(nframes_t nframes);
	void release_channel_dependent_memory();
	int configure_stream(char *device_name,
			const char *stream_name,
			snd_pcm_t *handle,
			snd_pcm_hw_params_t *hw_params,
			snd_pcm_sw_params_t *sw_params,
			unsigned int *nperiodsp,
			unsigned long *nchns,
			unsigned long sample_width);
	int set_parameters(nframes_t frames_per_cycle, nframes_t user_nperiods, nframes_t rate);
	int xrun_recovery(float *delayed_usecs);
	int reset_parameters(nframes_t frames_per_cycle, nframes_t user_nperiods, nframes_t rate);
	int alsa_driver_set_parameters(nframes_t frames_per_cycle, nframes_t user_nperiods, nframes_t rate);
	int get_channel_addresses (	snd_pcm_uframes_t *capture_avail,
				snd_pcm_uframes_t *playback_avail,
				snd_pcm_uframes_t *capture_offset,
				snd_pcm_uframes_t *playback_offset
				);

	int wait(int extra_fd, int *status, float *delayed_usecs);


	inline void mark_channel_done (channel_t chn)
	{
		bitset_remove (channels_not_done, chn);
		silent[chn] = 0;
	}

	inline void silence_on_channel (channel_t chn, nframes_t nframes)
	{
		if (playback_interleaved) {
			memset_interleave
			(playback_addr[chn],
			0, nframes * playback_sample_bytes,
			interleave_unit,
			playback_interleave_skip[chn]);
		} else {
			memset (playback_addr[chn], 0,	nframes * playback_sample_bytes);
		}
		mark_channel_done (chn);
	}

	inline void silence_on_channel_no_mark (channel_t chn, nframes_t nframes)
	{
		if (playback_interleaved) {
			memset_interleave
			(playback_addr[chn],
			0, nframes * playback_sample_bytes,
			interleave_unit,
			playback_interleave_skip[chn]);
		} else {
			memset (playback_addr[chn], 0, nframes * playback_sample_bytes);
		}
	}

	inline void read_from_channel (channel_t channel, audio_sample_t *buf, nframes_t nsamples)
	{
		read_via_copy (buf, capture_addr[channel], nsamples, capture_interleave_skip[channel]);
	}

	inline void write_to_channel (channel_t channel, audio_sample_t *buf, nframes_t nsamples)
	{
		write_via_copy (playback_addr[channel], buf, nsamples, playback_interleave_skip[channel], dither_state + channel);
		mark_channel_done (channel);
	}

	inline void copy_channel (channel_t input_channel, channel_t output_channel, nframes_t nsamples)
	{
		channel_copy (	playback_addr[output_channel],
			capture_addr[input_channel],
			nsamples * playback_sample_bytes,
			playback_interleave_skip[output_channel],
			capture_interleave_skip[input_channel]);
		mark_channel_done (output_channel);
	}



	typedef void (*ReadCopyFunction)  (	audio_sample_t *dst,
					char *src,
					unsigned long src_bytes,
					unsigned long src_skip_bytes
					);
	typedef void (*WriteCopyFunction) (	char *dst,
					audio_sample_t *src,
					unsigned long src_bytes,
					unsigned long dst_skip_bytes,
					dither_state_t *state
					);
	typedef void (*CopyCopyFunction)  (	char *dst,
					char *src,
					unsigned long src_bytes,
					unsigned long dst_skip_bytes,
					unsigned long src_skip_byte
					);

	int                           poll_timeout;
	trav_time_t                   poll_last;
	trav_time_t                   poll_next;
	char                        **playback_addr;
	char                        **capture_addr;
	const snd_pcm_channel_area_t *capture_areas;
	const snd_pcm_channel_area_t *playback_areas;
	struct pollfd                *pfd;
	unsigned int                  playback_nfds;
	unsigned int                  capture_nfds;
	unsigned long                 interleave_unit;
	unsigned long                *capture_interleave_skip;
	unsigned long                *playback_interleave_skip;
	channel_t                     max_nchannels;
	channel_t                     user_nchannels;
	channel_t                     playback_nchannels;
	channel_t                     capture_nchannels;
	unsigned long                 playback_sample_bytes;
	unsigned long                 capture_sample_bytes;


	unsigned long                *silent;
	char                         *alsa_name_playback;
	char                         *alsa_name_capture;
	char                         *alsa_driver;
	bitset_t			channels_not_done;
	bitset_t			channels_done;
	snd_pcm_format_t              playback_sample_format;
	snd_pcm_format_t              capture_sample_format;
	float                         max_sample_val;
	unsigned long                 user_nperiods;
	unsigned int                  playback_nperiods;
	unsigned int                  capture_nperiods;
	unsigned long                 last_mask;
	snd_ctl_t                    *ctl_handle;
	snd_pcm_t                    *playback_handle;
	snd_pcm_t                    *capture_handle;
	snd_pcm_hw_params_t          *playback_hw_params;
	snd_pcm_sw_params_t          *playback_sw_params;
	snd_pcm_hw_params_t          *capture_hw_params;
	snd_pcm_sw_params_t          *capture_sw_params;

	char soft_mode;
	char capture_and_playback_not_synced;
	char playback_interleaved;
	char capture_interleaved;
	char quirk_bswap;

	ReadCopyFunction read_via_copy;
	WriteCopyFunction write_via_copy;
	CopyCopyFunction channel_copy;

	int process_count;

};

#endif

//eof
