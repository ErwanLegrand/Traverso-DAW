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

#include "PeakDataReader.h"
#include <QFile>
#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


PeakDataReader::PeakDataReader(QString filename)
	: AbstractAudioReader(filename)
{
 
	m_file = fopen(m_fileName.toUtf8().data(),"rb");
	
	if (! m_file) {
		PERROR("Couldn't open peak file for reading! (%s)", m_fileName.toUtf8().data());
		m_length = m_channels = 0;
		return;
	}
	
	fseek (m_file, 0, SEEK_END);
	m_channels = 1;
	m_length = ftell (m_file);
	m_rate = 32000;
	
}


PeakDataReader::~PeakDataReader()
{
	if (m_file) {
		fclose(m_file);
	}
}


bool PeakDataReader::can_decode(QString filename)
{
	if (!filename.contains(".peak")) return false;
	return true;
}


bool PeakDataReader::seek_private(nframes_t start)
{
	Q_ASSERT(m_file);
	
	
	if (start >= m_length) {
		return false;
	}
	
	if (fseek (m_file, start, SEEK_SET) < 0) {
		PERROR("PeakDataReader: could not seek to data point %d within %s", start, m_fileName.toUtf8().data());
		return false;
	}
	
	return true;
} 


nframes_t PeakDataReader::read_private(DecodeBuffer* buffer, nframes_t frameCount)
{
	Q_ASSERT(m_file);
	
	int framesRead = fread(buffer->readBuffer, sizeof(peak_data_t), frameCount, m_file);
	
	peak_data_t* readbuffer = (peak_data_t*)buffer->readBuffer;
	
	// De-interlace
	switch (m_channels) {
		case 1:
			for (int f = 0; f < framesRead; f++) {
				buffer->destination[0][f] = (float)readbuffer[f];
			}
			break;	
		case 2:
			for (int f = 0; f < framesRead; f++) {
				int pos = f*2;
				buffer->destination[0][f] = (float)readbuffer[pos];
				buffer->destination[1][f] = (float)readbuffer[pos + 1];
			}
			break;	
		default:
			for (int f = 0; f < framesRead; f++) {
				for (int c = 0; c < m_channels; c++) {
					buffer->destination[c][f] = (float)readbuffer[f * m_channels + c];
				}
			}
	}
	
	return framesRead;
}

