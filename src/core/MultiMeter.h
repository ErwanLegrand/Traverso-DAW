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
 
    $Id: MultiMeter.h,v 1.1 2006/11/20 16:21:38 n_doebelin Exp $
*/

#ifndef MULTIMETER_H
#define MULTIMETER_H

#include "defines.h"

class AudioBus;

static const unsigned int BUFFER_SIZE = 96000;

class MultiMeter
{

public:
	MultiMeter();
	~MultiMeter();

	void process(AudioBus *);
	float get_correlation_coefficient();
	float get_direction();

private:
	AudioBus*	m_audiobus;
	audio_sample_t*	buf_l;
	audio_sample_t*	buf_r;
	float		ringBufferR[BUFFER_SIZE];
	float		ringBufferL[BUFFER_SIZE];
	float		avg_l;
	float		avg_r;
	float		level_l;
	float		level_r;
	float		prev_avg_l;
	float		prev_avg_r;
	float		prev_r;
	float		prev_level_l;
	float		prev_level_r;
	unsigned int	index;
	unsigned int	prev_index;
	bool		hasNewData;
};

#endif

/** EOF **/
