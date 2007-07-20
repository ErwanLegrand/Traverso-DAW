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

#ifndef WPAUDIOREADER_H
#define WPAUDIOREADER_H

#include <AbstractAudioReader.h>
#include "wavpack/wavpack.h"


class WPAudioReader : public AbstractAudioReader
{
public:
	WPAudioReader(QString filename);
	~WPAudioReader();

	int get_num_channels();
	nframes_t get_length();
	int get_rate();

	static bool can_decode(QString filename);

protected:
	bool seek_private(nframes_t start);
	nframes_t read_private(audio_sample_t** buffer, nframes_t frameCount);
	
	WavpackContext*	m_wp;
	bool		m_isFloat;
	int		m_bitsPerSample;
	int		m_channels;
	int32_t		*m_tmpBuffer;
	int		m_tmpBufferSize;
};

#endif
