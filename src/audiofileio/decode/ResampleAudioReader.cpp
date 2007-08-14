/*
Copyright (C) 2007 Ben Levitt 

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

#include "ResampleAudioReader.h"
#include <QString>

#define OVERFLOW_SIZE 1024

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


// On init, creates a child AudioReader for any filetype, and a samplerate converter
ResampleAudioReader::ResampleAudioReader(QString filename, int converter_type)
 : AbstractAudioReader(filename)
{
	m_reader = AbstractAudioReader::create_audio_reader(filename);
	if (!m_reader) {
		PERROR("ResampleAudioReader: couldn't create AudioReader");
		return;
	}
	
	m_channels = m_reader->get_num_channels();
	m_rate = m_reader->get_file_rate();
	m_length = m_reader->get_length();
	m_outputRate = m_rate;
	
	m_fileBuffers.resize(get_num_channels());
	m_filePointers.resize(get_num_channels());
	m_fileBufferLength = 0;
	
	init(converter_type);
}


ResampleAudioReader::~ResampleAudioReader()
{
	while (m_srcStates.size()) {
		src_delete(m_srcStates.back());
		m_srcStates.pop_back();
	}
	
	if (m_reader) {
		delete m_reader;
	}
	
	while (m_fileBuffers.size()) {
		delete [] m_fileBuffers.back();
		m_fileBuffers.pop_back();
	}
}


void ResampleAudioReader::init(int converter_type)
{
	int error;
	
	for (int c = 0; c < m_reader->get_num_channels(); c++) {
		m_srcStates.append(src_new(converter_type, 1, &error));
		if (!m_srcStates[c]) {
			PERROR("ResampleAudioReader: couldn't create libSampleRate SRC_STATE");
			delete m_reader;
			m_reader = 0;
			return;
		}
		m_srcStates.append(src_new(converter_type, 1, &error));
		if (!m_srcStates[c]) {
			PERROR("ResampleAudioReader: couldn't create libSampleRate SRC_STATE");
			delete m_reader;
			m_reader = 0;
			return;
		}
	}
	
	reset();
	seek_private(0);
}


// Clear the samplerateconverter to a clean state (used on seek)
void ResampleAudioReader::reset()
{
	for (int c = 0; c < m_reader->get_num_channels(); c++) {
		src_reset(m_srcStates[c]);
	}
	
	m_srcData.end_of_input = 0;
	m_overflowUsed = 0;
}


int ResampleAudioReader::get_output_rate()
{
	return m_outputRate;
}


void ResampleAudioReader::set_output_rate(int rate)
{
	if (!m_reader) {
		return;
	}
	m_outputRate = rate;
	m_length = file_to_song_frame(m_reader->get_length());
}


// if no conversion is necessary, pass the seek straight to the child AudioReader,
// otherwise convert and seek
bool ResampleAudioReader::seek_private(nframes_t start)
{
	Q_ASSERT(m_reader);
	
	if (m_outputRate == m_rate) {
		return m_reader->seek(start);
	}
	
	reset();
	
	return m_reader->seek(song_to_file_frame(start));
}


// If no conversion is necessary, pass the read straight to the child AudioReader,
// otherwise get data from childreader and use libsamplerate to convert
nframes_t ResampleAudioReader::read_private(DecodeBuffer* buffer, nframes_t frameCount)
{
	Q_ASSERT(m_reader);
	
	// pass through if not changing sampleRate.
	if (m_outputRate == m_rate) {
		return m_reader->read(buffer, frameCount);
	}
	
	nframes_t bufferUsed;
	nframes_t framesRead;
	
	nframes_t fileCnt = song_to_file_frame(frameCount);
	
	if (frameCount && !fileCnt) {
		fileCnt = 1;
	}
	
	bufferUsed = m_overflowUsed;
	
	if (!m_reader->eof()) {
		// make sure that the reusable m_fileBuffers are big enough for this read + OVERFLOW_SIZE
		if ((uint)m_fileBufferLength < fileCnt + OVERFLOW_SIZE) {
			for (int c = 0; c < m_channels; c++) {
				if (m_fileBufferLength) {
					delete [] m_fileBuffers[c];
				}
				m_fileBuffers[c] = new audio_sample_t[fileCnt + OVERFLOW_SIZE];
			}
			m_fileBufferLength = fileCnt + OVERFLOW_SIZE;
		}
		
		for (int c = 0; c < m_channels; c++) {
			m_filePointers[c] = m_fileBuffers[c] + m_overflowUsed;
		}
		
		// FIXME : this is of course very scary, needs proper fix!
		buffer->destination = m_filePointers.data(); // ????
		bufferUsed += m_reader->read(buffer, fileCnt + OVERFLOW_SIZE - m_overflowUsed);
		//printf("Resampler: Read %lu of %lu (%lu)\n", bufferUsed, fileCnt + OVERFLOW_SIZE - m_overflowUsed, m_reader->get_length());
	}
	
	if (m_reader->eof()) {
		m_srcData.end_of_input = 1;
	}
	
	nframes_t framesToConvert = frameCount;
	if (frameCount > m_length - m_readPos) {
		framesToConvert = m_length - m_readPos;
	}
	
	for (int c = 0; c < m_channels; c++) {
		// Set up sample rate converter struct for s.r.c. processing
		m_srcData.data_in = m_fileBuffers[c];
		m_srcData.input_frames = bufferUsed;
		m_srcData.data_out = buffer->destination[c];
		m_srcData.output_frames = framesToConvert;
		m_srcData.src_ratio = (double) m_outputRate / m_rate;
		src_set_ratio(m_srcStates[c], m_srcData.src_ratio);
		
		if (src_process(m_srcStates[c], &m_srcData)) {
			PERROR("Resampler: src_process() error!");
			return 0;
		}
		framesRead = m_srcData.output_frames_gen;
	}
	
	m_overflowUsed = bufferUsed - m_srcData.input_frames_used;
	
	if (m_overflowUsed < 0) {
		m_overflowUsed = 0;
	}
	
	if ((nframes_t)m_srcData.input_frames_used < bufferUsed) {
		for (int c = 0; c < m_channels; c++) {
			memmove(m_fileBuffers[c], m_fileBuffers[c] + m_srcData.input_frames_used, m_overflowUsed * sizeof(audio_sample_t));
		}
	}
	
	// Pad end of file with 0s if necessary
	if (framesRead == 0 && m_readPos < get_length()) {
		// NOTE WHOOPTYDOOOO, are you sure about this Ben ???
		int padLength = m_readPos;
		for (int c = 0; c < m_channels; c++) {
			memset(buffer->destination[c] + framesRead, 0, padLength * sizeof(audio_sample_t));
		}
		framesRead += padLength;
		printf("Resampler: padding: %d\n", padLength);
	}
	
	// Truncate so we don't return too many samples
	/*if (m_readPos + framesRead > get_length()) {
		printf("Resampler: truncating: %d\n", framesRead - (get_length() - m_readPos));
		framesRead = get_length() - m_readPos;
	}*/
	
	//printf("framesRead: %lu of %lu (overflow: %lu) (at: %lu of %lu)\n", framesRead, frameCount, m_overflowUsed, m_readPos + framesRead, get_length());
	
	return framesRead;
}


nframes_t ResampleAudioReader::song_to_file_frame(nframes_t frame)
{
	Q_ASSERT(m_reader);
	
	return (nframes_t)(frame * ((double) m_rate / m_outputRate));
}


nframes_t ResampleAudioReader::file_to_song_frame(nframes_t frame)
{
	Q_ASSERT(m_reader);
	
	return (nframes_t)(frame * ((double) m_outputRate / m_rate));
}

