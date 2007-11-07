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

#define OVERFLOW_SIZE 512

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


// On init, creates a child AudioReader for any filetype, and a samplerate converter
ResampleAudioReader::ResampleAudioReader(QString filename, const QString& decoder)
	: AbstractAudioReader(filename)
{
	m_reader = AbstractAudioReader::create_audio_reader(filename, decoder);
	if (!m_reader) {
		PERROR("ResampleAudioReader: couldn't create AudioReader");
		m_channels = m_nframes = 0;
	} else {
		m_channels = m_reader->get_num_channels();
		m_rate = m_reader->get_file_rate();
		m_nframes = m_reader->get_nframes();
		m_length = m_reader->get_length();

		m_outputRate = m_rate;
	}
	
	m_isResampleAvailable = false;
	m_overflowBuffers = 0;
	m_overflowUsed = 0;
	m_resampleDecodeBufferIsMine = false;
	m_resampleDecodeBuffer = 0;
}


ResampleAudioReader::~ResampleAudioReader()
{
	if (m_reader) {
		delete m_reader;
	}
	
	while (m_srcStates.size()) {
		src_delete(m_srcStates.back());
		m_srcStates.pop_back();
	}
	
	if (m_overflowBuffers) {
		for (int chan = 0; chan < m_channels; chan++) {
			delete [] m_overflowBuffers[chan];
		}
		delete [] m_overflowBuffers;
	}
	
	if (m_resampleDecodeBufferIsMine) {
		delete m_resampleDecodeBuffer;
	}
}


void ResampleAudioReader::clear_buffers()
{
	if (m_overflowBuffers) {
		for (int chan = 0; chan < m_channels; chan++) {
			delete [] m_overflowBuffers[chan];
		}
		delete [] m_overflowBuffers;
		m_overflowBuffers = 0;
	}
	
	if (m_reader) {
		m_reader->clear_buffers();
	}
}

// Clear the samplerateconverter to a clean state (used on seek)
void ResampleAudioReader::reset()
{
	foreach(SRC_STATE* state, m_srcStates) {
		src_reset(state);
	}
	
	m_srcData.end_of_input = 0;
	m_overflowUsed = 0;
	
	// Read extra frames from the child reader on the first read after a seek.
	// This keeps the resampler supplied with plenty of samples to produce the 
	// requested output on each read.
	m_readExtraFrames = OVERFLOW_SIZE;
}

void ResampleAudioReader::set_converter_type(int converter_type)
{
	int error;
	
	while (m_srcStates.size()) {
		src_delete(m_srcStates.back());
		m_srcStates.pop_back();
	}
	
	clear_buffers();
	
	for (int c = 0; c < m_reader->get_num_channels(); c++) {
		
		m_srcStates.append(src_new(converter_type, 1, &error));
		
		if (!m_srcStates[c]) {
			PERROR("ResampleAudioReader: couldn't create libSampleRate SRC_STATE");
			m_isResampleAvailable = false;
			break;
		} else {
			m_isResampleAvailable = true;
		}
	}
	
	// seek_private will reset the src states!
	seek_private(0);
}

int ResampleAudioReader::get_output_rate()
{
	return m_outputRate;
}

int ResampleAudioReader::get_file_rate()
{
	return m_reader->get_file_rate();
}

void ResampleAudioReader::set_output_rate(int rate)
{
	if (!m_reader) {
		return;
	}
	m_outputRate = rate;
	m_nframes = file_to_resampled_frame(m_reader->get_nframes());
	m_length = TimeRef(m_nframes, m_outputRate);
	
	reset();
}


// if no conversion is necessary, pass the seek straight to the child AudioReader,
// otherwise convert and seek
bool ResampleAudioReader::seek_private(nframes_t start)
{
	Q_ASSERT(m_reader);
	
	if (m_outputRate == m_rate || !m_isResampleAvailable) {
		return m_reader->seek(start);
	}
	
	reset();
// 	printf("ResampleAudioReader::seek_private: start: %d\n", resampled_to_file_frame(start));
	return m_reader->seek(resampled_to_file_frame(start));
}


