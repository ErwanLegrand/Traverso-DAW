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
	m_fileBuffer = 0;
	m_fileBufferLength = 0;
	
	m_realReader = AbstractAudioReader::create_audio_reader(filename);
	if (!m_realReader) {
		PERROR("ResampleAudioReader: couldn't create AudioReader");
		return;
	}
	
	m_srcState = 0;
	init(converter_type);
}


ResampleAudioReader::~ResampleAudioReader()
{
	if (m_srcState) {
		src_delete(m_srcState);
	}
	
	if (m_realReader) {
		delete m_realReader;
	}
	
	if (m_fileBuffer) {
		delete m_fileBuffer;
	}
}


void ResampleAudioReader::init(int converter_type)
{
	if (m_srcState) {
		src_delete(m_srcState);
	}
	
	int error;
	m_srcState = src_new (converter_type, m_realReader->get_num_channels(), &error);
	if (!m_srcState) {
		PERROR("ResampleAudioReader: couldn't create libSampleRate SRC_STATE");
		delete m_realReader;
		m_realReader = 0;
	}
	
	reset();
	seek(0);
}


// Clear the samplerateconverter to a clean state (used on seek)
void ResampleAudioReader::reset()
{
	src_reset(m_srcState);
	m_srcData.end_of_input = 0;
}


// Get from child AudioReader
int ResampleAudioReader::get_num_channels()
{
	if (m_realReader) {
		return m_realReader->get_num_channels();
	}
	return 0;
}


// Get from child AudioReader, convert from file's frames to song's frames
nframes_t ResampleAudioReader::get_length()
{
	if (m_realReader) {
		if (audiodevice().get_sample_rate() == m_realReader->get_rate()) {
			return m_realReader->get_length();
		}
		return file_to_song_frame(m_realReader->get_length());
	}
	return 0;
}


// Always the rate of the audio device
int ResampleAudioReader::get_rate()
{
	return audiodevice().get_sample_rate();
}


// if no conversion is necessary, pass the seek straight to the child AudioReader,
// otherwise convert and seek
bool ResampleAudioReader::seek(nframes_t start)
{
	Q_ASSERT(m_realReader);
	
	if (audiodevice().get_sample_rate() == m_realReader->get_rate()) {
		return m_realReader->seek(start);
	}
	
	reset();
	
	m_nextFrame = start;
	
	return m_realReader->seek(song_to_file_frame(start));
}


// If no conversion is necessary, pass the read straight to the child AudioReader,
// otherwise get data from childreader and use libsamplerate to convert
int ResampleAudioReader::read(audio_sample_t* dst, int sampleCount)
{
	uint samplesRead;
	Q_ASSERT(m_realReader);
	
	/////////////////////////////////
	// Ben says: FIXME: Add an overflow buffer, and grab more samples at a time, saving the extra to the overflow.
	// This may improve performance? And should fix micro-view waveform painting errors.
	/////////////////////////////////
	
	// pass through if not changing sampleRate.
	if (audiodevice().get_sample_rate() == m_realReader->get_rate()) {
		samplesRead = m_realReader->read(dst, sampleCount);
		m_nextFrame += samplesRead / get_num_channels();
		return samplesRead;
	}
	
	uint fileCnt = (song_to_file_frame(sampleCount / get_num_channels())) * get_num_channels();
	
	if (sampleCount && fileCnt / get_num_channels() < 1) {
		fileCnt = 1 * get_num_channels();
	}
	
	// make sure that the reusable m_fileBuffer is big enough for this read
	if (m_fileBufferLength < fileCnt) {
		if (m_fileBuffer) {
			delete m_fileBuffer;
		}
		m_fileBuffer = new audio_sample_t[fileCnt];
		m_fileBufferLength = fileCnt;
	}
	
	samplesRead = m_realReader->read(m_fileBuffer, fileCnt);
	
	//printf("Resampler: sampleCount %lu, fileCnt %lu, returned %lu\n", sampleCount/get_num_channels(), fileCnt/get_num_channels(), samplesRead/get_num_channels());
	
	// Set up sample rate converter struct for s.r.c. processing
	m_srcData.data_in = m_fileBuffer;
	m_srcData.input_frames = samplesRead / get_num_channels();
	m_srcData.data_out = dst;
	m_srcData.output_frames = sampleCount / get_num_channels();
	m_srcData.src_ratio = (double) audiodevice().get_sample_rate() / m_realReader->get_rate();
	src_set_ratio(m_srcState, m_srcData.src_ratio);
	
	if (src_process(m_srcState, &m_srcData)) {
		PERROR("Resampler: src_process() error!");
		return 0;
	}
	
	samplesRead = m_srcData.output_frames_gen * get_num_channels();
	
	// Pad end of file with 0s if necessary
	int remainingSamplesRequested = sampleCount - samplesRead;
	int remainingSamplesInFile = (get_length() - m_nextFrame - m_srcData.output_frames_gen) * get_num_channels();
	
	if (samplesRead == 0 && remainingSamplesRequested > 0 && remainingSamplesInFile > 0) {
		int padLength = (remainingSamplesRequested > remainingSamplesInFile) ? remainingSamplesInFile : remainingSamplesRequested;
		memset(dst + samplesRead, 0, padLength * sizeof(audio_sample_t));
		samplesRead += padLength;
		printf("Resampler: padding: %d\n", padLength);
	}	
	
	// Truncate so we don't return too many samples
	if (samplesRead > remainingSamplesInFile) {
		printf("Resampler: truncating: %d\n", samplesRead - remainingSamplesInFile);
		samplesRead = remainingSamplesInFile;
	}
	
	m_nextFrame += samplesRead / get_num_channels();
	
	//printf("Resampler: req: %d, got: %d\n", sampleCount, samplesRead);
	return samplesRead;
}


nframes_t ResampleAudioReader::song_to_file_frame(nframes_t frame)
{
	Q_ASSERT(m_realReader);
	
	return (nframes_t)(frame * (((double) m_realReader->get_rate()) / audiodevice().get_sample_rate()));
}


nframes_t ResampleAudioReader::file_to_song_frame(nframes_t frame)
{
	Q_ASSERT(m_realReader);
	
	return (nframes_t)(frame * (((double) audiodevice().get_sample_rate()) / m_realReader->get_rate()));
}

