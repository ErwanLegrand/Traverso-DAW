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

RELAYTOOL_WAVPACK

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


WPAudioReader::WPAudioReader(QString filename)
 : AbstractAudioReader(filename)
{
	char error[80];
	
	m_wp = WavpackOpenFileInput(m_fileName.toUtf8().data(), error, OPEN_2CH_MAX | OPEN_NORMALIZE | OPEN_WVC, 0);
	
	if (m_wp == 0) {
		PERROR("Couldn't open soundfile (%s) %s", filename.toUtf8().data(), error);
		return;
	}
	
	m_isFloat = ((WavpackGetMode(m_wp) & MODE_FLOAT) != 0);
	m_bitsPerSample = WavpackGetBitsPerSample(m_wp);
	m_bytesPerSample = WavpackGetBytesPerSample(m_wp);
	m_channels = WavpackGetReducedChannels(m_wp);
	m_nframes = WavpackGetNumSamples(m_wp);
	m_rate = WavpackGetSampleRate(m_wp);
	m_length = TimeRef(m_nframes, m_rate);
}


WPAudioReader::~WPAudioReader()
{
	if (m_wp) {
		WavpackCloseFile(m_wp);
	}
}


bool WPAudioReader::can_decode(QString filename)
{
	if (!libwavpack_is_present) {
		return false;
	}
	
	char error[80];
	
	WavpackContext *wp = WavpackOpenFileInput(filename.toUtf8().data(), error, OPEN_2CH_MAX | OPEN_NORMALIZE | OPEN_WVC, 0);
	
	if (wp == 0) {
		return false;
	}
	
	WavpackCloseFile(wp);
	
	return true;
}


bool WPAudioReader::seek_private(nframes_t start)
{
	Q_ASSERT(m_wp);
	
	
	if (start >= m_nframes) {
		return false;
	}
	
	if (!WavpackSeekSample(m_wp, start)) {
		PERROR("could not seek to frame %d within %s", start, m_fileName.toUtf8().data());
		return false;
	}
	
	return true;
}


nframes_t WPAudioReader::read_private(DecodeBuffer* buffer, nframes_t frameCount)
{
	Q_ASSERT(m_wp);
	
	// WavPack only reads into a int32_t buffer...
	int32_t* readbuffer = (int32_t*)buffer->readBuffer;
	
	nframes_t framesRead = WavpackUnpackSamples(m_wp, readbuffer, frameCount);
	
	const uint divider = ((uint)1<<(m_bytesPerSample * 8 - 1));
	
	// De-interlace
	if (m_isFloat) {
		switch (m_channels) {
			case 1:
				memcpy(buffer->destination[0], readbuffer, framesRead * sizeof(audio_sample_t));
				break;	
			case 2:
				for (nframes_t f = 0; f < framesRead; f++) {
					uint pos = f*2;
					buffer->destination[0][f] = ((float*)readbuffer)[pos];
					buffer->destination[1][f] = ((float*)readbuffer)[pos + 1];
				}
				break;	
			default:
				for (nframes_t f = 0; f < framesRead; f++) {
					for (int c = 0; c < m_channels; c++) {
						buffer->destination[c][f] = ((float*)readbuffer)[f * m_channels + c];
					}
				}
		}
	}
	else {
		switch (m_channels) {
			case 1:
				for (nframes_t f = 0; f < framesRead; f++) {
					buffer->destination[0][f] = (float)((float)readbuffer[f]/ divider);
				}
				break;	
			case 2:
				for (nframes_t f = 0; f < framesRead; f++) {
					uint pos = f*2;
					buffer->destination[0][f] = (float)((float)readbuffer[pos]/ divider);
					buffer->destination[1][f] = (float)((float)readbuffer[pos + 1]/ divider);
				}
				break;	
			default:
				for (nframes_t f = 0; f < framesRead; f++) {
					for (int c = 0; c < m_channels; c++) {
						buffer->destination[c][f] = (float)((float)readbuffer[f * m_channels + c]/ divider);
					}
				}
		}
	}
	
	return framesRead;
}

