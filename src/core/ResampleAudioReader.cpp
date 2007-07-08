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
ResampleAudioReader::ResampleAudioReader(QString filename)
 : AbstractAudioReader(filename)
{
	m_fileBuffer = 0;
	m_fileBufferLength = 0;
	
	m_realReader = AbstractAudioReader::create_audio_reader(filename);
	if (!m_realReader) {
		PERROR("ResampleAudioReader: couldn't create AudioReader");
		return;
	}
	
	int error;
	m_srcState = src_new (SRC_SINC_MEDIUM_QUALITY, m_realReader->get_num_channels(), &error);
	if (!m_srcState) {
		PERROR("ResampleAudioReader: couldn't create libSampleRate SRC_STATE");
		delete m_realReader;
		m_realReader = 0;
	}
	
	reset();
	seek(0);
}


ResampleAudioReader::~ResampleAudioReader()
{
	if (m_realReader) {
		src_delete(m_srcState);
		delete m_realReader;
	}

	if (m_fileBuffer) {
		delete m_fileBuffer;
	}
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


// Still not sure if this is going to be necessary...
bool ResampleAudioReader::is_compressed()
{
	if (m_realReader) {
		return m_realReader->is_compressed();
	}
	return false;
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
	Q_ASSERT(m_realReader);

	if (audiodevice().get_sample_rate() == m_realReader->get_rate()) {
		return m_realReader->read(dst, sampleCount);
	}
	
	// The +1 means decode a tiny bit extra from the file to make sure we can get enough resampled data
	nframes_t fileCnt = (song_to_file_frame(sampleCount / get_num_channels()) +1) * get_num_channels();
	
	// make sure that the reusable m_fileBuffer is big enough for this read
	if (m_fileBufferLength < fileCnt) {
		if (m_fileBuffer) {
			delete m_fileBuffer;
		}
		m_fileBuffer = new audio_sample_t[fileCnt];
		m_fileBufferLength = fileCnt;
	}
	
	int samplesRead;
	samplesRead = m_realReader->read(m_fileBuffer, fileCnt);
	
	if (samplesRead == fileCnt) {
		m_nextFrame += sampleCount / get_num_channels();
	}
	else {
		m_nextFrame += file_to_song_frame(samplesRead) / get_num_channels();
	}
	
	m_srcData.data_in = m_fileBuffer;
	m_srcData.input_frames = samplesRead / get_num_channels();
	m_srcData.data_out = dst;
	m_srcData.output_frames = sampleCount / get_num_channels();
	m_srcData.src_ratio = (double) audiodevice().get_sample_rate() / m_realReader->get_rate();
	src_set_ratio(m_srcState, m_srcData.src_ratio);
	
	if (src_process(m_srcState, &m_srcData)) {
		PERROR("src_process() error!");
		return 0;
	}
	
	return m_srcData.output_frames_gen * get_num_channels();
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