// If no conversion is necessary, pass the read straight to the child AudioReader,
// otherwise get data from childreader and use libsamplerate to convert
nframes_t ResampleAudioReader::read_private(DecodeBuffer* buffer, nframes_t frameCount)
{
	Q_ASSERT(m_reader);
	
	// pass through if not changing sampleRate.
	if (m_outputRate == m_rate || !m_isResampleAvailable) {
		return m_reader->read(buffer, frameCount);
	} else if (!m_overflowBuffers) {
		create_overflow_buffers();
	}
	
	nframes_t bufferUsed;
	nframes_t framesRead = 0;
	
	nframes_t fileCnt = resampled_to_file_frame(frameCount);
	
	if (frameCount && !fileCnt) {
		fileCnt = 1;
	}
	
	if (!m_resampleDecodeBuffer) {
		m_resampleDecodeBuffer = new DecodeBuffer;
		m_resampleDecodeBufferIsMine = true;
	}

	if (!m_resampleDecodeBuffer->destination) {
		reset();
	}
	
	bufferUsed = m_overflowUsed;
	
	if (m_overflowUsed) {
		// Copy pre-existing overflow into the buffer
		for (int chan = 0; chan < m_channels; chan++) {
			memcpy(m_resampleDecodeBuffer->destination[chan], m_overflowBuffers[chan], m_overflowUsed * sizeof(audio_sample_t));
		}
	}
		
	if (!m_reader->eof()) {
		if (m_overflowUsed) {
			for (int chan = 0; chan < m_channels; chan++) {
				m_resampleDecodeBuffer->destination[chan] += m_overflowUsed;
			}
		}
		
		bufferUsed += m_reader->read(m_resampleDecodeBuffer, fileCnt + m_readExtraFrames - m_overflowUsed);
		
		if (m_overflowUsed) {
			for (int chan = 0; chan < m_channels; chan++) {
				m_resampleDecodeBuffer->destination[chan] -= m_overflowUsed;
			}
		}
		//printf("Resampler: Read %lu of %lu (%lu)\n", bufferUsed, fileCnt + OVERFLOW_SIZE - m_overflowUsed, m_reader->get_length());
	}
	
	// Don't read extra frames next time.
	m_readExtraFrames = 0;
	
	if (m_reader->eof()) {
		m_srcData.end_of_input = 1;
	}
	
	nframes_t framesToConvert = frameCount;
	if (frameCount > m_nframes - m_readPos) {
		framesToConvert = m_nframes - m_readPos;
	}
	
	for (int chan = 0; chan < m_channels; chan++) {
		// Set up sample rate converter struct for s.r.c. processing
		m_srcData.data_in = m_resampleDecodeBuffer->destination[chan];
		m_srcData.input_frames = bufferUsed;
		m_srcData.data_out = buffer->destination[chan];
		m_srcData.output_frames = framesToConvert;
		m_srcData.src_ratio = (double) m_outputRate / m_rate;
		src_set_ratio(m_srcStates[chan], m_srcData.src_ratio);
		
		if (src_process(m_srcStates[chan], &m_srcData)) {
			PERROR("Resampler: src_process() error!");
			return 0;
		}
		framesRead = m_srcData.output_frames_gen;
	}
	
	m_overflowUsed = bufferUsed - m_srcData.input_frames_used;
	if (m_overflowUsed < 0) {
		m_overflowUsed = 0;
	}
	if (m_overflowUsed) {
		// If there was overflow, save it for the next read.
		for (int chan = 0; chan < m_channels; chan++) {
			memcpy(m_overflowBuffers[chan], m_resampleDecodeBuffer->destination[chan] + m_srcData.input_frames_used, m_overflowUsed * sizeof(audio_sample_t));
		}
	}
	
	// Pad end of file with 0s if necessary
	if (framesRead == 0 && m_readPos < get_nframes()) {
		int padLength = get_nframes() - m_readPos;
		printf("Resampler: padding: %d\n", padLength);
		for (int chan = 0; chan < m_channels; chan++) {
			memset(buffer->destination[chan] + framesRead, 0, padLength * sizeof(audio_sample_t));
		}
		framesRead += padLength;
	}
	
	// Truncate so we don't return too many samples
	if (m_readPos + framesRead > get_nframes()) {
		printf("Resampler: truncating: %d\n", framesRead - (get_nframes() - m_readPos));
		framesRead = get_nframes() - m_readPos;
	}
	
// 	printf("framesRead: %lu of %lu (overflow: %lu) (at: %lu of %lu)\n", framesRead, frameCount, m_overflowUsed, m_readPos /*+ framesRead*/, get_nframes());
	
	return framesRead;
}


nframes_t ResampleAudioReader::resampled_to_file_frame(nframes_t frame)
{
	TimeRef location(frame, m_outputRate);
	return location.to_frame(m_rate);
}


nframes_t ResampleAudioReader::file_to_resampled_frame(nframes_t frame)
{
	TimeRef location(frame, m_rate);
	return location.to_frame(m_outputRate);
}

void ResampleAudioReader::create_overflow_buffers()
{
	m_overflowBuffers = new audio_sample_t*[m_channels];
	for (int chan = 0; chan < m_channels; chan++) {
		m_overflowBuffers[chan] = new audio_sample_t[OVERFLOW_SIZE];
	}
}

void ResampleAudioReader::set_resample_decode_buffer(DecodeBuffer * buffer)
{
	if (m_resampleDecodeBufferIsMine && m_resampleDecodeBuffer) {
		delete m_resampleDecodeBuffer;
		m_resampleDecodeBufferIsMine = false;
	}
	m_resampleDecodeBuffer = buffer;
	reset();
}

