/*
Copyright (C) 2006 Remon Sijrier

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

$Id: WriteSource.h,v 1.2 2006/08/25 11:24:53 r_sijrier Exp $
*/

#ifndef WRITESOURCE_H
#define WRITESOURCE_H

#include "AudioSource.h"

#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
#include <samplerate.h>
#endif
#include "gdither.h"

struct ExportSpecification;

class WriteSource : public AudioSource
{
	Q_OBJECT

public :
	WriteSource(ExportSpecification* spec);
	WriteSource(ExportSpecification* spec, int channelNumber);
	~WriteSource();

	int rb_write(const audio_sample_t* src, nframes_t start, nframes_t cnt);
	int rb_file_write(nframes_t cnt);
	int process_ringbuffer(audio_sample_t* framebuffer);

	int process(nframes_t nframes);

	int prepare_export(ExportSpecification* spec);
	int finish_export();

	void set_process_peaks(bool process);
	void set_recording(bool rec);

	bool is_recording();

	ExportSpecification*		spec;

private:
	GDither             	dither;
	nframes_t      		out_samples_max;
	nframes_t      		sample_rate;
	uint				channels;
	uint32_t       		sample_bytes;
	nframes_t      		leftover_frames;
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
	SRC_DATA        	src_data;
	SRC_STATE*     	src_state;
#endif
	nframes_t      		max_leftover_frames;
	float*			leftoverF;
	float*			dataF2;
	void*               		output_data;
	bool				processPeaks;
	bool				recording;

signals:
	void exportFinished(WriteSource* );
};

#endif




