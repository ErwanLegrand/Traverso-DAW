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

#ifndef ABSTRACTAUDIOREADER_H
#define ABSTRACTAUDIOREADER_H

#include "defines.h"

#include <QString>

struct DecodeBuffer {
	
	DecodeBuffer() {
		readBuffer = 0;
		resampleBuffer = 0;
		destinationChannelCount = destinationBufferSize = resampleBufferSize = readBufferSize = 0;
	}
	
	DecodeBuffer(audio_sample_t** dest, audio_sample_t* readbuf, uint destbufsize, uint readbufsize) {
		destination = dest;
		readBuffer = readbuf;
		destinationBufferSize = destbufsize;
		readBufferSize = readbufsize;
	}
	
	void check_readbuffer_capacity(uint size) {
		if (readBufferSize < size) {
			if (readBuffer) {
				delete [] readBuffer;
			}
			readBuffer = new audio_sample_t[size];
			readBufferSize = size;
		}
	}
	
	void check_resamplebuffer_capacity(uint frames) {
		if (resampleBufferSize < frames) {
			if (!resampleBuffer) {
				resampleBuffer = new audio_sample_t*[destinationChannelCount];
			}
			for (uint chan = 0; chan < destinationChannelCount; chan++) {
				if (resampleBufferSize) {
					delete [] resampleBuffer[chan];
				}
				resampleBuffer[chan] = new audio_sample_t[frames];
			}
			resampleBufferSize = frames;
		}
	}
	
	void prepare_for_child_read(nframes_t offset) {
		if (resampleBuffer) {
			origDestination = destination;
			destination = resampleBuffer;
			
			// Let the child reader write into the buffer starting offset samples past the beginning.
			// This lets the resampler prefill the buffers with the pre-existing overflow.
			for (uint chan = 0; chan < destinationChannelCount; chan++) {
				resampleBuffer[chan] += offset;
			}
		}
	}
	
	void finish_child_read(nframes_t offset) {
		if (origDestination) {
			destination = origDestination;
			
			for (uint chan = 0; chan < destinationChannelCount; chan++) {
				resampleBuffer[chan] -= offset;
			}
		}
	}
	
	audio_sample_t** destination;
	audio_sample_t** origDestination; // Used to store destination during a child read in the resampler
	audio_sample_t* readBuffer;
	audio_sample_t** resampleBuffer;
	uint destinationChannelCount;
	uint destinationBufferSize;
	uint readBufferSize;
	uint resampleBufferSize; // ????
};

class AbstractAudioReader
{
	
public:
	AbstractAudioReader(const QString& filename);
	virtual ~AbstractAudioReader();
	
	int get_num_channels();
	nframes_t get_length();
	int get_file_rate();
	bool eof();
	nframes_t pos();
	
	nframes_t read_from(DecodeBuffer* buffer, nframes_t start, nframes_t count);
	bool seek(nframes_t start);
	nframes_t read(DecodeBuffer* buffer, nframes_t frameCount);
	
	bool is_valid() {return (m_channels > 0 && m_length > 0);}
	virtual QString decoder_type() const = 0;
	
	static AbstractAudioReader* create_audio_reader(const QString& filename, const QString& decoder = 0);
	
protected:
	virtual bool seek_private(nframes_t start) = 0;
	virtual nframes_t read_private(DecodeBuffer* buffer, nframes_t frameCount) = 0;
	
	QString		m_fileName;

	nframes_t	m_readPos;
	int		m_channels;
	nframes_t	m_length;
	int		m_rate;
};

#endif
