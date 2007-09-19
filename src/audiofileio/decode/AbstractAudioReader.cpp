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
	m_readPos = m_channels = m_nframes = 0;
	m_length = 0;
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


int AbstractAudioReader::get_file_rate()
{
	return m_rate;
}


bool AbstractAudioReader::eof()
{
	return (m_readPos >= m_nframes);
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
	if (count && m_readPos < m_nframes) {
		
		// Make sure the read buffer is big enough for this read
		buffer->check_buffers_capacity(count, m_channels);
		
		// printf("read_from:: after_seek from %d, framepos is %d\n", start, m_readPos);
		nframes_t framesRead = read_private(buffer, count);
		
		m_readPos += framesRead;
		
		return framesRead;
	}
	
	return 0;
}


// Static method used by other classes to get an AudioReader for the correct file type
AbstractAudioReader* AbstractAudioReader::create_audio_reader(const QString& filename, const QString& decoder)
{
	AbstractAudioReader* newReader = 0;
	
	if ( ! (decoder.isEmpty() || decoder.isNull()) ) {
		if (decoder == "sndfile") {
			newReader = new SFAudioReader(filename);
		} else if (decoder == "wavpack") {
			newReader = new WPAudioReader(filename);
		} else if (decoder == "flac") {
			newReader = new FlacAudioReader(filename);
		} else if (decoder == "vorbis") {
			newReader = new VorbisAudioReader(filename);
		} else if (decoder == "mad") {
			newReader = new MadAudioReader(filename);
		}
		if (newReader && !newReader->is_valid()) {
			PERROR("new %s reader is invalid! (channels: %d, frames: %d)", (const char *)(newReader->decoder_type().toUtf8()), newReader->get_num_channels(), newReader->get_nframes());
			delete newReader;
			newReader = 0;
		}
	}
	
	if (!newReader) {
	
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
	}
	
	if (newReader && !newReader->is_valid()) {
		PERROR("new %s reader is invalid! (channels: %d, frames: %d)", (const char *)(newReader->decoder_type().toUtf8()), newReader->get_num_channels(), newReader->get_nframes());
		delete newReader;
		newReader = 0;
	}

	return newReader;
}

DecodeBuffer::DecodeBuffer()
{
	destination = 0;
	resampleBuffer = 0;
	readBuffer = 0;
	origDestination = 0;
	m_noDestBuffer = false;
	m_channels = destinationBufferSize = resampleBufferSize = readBufferSize = 0;
	m_bufferSizeCheckCounter = m_totalCheckSize = m_smallerReadCounter = 0;
		
}


void DecodeBuffer::check_buffers_capacity(uint size, uint channels)
{
/*	m_bufferSizeCheckCounter++;
	m_totalCheckSize += size;
	
	if (((m_totalCheckSize / m_bufferSizeCheckCounter) + 128) < destinationBufferSize) {
		m_smallerReadCounter++;
		if (m_smallerReadCounter > 5) {
			// Force recreation of the buffers;
			destinationBufferSize = 0;
			m_bufferSizeCheckCounter = m_smallerReadCounter = 0;
			m_totalCheckSize = 0;
		}
	}*/
				
		
	if (destinationBufferSize < size || m_channels < channels) {
			
		delete_destination_buffers();
			
		m_channels = channels;
			
		destination = new audio_sample_t*[m_channels];
			
		for (uint chan = 0; chan < m_channels; chan++) {
			destination[chan] = new audio_sample_t[size];
		}
			
		destinationBufferSize = size;
// 		printf("resizing destination to %.3f KB\n", (float)size*4/1024);
	}
		
	if (readBufferSize < (size*m_channels)) {
			
		delete_readbuffer();
			
		readBuffer = new audio_sample_t[size*m_channels];
		readBufferSize = (size*m_channels);
	}
}

void DecodeBuffer::check_resamplebuffer_capacity(uint frames)
{
		
	if (resampleBufferSize < frames) {
			
		delete_resample_buffers();
			
		resampleBuffer = new audio_sample_t*[m_channels];
			
		for (uint chan = 0; chan < m_channels; chan++) {
			resampleBuffer[chan] = new audio_sample_t[frames];
		}
			
		resampleBufferSize = frames;
// 		printf("resizing resamplebuffer to %.3f KB\n", (float)frames*4/1024);
	}
}

void DecodeBuffer::use_custom_destination_buffer(bool custom)
{
	if (custom) {
		delete_destination_buffers();
		m_noDestBuffer = true;
	}
}

void DecodeBuffer::delete_destination_buffers()
{
	if (m_noDestBuffer) {
		return;
	}
	if (destination) {
		for (uint chan = 0; chan < m_channels; chan++) {
			delete [] destination[chan];
		}
		delete [] destination;
	}
	destination = 0;
	destinationBufferSize = 0;
}

void DecodeBuffer::delete_readbuffer()
{
	if (readBuffer) {
		delete [] readBuffer;
	}
	readBuffer = 0;
	readBufferSize = 0;
}

void DecodeBuffer::delete_resample_buffers()
{
	if (resampleBuffer) {
		for (uint chan = 0; chan < m_channels; chan++) {
			if (resampleBufferSize) {
				delete [] resampleBuffer[chan];
			}
		}
		delete [] resampleBuffer;
	}
	resampleBuffer = 0;
	resampleBufferSize = 0;
}
