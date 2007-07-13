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

#ifndef MADAUDIOREADER_H
#define MADAUDIOREADER_H

#include <AbstractAudioReader.h>

extern "C" {
#include <mad.h>
}


class MadAudioReader : public AbstractAudioReader
{
public:
	MadAudioReader(QString filename);
	~MadAudioReader();

	int get_num_channels();
	nframes_t get_length();
	int get_rate();
	bool seek(nframes_t start);
	int read(audio_sample_t* dst, int sampleCount);

	static bool can_decode(QString filename);

protected:
	bool initDecoderInternal();
	unsigned long countFrames();
	bool createPcmSamples(mad_synth* synth);

	static int	MaxAllowedRecoverableErrors;
	nframes_t	m_frames;
	int		m_channels;

	class MadDecoderPrivate;
	MadDecoderPrivate* d;
};

#endif
