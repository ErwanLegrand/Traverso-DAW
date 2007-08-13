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

#include "WPAudioWriter.h"

#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


WPAudioWriter::WPAudioWriter()
 : AbstractAudioWriter()
{
	m_wp = 0;
	m_firstBlock = 0;
	m_firstBlockSize = 0;
}


WPAudioWriter::~WPAudioWriter()
{
	if (m_wp) {
		close_private();
	}
	if (m_firstBlock) {
		delete m_firstBlock;
	}
}


const char* WPAudioWriter::get_extension()
{
	return ".wv";
}


bool WPAudioWriter::open_private()
{
	m_file = fopen(m_fileName.toUtf8().data(), "wb");
	if (!m_file) {
		PERROR("Couldn't open file %s.", m_fileName.toUtf8().data());
		return false;
	}
	
	m_wp = WavpackOpenFileOutput(WPAudioWriter::write_block, (void *)this, NULL);
	if (!m_wp) {
		fclose(m_file);
		return false;
	}

	memset (&m_config, 0, sizeof(m_config));
	m_config.bytes_per_sample = (m_sampleWidth == 1) ? 4 : m_sampleWidth/8;
	m_config.bits_per_sample = (m_sampleWidth == 1) ? 32 : m_sampleWidth;
	m_config.channel_mask = (m_channels == 2) ? 3 : 4; // Microsoft standard (mono = 4, stereo = 3)
	m_config.num_channels = m_channels;
	m_config.sample_rate = m_rate;
	WavpackSetConfiguration(m_wp, &m_config, -1);
	
	if (!WavpackPackInit(m_wp)) {
		fclose(m_file);
		WavpackCloseFile(m_wp);
		m_wp = 0;
		return false;
	}
	
	m_firstBlock = 0;
	m_firstBlockSize = 0;
	
	return true;
}


int WPAudioWriter::write_to_file(void *lpBuffer, uint32_t nNumberOfBytesToWrite, uint32_t *lpNumberOfBytesWritten)
{
	uint32_t bcount;
	
	*lpNumberOfBytesWritten = 0;
	
	while (nNumberOfBytesToWrite) {
		bcount = fwrite((uchar *) lpBuffer + *lpNumberOfBytesWritten, 1, nNumberOfBytesToWrite, m_file);
	
		if (bcount) {
			*lpNumberOfBytesWritten += bcount;
			nNumberOfBytesToWrite -= bcount;
		}
		else {
			break;
		}
	}
	int err = ferror(m_file);
	return !err;
}


int WPAudioWriter::write_block(void *id, void *data, int32_t length)
{
	WPAudioWriter* writer = (WPAudioWriter*) id;
	uint32_t bcount;
	
	if (writer && writer->m_file && data && length) {
		if (writer->m_firstBlock == 0) {
			writer->m_firstBlock = new char[length];
			memcpy(writer->m_firstBlock, data, length);
			writer->m_firstBlockSize = length;
		}
		if (!writer->write_to_file(data, (uint32_t)length, (uint32_t*)&bcount) || bcount != (uint32_t)length) {
			fclose(writer->m_file);
			writer->m_wp = 0;
			return false;
		}
	}

	return true;
}


bool WPAudioWriter::rewrite_first_block()
{
	if (!m_firstBlock || !m_file || !m_wp) {
		return false;
	}
	WavpackUpdateNumSamples (m_wp, m_firstBlock);
	if (fseek(m_file, 0, SEEK_SET) != 0) {
		return false;
	}
	if (!write_block(this, m_firstBlock, m_firstBlockSize)) {
		return false;
	}
	
	return true;
}


nframes_t WPAudioWriter::write_private(void* buffer, nframes_t frameCount)
{
	// FIXME:
	// Instead of this block, add an option to gdither to leave each
	// 8bit or 16bit sample in a 0-padded, int32_t
	// 
	if (m_sampleWidth > 1 && m_sampleWidth < 24) { // Not float, or 32bit int, or 24bit int
		int32_t *tmp_buffer = new int32_t[frameCount * m_channels];
		for (nframes_t s = 0; s < frameCount * m_channels; s++) {
			switch (m_sampleWidth) {
				case 8:
					tmp_buffer[s] = ((int8_t*)buffer)[s];
					break;
				case 16:
					tmp_buffer[s] = ((int16_t*)buffer)[s];
					break;
				//case 24:
				//	tmp_buffer[s] = ((int32_t*)buffer)[s]; //FIXME: does this need to read 3 bytes at a time??
				//	break;
			}
		}
		WavpackPackSamples(m_wp, tmp_buffer, frameCount);
		delete tmp_buffer;
		return frameCount;
	}
	
	WavpackPackSamples(m_wp, (int32_t *)buffer, frameCount);
	return frameCount;
}


void WPAudioWriter::close_private()
{
	WavpackFlushSamples(m_wp);
	rewrite_first_block();
	WavpackCloseFile(m_wp);
	fclose(m_file);
	m_wp = 0;
	if (m_firstBlock) {
		delete m_firstBlock;
		m_firstBlock = 0;
		m_firstBlockSize = 0;
	}
}

