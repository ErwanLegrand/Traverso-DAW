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

#include "AbstractAudioReader.h"
#include "SFAudioReader.h"
#include "FlacAudioReader.h"
#include "MadAudioReader.h"
#include "WPAudioReader.h"
#include "VorbisAudioReader.h"
#include "ResampleAudioReader.h"

#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AbstractAudioReader::AbstractAudioReader(const QString& filename)
{
	m_fileName = filename;
	m_readPos = 0;
}


AbstractAudioReader::~AbstractAudioReader()
{
}


// Read cnt frames starting at start from the AudioReader, into dst
// uses seek() and read() from AudioReader subclass
nframes_t AbstractAudioReader::read_from(DecodeBuffer* buffer, nframes_t start, nframes_t count)
{
// 	printf("read_from:: before_seek from %d, framepos is %d\n", start, m_readPos);
	
	if (!seek(start)) {
		return 0;
	}
	
	return read(buffer, count);
}


int AbstractAudioReader::get_num_channels()
{
	return m_channels;
}


nframes_t AbstractAudioReader::get_length()
{
	return m_length;
}


int AbstractAudioReader::get_file_rate()
{
	return m_rate;
}


bool AbstractAudioReader::eof()
{
	return (m_readPos >= m_length);
}


nframes_t AbstractAudioReader::pos()
{
	return m_readPos;
}


bool AbstractAudioReader::seek(nframes_t start)
{
	if (m_readPos != start) {
		if (!seek_private(start)) {
			return false;
		}
		m_readPos = start;
	}
	
	return true;
}


nframes_t AbstractAudioReader::read(DecodeBuffer* buffer, nframes_t count)
{
	if (count && m_readPos < m_length) {
	// 	printf("read_from:: after_seek from %d, framepos is %d\n", start, m_readPos);
		nframes_t framesRead = read_private(buffer, count);
		
		m_readPos += framesRead;
		
		return framesRead;
	}
	
	return 0;
}


// Static method used by other classes to get an AudioReader for the correct file type
AbstractAudioReader* AbstractAudioReader::create_audio_reader(const QString& filename)
{
	AbstractAudioReader* newReader;
	
	if (FlacAudioReader::can_decode(filename)) {
		newReader = new FlacAudioReader(filename);
	}
	else if (VorbisAudioReader::can_decode(filename)) {
		newReader = new VorbisAudioReader(filename);
	}
	else if (WPAudioReader::can_decode(filename)) {
		newReader = new WPAudioReader(filename);
	}
	else if (SFAudioReader::can_decode(filename)) {
		newReader = new SFAudioReader(filename);
	}
	else if (MadAudioReader::can_decode(filename)) {
		newReader = new MadAudioReader(filename);
	}
	else {
		return 0;
	}
	
	if (newReader->get_num_channels() <= 0) {
		PERROR("new reader has 0 channels!");
		return 0;
	}

	return newReader;
}

