/*
Copyright (C) 2007 Remon Sijrier 

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

#ifndef RESAMPLER_H
#define RESAMPLER_H

#include "defines.h"
#include <samplerate.h>


class Resampler : public QObject
{
public :
	Resampler();
	~Resampler();
	
	void set_output_rate(uint rate);
	void set_input_rate(uint rate);
	
	void set_input_buffer(audio_sample_t* buffer);
	void set_output_buffer(audio_sample_t* buffer);
	
	int set_converter_type(int converter_type);
	int process(nframes_t frames);
	int prepare_process(nframes_t fileCnt);
	
	void reset();
	void clear_buffers();
	
private:

	SRC_STATE*		m_srcState;
	SRC_DATA		m_srcData;
	audio_sample_t*		m_overflowBuffer;
	audio_sample_t*		m_inputBuffer;
	audio_sample_t*		m_outputBuffer;
	long			m_overflowUsed;
	uint			m_outputRate;
	uint 			m_inputRate;
	bool			m_isResampleAvailable;
	nframes_t		m_readExtraFrames;
	nframes_t 		bufferUsed;	
	nframes_t framesRead;
};



#endif
