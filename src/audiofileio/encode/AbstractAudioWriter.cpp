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

#include "AbstractAudioWriter.h"
#include "SFAudioWriter.h"
//#include "WPAudioWriter.h"

#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AbstractAudioWriter::AbstractAudioWriter(const QString& filename)
 : QObject(0)
{
	m_fileName = filename;
	m_channels = 0;
	m_rate = 0;
	m_sampleWidth = 0;
	m_writePos = 0;
	
	m_isOpen = false;
}


AbstractAudioWriter::~AbstractAudioWriter()
{
}


void AbstractAudioWriter::set_num_channels(int channels)
{
	m_channels = channels;
}


void AbstractAudioWriter::set_bits_per_sample(int bits)
{
	m_sampleWidth = bits;
}


void AbstractAudioWriter::set_rate(int rate)
{
	m_rate = rate;
}


nframes_t AbstractAudioWriter::pos()
{
	return m_writePos;
}


bool AbstractAudioWriter::open()
{
	close();
	
	m_writePos = 0;
	
	m_isOpen = open_private();
	
	return m_isOpen;
}


void AbstractAudioWriter::close()
{
	if (m_isOpen) {
		close_private();
		m_isOpen = false;
	}
}


nframes_t AbstractAudioWriter::write(void* buffer, nframes_t count)
{
	if (count) {
		nframes_t framesWritten = write_private(buffer, count);
		
		if (framesWritten > 0) {
			m_writePos += framesWritten;
		}
		
		return framesWritten;
	}
	
	return 0;
}
