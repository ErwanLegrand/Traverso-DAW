/*
    Copyright (C) 2005-2006 Nicola Doebelin

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

    $Id: MultiMeter.cpp,v 1.1 2006/11/20 16:21:38 n_doebelin Exp $
*/

#include "MultiMeter.h"
#include "AudioBus.h"

#include <QDebug>
#include <math.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MultiMeter::MultiMeter()
{
	for (unsigned int i = 0; i < BUFFER_SIZE; ++i) {
		ringBufferR[i] = 0.0;
		ringBufferL[i] = 0.0;
	}

	avg_l = 0.0;
	avg_r = 0.0;
	index = 0;
	prev_avg_l = 0.0;
	prev_avg_r = 0.0;
	prev_r = 1.0;

	level_l = 0.0;
	level_r = 0.0;
	prev_level_l = 0.0;
	prev_level_r = 0.0;
	prev_index = 0;
	hasNewData = false;
}

MultiMeter::~MultiMeter()
{
}


void MultiMeter::process(AudioBus *a_bus)
{
	if (a_bus) {
		m_audiobus = a_bus;
	} else {
		return;
	}

	// check if audiobus is stereo (2 channels)
	// if not, do nothing
	if (m_audiobus->get_channel_count() != 2) return;

	uint buf_size = m_audiobus->get_channel(0)->get_buffer_size();

	buf_l = m_audiobus->get_buffer(0, buf_size);
	buf_r = m_audiobus->get_buffer(1, buf_size);

	for (uint i = 0; i < buf_size; ++i) {
		ringBufferL[index] = buf_l[i];
		ringBufferR[index] = buf_r[i];

		++index;
		if (index >= BUFFER_SIZE) {
			index = 0;
		}
	}

	hasNewData = true;
}

/**
 * Compute the correlation coefficient of the stereo master output data
 * of the active song. The number of samples considered in the calculation
 * is fixed.
 *
 * @returns linear correlation coefficient (1.0: complete positive correlation,
 *  0.0: uncorrelated, -1.0: complete negative correlation).
 **/
float MultiMeter::get_correlation_coefficient()
{
//  All prev_* variables are used to avoid iterating over the entire ring
//  buffer each time. The idea is to store the previous values, and only iterate
//  over the new data in the ring buffer. The results are merged afterwards.
//  This technique reduced CPU usage from 47% to 14% on my system!
	if (!hasNewData) {
		return 1.0;
	}

	float r = 1.0;
	float a1 = 0.0;
	float a2 = 0.0;
	float aa = 0.0;
	float a1sq = 0.0;
	float a2sq = 0.0;

	unsigned int num_samples = index - prev_index;
	if (index < prev_index) {
		num_samples = index + (BUFFER_SIZE - prev_index);
	}

	float fract = (float)num_samples / (float)BUFFER_SIZE;

	float vl;
	float vr;

	// calulate averages
	unsigned int j = prev_index;
	for (unsigned int i = 0; i < num_samples; ++i) {
		if (j >= BUFFER_SIZE) {
			j = 0;
		}
		vl = ringBufferL[j];
		vr = ringBufferR[j];

		avg_l += vl;
		avg_r += vr;

		level_l += fabs(vl);
		level_r += fabs(vr);

		++j;
	}

	avg_l /= (float)num_samples;
	avg_r /= (float)num_samples;

	level_l /= (float)num_samples;
	level_r /= (float)num_samples;

	avg_l = avg_l * fract + prev_avg_l * (1.0 - fract);
	avg_r = avg_r * fract + prev_avg_r * (1.0 - fract);

	level_l = level_l * fract + prev_level_l * (1.0 - fract);
	level_r = level_r * fract + prev_level_r * (1.0 - fract);

	// check for dividions by 0
	if ((!avg_l) || (!avg_r)) {
		return 1.0;
	}

	// calculate coefficient
	j = prev_index;
	for (unsigned int i = 0; i < num_samples; ++i) {
		if (j >= BUFFER_SIZE) {
			j = 0;
		}
		a1 = ringBufferL[j] - avg_l;
		a2 = ringBufferR[j] - avg_r;

		aa += a1 * a2;
		a1sq += a1 * a1;
		a2sq += a2 * a2;
	}

	r = aa / (sqrtf(a1sq) * sqrtf(a2sq));

	r = r * fract + prev_r * (1.0 - fract);

	// store the current values
	prev_index = index;
	prev_avg_l = avg_l;
	prev_avg_r = avg_r;
	prev_r = r;
	prev_level_l = level_l;
	prev_level_r = level_r;

	hasNewData = false;

	return r;
}

float MultiMeter::get_direction()
{
	if ((prev_level_l == 0.0) && (prev_level_r == 0.0)) {
		return 0.0;
	}

	float vl = prev_level_l / (prev_level_l + prev_level_r);
	float vr = prev_level_r / (prev_level_l + prev_level_r);

	return vr - vl;
}

// EOF
