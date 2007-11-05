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
 
    $Id: Mixer.cpp,v 1.3 2007/11/05 19:19:23 r_sijrier Exp $
*/

#include "Mixer.h"
#include "defines.h"
#include <cmath> // used for fabs

Mixer::compute_peak_t			Mixer::compute_peak 		= 0;
Mixer::apply_gain_to_buffer_t		Mixer::apply_gain_to_buffer 	= 0;
Mixer::mix_buffers_with_gain_t		Mixer::mix_buffers_with_gain 	= 0;
Mixer::mix_buffers_no_gain_t		Mixer::mix_buffers_no_gain 	= 0;



float default_compute_peak (const audio_sample_t* buf, nframes_t nsamples, float current)
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

void default_mix_buffers_with_gain (audio_sample_t* dst, const audio_sample_t* src, nframes_t nframes, float gain)
{
        for (nframes_t i = 0; i < nframes; i++) {
                dst[i] += src[i] * gain;
        }
}

void default_mix_buffers_no_gain (audio_sample_t* dst, const audio_sample_t* src, nframes_t nframes)
{
        for (nframes_t i=0; i < nframes; i++) {
                dst[i] += src[i];
        }
}


#if defined (__APPLE__) && defined (BUILD_VECLIB_OPTIMIZATIONS)
#include <Accelerate/Accelerate.h>

float veclib_compute_peak (const audio_sample_t* buf, nframes_t nsamples, float current)
{
	float tmpmax = 0.0f;
	vDSP_maxmgv(buf, 1, &tmpmax, nsamples);
	return f_max(current, tmpmax);
}

void veclib_find_peaks (const audio_sample_t* buf, nframes_t nframes, float *min, float *max)
{
	vDSP_maxv (const_cast<audio_sample_t*>(buf), 1, max, nframes);
	vDSP_minv (const_cast<audio_sample_t*>(buf), 1, min, nframes);
}

void veclib_apply_gain_to_buffer (audio_sample_t * buf, nframes_t nframes, float gain)
{
	vDSP_vsmul(buf, 1, &gain, buf, 1, nframes);
}

void veclib_mix_buffers_with_gain (audio_sample_t * dst, const audio_sample_t * src, nframes_t nframes, float gain)
{
	vDSP_vsma(src, 1, &gain, dst, 1, dst, 1, nframes);
}

void veclib_mix_buffers_no_gain (audio_sample_t * dst, const audio_sample_t * src, nframes_t nframes)
{
	// It seems that a vector mult only operation does not exist...
	float gain = 1.0f;
	vDSP_vsma(src, 1, &gain, dst, 1, dst, 1, nframes);
}

#endif
