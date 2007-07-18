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
#include "Utils.h"
#include "AudioDevice.h"

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
	
	m_fileBuffers.resize(get_num_channels());
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
		delete m_fileBuffers.back();
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
	}
	
	reset();
	seek(0);
}


// Clear the samplerateconverter to a clean state (used on seek)
void ResampleAudioReader::reset()
{
	for (int c = 0; c < m_reader->get_num_channels(); c++) {
		src_reset(m_srcStates[c]);
	}
	
	m_srcData.end_of_input = 0;
}


// Get from child AudioReader
int ResampleAudioReader::get_num_channels()
{
	if (m_reader) {
		return m_reader->get_num_channels();
	}
	
	return 0;
}


// Get from child AudioReader, convert from file's frames to song's frames
nframes_t ResampleAudioReader::get_length()
{
	if (m_reader) {
		if (audiodevice().get_sample_rate() == (uint)m_reader->get_rate()) {
			return m_reader->get_length();
		}
		return file_to_song_frame(m_reader->get_length());
	}
	
	return 0;
}


// Always the rate of the audio device
// Remon @ Ben: why is that? imo it should just return the real rate, how else
// is anything gonna to know what the 'real' rate of an audiofile is ?
int ResampleAudioReader::get_rate()
{
	if (m_reader) {
		//return m_reader->get_rate();
		return audiodevice().get_sample_rate();
	}
	
	return 0;
}


// if no conversion is necessary, pass the seek straight to the child AudioReader,
// otherwise convert and seek
bool ResampleAudioReader::seek(nframes_t start)
{
	Q_ASSERT(m_reader);
	
	if (audiodevice().get_sample_rate() == (uint)m_reader->get_rate()) {
		return m_reader->seek(start);
	}
	
	reset();
	
	return m_reader->seek(song_to_file_frame(start));
}


// If no conversion is necessary, pass the read straight to the child AudioReader,
// otherwise get data from childreader and use libsamplerate to convert
nframes_t ResampleAudioReader::read(audio_sample_t** buffer, nframes_t frameCount)
{
	nframes_t sourceFramesRead;
	nframes_t framesRead;
	Q_ASSERT(m_reader);
	
	/////////////////////////////////
	// Ben says: FIXME: Add an overflow buffer, and grab more samples at a time, saving the extra to the overflow.
	// This may improve performance? And should fix micro-view waveform painting errors.
	/////////////////////////////////
	
	// pass through if not changing sampleRate.
	if (audiodevice().get_sample_rate() == (uint)m_reader->get_rate()) {
		sourceFramesRead = m_reader->read(buffer, frameCount);
		return sourceFramesRead;
	}
	
	nframes_t fileCnt = song_to_file_frame(frameCount);
	
	if (frameCount && !fileCnt) {
		fileCnt = 1;
	}
	
	// make sure that the reusable m_fileBuffers are big enough for this read
	if ((uint)m_fileBufferLength < fileCnt) {
		for (int c = 0; c < get_num_channels(); c++) {
			if (m_fileBuffers.size()) {
				delete m_fileBuffers[c];
			}
			m_fileBuffers[c] = new audio_sample_t[fileCnt];
		}
		m_fileBufferLength = fileCnt;
	}
	
	sourceFramesRead = m_reader->read(m_fileBuffers.data(), fileCnt);
	
	//printf("Resampler: sampleCount %lu, fileCnt %lu, returned %lu\n", sampleCount/get_num_channels(), fileCnt/get_num_channels(), samplesRead/get_num_channels());
	
	for (int c = 0; c < get_num_channels(); c++) {
		// Set up sample rate converter struct for s.r.c. processing
		m_srcData.data_in = m_fileBuffers[c];
		m_srcData.input_frames = sourceFramesRead;
		m_srcData.data_out = buffer[c];
		m_srcData.output_frames = frameCount;
		m_srcData.src_ratio = (double) audiodevice().get_sample_rate() / m_reader->get_rate();
		src_set_ratio(m_srcStates[c], m_srcData.src_ratio);
		
		if (src_process(m_srcStates[c], &m_srcData)) {
			PERROR("Resampler: src_process() error!");
			return 0;
		}
		framesRead = m_srcData.output_frames_gen;
		printf("%lu -- ", framesRead);
	}
	printf("(frames read per channel)\n");
	
	// Pad end of file with 0s if necessary
	int remainingFramesRequested = frameCount - framesRead;
	int remainingFramesInFile = get_length() - m_readPos - m_srcData.output_frames_gen;
	
	if (framesRead == 0 && remainingFramesRequested > 0 && remainingFramesInFile > 0) {
		int padLength = (remainingFramesRequested > remainingFramesInFile) ? remainingFramesInFile : remainingFramesRequested;
		for (int c = 0; c < get_num_channels(); c++) {
			memset(buffer[c] + framesRead, 0, padLength * sizeof(audio_sample_t));
		}
		framesRead += padLength;
		printf("Resampler: padding: %d\n", padLength);
	}	
	
	// Truncate so we don't return too many samples
	if (framesRead > (uint)remainingFramesInFile) {
		printf("Resampler: truncating: %d\n", framesRead - remainingFramesInFile);
		framesRead = remainingFramesInFile;
	}
	
	return framesRead;
}


nframes_t ResampleAudioReader::song_to_file_frame(nframes_t frame)
{
	Q_ASSERT(m_reader);
	
	return (nframes_t)(frame * (((double) m_reader->get_rate()) / audiodevice().get_sample_rate()));
}


nframes_t ResampleAudioReader::file_to_song_frame(nframes_t frame)
{
	Q_ASSERT(m_reader);
	
	return (nframes_t)(frame * (((double) audiodevice().get_sample_rate()) / m_reader->get_rate()));
}

