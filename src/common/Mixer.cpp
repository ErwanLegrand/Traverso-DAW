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
 
    $Id: Mixer.cpp,v 1.1 2007/10/20 17:38:16 r_sijrier Exp $
*/

#include "Mixer.h"
#include "defines.h"
#include <cmath>

Mixer::compute_peak_t			Mixer::compute_peak 		= 0;
Mixer::apply_gain_to_buffer_t		Mixer::apply_gain_to_buffer 	= 0;
Mixer::mix_buffers_with_gain_t		Mixer::mix_buffers_with_gain 	= 0;
Mixer::mix_buffers_no_gain_t		Mixer::mix_buffers_no_gain 	= 0;



float default_compute_peak (audio_sample_t* buf, nframes_t nsamples, float current)
{
        for (nframes_t i = 0; i < nsamples; ++i) {
                current = f_max (current, fabsf (buf[i]));
        }

        return current;
}

void default_apply_gain_to_buffer (audio_sample_t* buf, nframes_t nframes, float gain)
{
        for (nframes_t i=0; i<nframes; i++)
                buf[i] *= gain;
}

void default_mix_buffers_with_gain (audio_sample_t* dst, audio_sample_t* src, nframes_t nframes, float gain)
{
        for (nframes_t i = 0; i < nframes; i++) {
                dst[i] += src[i] * gain;
        }
}

void default_mix_buffers_no_gain (audio_sample_t* dst, audio_sample_t* src, nframes_t nframes)
{
        for (nframes_t i=0; i < nframes; i++) {
                dst[i] += src[i];
        }
}
