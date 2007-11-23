/*
Copyright (C) 2006-2007 Remon Sijrier

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

#ifndef WRITESOURCE_H
#define WRITESOURCE_H

#include "AudioSource.h"

#include "gdither.h"
#include <samplerate.h>

struct ExportSpecification;
class Peak;
class DiskIO;
class AbstractAudioWriter;

/// WriteSource is an AudioSource used for writing (recording, rendering) purposes
class WriteSource : public AudioSource
{
	Q_OBJECT

public :
	WriteSource(ExportSpecification* spec);
	~WriteSource();

	int rb_write(audio_sample_t** src, nframes_t cnt);
	int rb_file_write(nframes_t cnt);
	void process_ringbuffer(audio_sample_t* buffer);
	int get_processable_buffer_space() const;
	int get_chunck_size() const {return m_chunkSize;}
	int get_buffer_size() const {return m_bufferSize;}
	Peak* get_peak() {return m_peak;}

	int process(nframes_t nframes);
	
	int prepare_export();
	int finish_export();
	void set_process_peaks(bool process);
	void set_recording(int rec);

	size_t is_recording() const;

	void set_diskio(DiskIO* io );

private:
	AbstractAudioWriter*	m_writer;
	ExportSpecification*	m_spec;
	Peak*			m_peak;
	
	DiskIO*		m_diskio;
	GDither         m_dither;
	bool		m_processPeaks;
	size_t		m_isRecording;
	nframes_t       m_sampleRate;
	uint32_t        m_sample_bytes;
	
	// Sample rate conversion variables
	nframes_t       m_out_samples_max;
	nframes_t       m_leftover_frames;
	SRC_DATA        m_src_data;
	SRC_STATE*      m_src_state;
	nframes_t       m_max_leftover_frames;
	float*		m_leftoverF;
	float*		m_dataF2;
	void*           m_output_data;
	
	
	void prepare_rt_buffers();
	
signals:
	void exportFinished();
};


inline int WriteSource::get_processable_buffer_space( ) const
{
	return m_buffers.at(0)->read_space();
}

inline size_t WriteSource::is_recording( ) const
{
	return m_isRecording;
}

#endif




