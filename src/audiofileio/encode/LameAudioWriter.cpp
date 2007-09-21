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

#include "LameAudioWriter.h"

#include <stdio.h>
#include <lame/lame.h>

#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


// This just exists to keep lame types private and out of LameAudioWriter.h
struct LameAudioWriter::LameInfo {
	lame_global_flags *flags;
};


LameAudioWriter::LameAudioWriter()
 : AbstractAudioWriter()
{
	m_fid = 0;
	m_lameInfo = new LameInfo();
	m_lameInfo->flags = 0;
	m_bufferSize = 0;
	m_buffer = 0;
	
	// Quality settings
	m_method = 0;
	m_minBitrate = 0;
	m_maxBitrate = 0;
	m_quality = 0;
}


LameAudioWriter::~LameAudioWriter()
{
	if (m_fid) {
		close_private();
	}
	if (m_buffer) {
		delete [] m_buffer;
	}
	delete m_lameInfo;
}


const char* LameAudioWriter::get_extension()
{
	return ".mp3";
}


bool LameAudioWriter::set_format_attribute(const QString& key, const QString& value)
{

	if (key == "method") {
		if (value == "cbr") {
			m_method = 0;
			return true;
		}
		else if (value == "abr") {
			m_method = 1;
			return true;
		}
		else if (value == "vbr-old") {
			m_method = 2;
			return true;
		}
		else if (value == "vbr-new") {
			m_method = 3;
			return true;
		}
	}
	else if (key == "minBitrate") {
		m_minBitrate = value.toInt();
		return true;
	}
	else if (key == "maxBitrate") {
		m_maxBitrate = value.toInt();
		return true;
	}
	else if (key == "quality") {
		m_quality = value.toInt();
		return true;
	}
	
	return false;
}


bool LameAudioWriter::open_private()
{
	m_fid = fopen(m_fileName.toUtf8().data(), "w+");
	if (!m_fid) {
		return false;
	}
	
	m_lameInfo->flags = lame_init();
	
	if (m_lameInfo->flags == 0) {
		PERROR("lame_init failed.");
		return false;
	}
	
	lame_set_in_samplerate(m_lameInfo->flags, m_rate);
	lame_set_num_channels(m_lameInfo->flags, m_channels);
	lame_set_out_samplerate(m_lameInfo->flags, m_rate);
	

	if(m_method == 0) {
		// Constant Bitrate
		lame_set_VBR(m_lameInfo->flags, vbr_off);
		lame_set_brate(m_lameInfo->flags, m_maxBitrate);
	}
	else if (m_method == 1) {
		// Average Bitrate
		lame_set_VBR(m_lameInfo->flags, vbr_abr);
		lame_set_VBR_mean_bitrate_kbps(m_lameInfo->flags, m_maxBitrate);
	}
	else if (m_method == 2) {
		// Variable Bitrate (old)
		lame_set_VBR(m_lameInfo->flags, vbr_default);
		lame_set_VBR_min_bitrate_kbps(m_lameInfo->flags, m_minBitrate);
		lame_set_VBR_max_bitrate_kbps(m_lameInfo->flags, m_maxBitrate);
	}
	else if (m_method == 3) {
		// Variable Bitrate (new)
		lame_set_VBR(m_lameInfo->flags, vbr_default);
		lame_set_VBR_min_bitrate_kbps(m_lameInfo->flags, m_minBitrate);
		lame_set_VBR_max_bitrate_kbps(m_lameInfo->flags, m_maxBitrate);
	}

	lame_set_quality(m_lameInfo->flags, m_quality);
	
	
	//
	// file options
	//
	lame_set_copyright(m_lameInfo->flags, false);
	lame_set_original(m_lameInfo->flags, true);
	lame_set_strict_ISO(m_lameInfo->flags, false);
	lame_set_error_protection(m_lameInfo->flags, false);
	
	return (lame_init_params(m_lameInfo->flags ) != -1);
}


nframes_t LameAudioWriter::write_private(void* buffer, nframes_t frameCount)
{
	if (m_bufferSize < frameCount * 1.25 + 7200) {
		if (m_buffer) {
			delete [] m_buffer;
		}
		m_bufferSize = (long)(frameCount * 1.25 + 7200);
		m_buffer = new char[m_bufferSize];
	}
	
	int size = lame_encode_buffer_interleaved(m_lameInfo->flags,
						  (short*)buffer,
						  frameCount,
						  (unsigned char*)m_buffer,
						  m_bufferSize);
	
	if (size < 0) {
		PERROR("lame_encode_buffer_interleaved failed.");
		return 0;
	}
	if (size > 0 && fwrite(m_buffer, 1, size, m_fid) != (nframes_t)size) {
		PERROR("writing mp3 data failed.");
		return 0;
	}
	
	return frameCount;
}


void LameAudioWriter::close_private()
{
	if (m_fid)
	{
		if (m_bufferSize < 7200) {
			if (m_buffer) {
				delete [] m_buffer;
			}
			m_bufferSize = 7200;
			m_buffer = new char[m_bufferSize];
		}
		
		int size = lame_encode_flush(m_lameInfo->flags,
					     (unsigned char*)m_buffer,
					     m_bufferSize);
		if(size > 0) {
			fwrite(m_buffer, 1, size, m_fid);
		}
		
		lame_mp3_tags_fid(m_lameInfo->flags, m_fid);
		
		lame_close(m_lameInfo->flags);
		m_lameInfo->flags = 0;
		
		fclose(m_fid);
		m_fid = 0;
	}
}

