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

#include "WPAudioReader.h"
#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


WPAudioReader::WPAudioReader(QString filename)
 : AbstractAudioReader(filename)
{
	char error[80];
	
	m_wp = WavpackOpenFileInput(m_fileName.toUtf8().data(), error, OPEN_2CH_MAX, 1);
	
	if (m_wp == 0) {
		PERROR("Couldn't open soundfile (%s) %s", filename.toUtf8().data(), error);
	}
	
	m_isFloat = ((WavpackGetMode(m_wp) & MODE_FLOAT) != 0);
	m_bitsPerSample = WavpackGetBitsPerSample(m_wp);
	m_channels = WavpackGetReducedChannels(m_wp);
	m_length = WavpackGetNumSamples(m_wp);
	m_rate = WavpackGetSampleRate(m_wp);
	
	m_tmpBuffer = 0;
	m_tmpBufferSize = 0;
}


WPAudioReader::~WPAudioReader()
{
	if (m_tmpBuffer) {
		delete m_tmpBuffer;
	}
	
	if (m_wp) {
		WavpackCloseFile(m_wp);
	}
}


bool WPAudioReader::can_decode(QString filename)
{
	char error[80];
	
	WavpackContext *wp = WavpackOpenFileInput(filename.toUtf8().data(), error, OPEN_2CH_MAX, 1);
	
	if (wp == 0) {
		return false;
	}
	
	WavpackCloseFile(wp);
	
	return true;
}


bool WPAudioReader::seek_private(nframes_t start)
{
	Q_ASSERT(m_wp);
	
	
	if (start >= m_length) {
		return false;
	}
	
	if (!WavpackSeekSample(m_wp, start)) {
		PERROR("could not seek to frame %d within %s", start, m_fileName.toUtf8().data());
		return false;
	}
	
	return true;
}


nframes_t WPAudioReader::read_private(audio_sample_t** buffer, nframes_t frameCount)
{
	Q_ASSERT(m_wp);
	
	// Make sure the temp buffer is big enough for this read
	if (m_tmpBufferSize < frameCount) {
		if (m_tmpBuffer) {
			delete m_tmpBuffer;
		}
		m_tmpBuffer = new int32_t[frameCount * m_channels];
	}
	nframes_t framesRead = WavpackUnpackSamples(m_wp, m_tmpBuffer, frameCount);
	
	// De-interlace
	if (m_isFloat) {
		switch (m_channels) {
			case 1:
				memcpy(buffer[0], m_tmpBuffer, framesRead * sizeof(audio_sample_t));
				break;	
			case 2:
				for (nframes_t f = 0; f < framesRead; f++) {
					buffer[0][f] = ((float*)m_tmpBuffer)[f * 2];
					buffer[1][f] = ((float*)m_tmpBuffer)[f * 2 + 1];
				}
				break;	
			default:
				for (nframes_t f = 0; f < framesRead; f++) {
					for (int c = 0; c < m_channels; c++) {
						buffer[c][f] = ((float*)m_tmpBuffer)[f * m_channels + c];
					}
				}
		}
	}
	else {
		switch (m_channels) {
			case 1:
				for (nframes_t f = 0; f < framesRead; f++) {
					buffer[0][f] = (float)((float)m_tmpBuffer[f]/ (float)((uint)1<<(m_bitsPerSample-1)));
				}
				break;	
			case 2:
				for (nframes_t f = 0; f < framesRead; f++) {
					buffer[0][f] = (float)((float)m_tmpBuffer[f * 2]/ (float)((uint)1<<(m_bitsPerSample-1)));
					buffer[1][f] = (float)((float)m_tmpBuffer[f * 2 + 1]/ (float)((uint)1<<(m_bitsPerSample-1)));
				}
				break;	
			default:
				for (nframes_t f = 0; f < framesRead; f++) {
					for (int c = 0; c < m_channels; c++) {
						buffer[c][f] = (float)((float)m_tmpBuffer[f + m_channels + c]/ (float)((uint)1<<(m_bitsPerSample-1)));
					}
				}
		}
	}
	
	return framesRead;
}

