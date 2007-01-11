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

$Id: WriteSource.h,v 1.9 2007/01/11 14:57:37 r_sijrier Exp $
*/

#ifndef WRITESOURCE_H
#define WRITESOURCE_H

#include "AudioSource.h"
#include "RingBufferNPT.h"
#include <sndfile.h>
#include "gdither.h"


#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
#include <samplerate.h>
#endif

struct ExportSpecification;
class Peak;
class DiskIO;

/// WriteSource is an AudioSource only used for writing (recording, rendering) purposes
class WriteSource : public AudioSource
{
	Q_OBJECT

public :
	WriteSource(ExportSpecification* spec);
	WriteSource(ExportSpecification* spec, int channelNumber, int superChannelCount);
	~WriteSource();

	int rb_write(audio_sample_t* src, nframes_t cnt);
	int rb_file_write(nframes_t cnt);
	void process_ringbuffer(audio_sample_t* framebuffer, bool seek);
	int get_processable_buffer_space() const;

	int process(nframes_t nframes);

	int prepare_export(ExportSpecification* spec);
	int finish_export();

	void set_process_peaks(bool process);
	void set_recording(int rec);
	void prepare_buffer();

	size_t is_recording() const;

	ExportSpecification*		spec;
	void set_diskio(DiskIO* io );

private:
	RingBufferNPT<audio_sample_t>*	m_buffer;
	Peak* 		m_peak;
	SNDFILE*	sf;
	SF_INFO 	sfinfo;
	DiskIO*		diskio;
	
	GDither         dither;
	nframes_t       out_samples_max;
	nframes_t       sample_rate;
	uint		channels;
	uint32_t        sample_bytes;
	nframes_t       leftover_frames;
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
	SRC_DATA        src_data;
	SRC_STATE*      src_state;
#endif
	nframes_t       max_leftover_frames;
	float*		leftoverF;
	float*		dataF2;
	void*           output_data;
	bool		processPeaks;
	size_t		m_isRecording;
	int		m_channelNumber;

signals:
	void exportFinished(WriteSource* );
};


inline int WriteSource::get_processable_buffer_space( ) const
{
	return m_buffer->read_space();
}

inline size_t WriteSource::is_recording( ) const
{
	return m_isRecording;
}

#endif




