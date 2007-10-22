/*
Copyright (C) 2007 Remon Sijrier 

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
 
#include "Resampler.h"

#include "Debugger.h"

#define OVERFLOW_SIZE 512

Resampler::Resampler()
{
	m_isResampleAvailable = false;
	m_overflowBuffer = 0;
	m_inputBuffer = 0;
	m_outputBuffer = 0;
	m_overflowUsed = 0;
}

Resampler::~ Resampler()
{
	src_delete(m_srcState);
	
	if (m_overflowBuffer) {
		delete [] m_overflowBuffer;
	}
	
}

void Resampler::set_output_rate(uint rate)
{
	m_outputRate = rate;
	reset();
}

void Resampler::set_input_rate(uint rate)
{
	m_inputRate = rate;
}

void Resampler::reset()
{
	src_reset(m_srcState);
	m_srcData.end_of_input = 0;
	
	m_overflowUsed = 0;
	
	// Read extra frames from the child reader on the first read after a seek.
	// This keeps the resampler supplied with plenty of samples to produce the 
	// requested output on each read.
	m_readExtraFrames = OVERFLOW_SIZE;
}

void Resampler::clear_buffers()
{
	if (m_overflowBuffer) {
		delete [] m_overflowBuffer;
		m_overflowBuffer = 0;
	}
}

int Resampler::set_converter_type(int converter_type)
{
	int error;
	
	src_delete(m_srcState);
	
	clear_buffers();
	
	m_srcState = src_new(converter_type, 1, &error);
		
	if (!m_srcState) {
		PERROR("ResampleAudioReader: couldn't create libSampleRate SRC_STATE");
		m_isResampleAvailable = false;
		return -1;
	} else {
		m_isResampleAvailable = true;
	}
	
	return 1;
}

int Resampler::prepare_process(nframes_t fileCnt)
{
	bufferUsed = m_overflowUsed;
	
	if (m_overflowUsed) {
		// Copy pre-existing overflow into the buffer
		memcpy(m_inputBuffer, m_overflowBuffer, m_overflowUsed * sizeof(audio_sample_t));
		m_inputBuffer += m_overflowUsed;
	}
	
	return fileCnt + m_readExtraFrames - m_overflowUsed;
}

int Resampler::process(nframes_t frames)
{
		
// 	bufferUsed += m_reader->read(m_resampleDecodeBuffer, fileCnt + m_readExtraFrames - m_overflowUsed);
	
	if (m_overflowUsed) {
		m_inputBuffer -= m_overflowUsed;
	}
	//printf("Resampler: Read %lu of %lu (%lu)\n", bufferUsed, fileCnt + OVERFLOW_SIZE - m_overflowUsed, m_reader->get_length());
	
	// Don't read extra frames next time.
	m_readExtraFrames = 0;
	
/*	if (m_reader->eof()) {
		m_srcData.end_of_input = 1;
	}*/
	
	nframes_t framesToConvert = frames;
/*	if (frameCount > m_nframes - m_readPos) {
		framesToConvert = m_nframes - m_readPos;
	}*/
	
	// Set up sample rate converter struct for s.r.c. processing
	m_srcData.data_in = m_inputBuffer;
	m_srcData.input_frames = bufferUsed;
	m_srcData.data_out = m_outputBuffer;
	m_srcData.output_frames = framesToConvert;
	m_srcData.src_ratio = (double) m_outputRate / m_inputRate;
	src_set_ratio(m_srcState, m_srcData.src_ratio);
	
	if (src_process(m_srcState, &m_srcData)) {
		PERROR("Resampler: src_process() error!");
		return 0;
	}
	
	framesRead = m_srcData.output_frames_gen;
	
	m_overflowUsed = bufferUsed - m_srcData.input_frames_used;
	if (m_overflowUsed < 0) {
		m_overflowUsed = 0;
	}
	if (m_overflowUsed) {
		// If there was overflow, save it for the next read.
		memcpy(m_overflowBuffer, m_inputBuffer + m_srcData.input_frames_used, m_overflowUsed * sizeof(audio_sample_t));
	}
	
/*	// Pad end of file with 0s if necessary
	if (framesRead == 0 && m_readPos < get_nframes()) {
		int padLength = get_nframes() - m_readPos;
		printf("Resampler: padding: %d\n", padLength);
		for (int chan = 0; chan < m_channels; chan++) {
			memset(buffer->destination[chan] + framesRead, 0, padLength * sizeof(audio_sample_t));
		}
		framesRead += padLength;
	}*/
	
/*	// Truncate so we don't return too many samples
	if (m_readPos + framesRead > get_nframes()) {
		printf("Resampler: truncating: %d\n", framesRead - (get_nframes() - m_readPos));
		framesRead = get_nframes() - m_readPos;
	}*/
	
// 	printf("framesRead: %lu of %lu (overflow: %lu) (at: %lu of %lu)\n", framesRead, frameCount, m_overflowUsed, m_readPos /*+ framesRead*/, get_nframes());
	
	return framesRead;
}
