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

#ifndef RESAMPLEAUDIOREADER_H
#define RESAMPLEAUDIOREADER_H

#include <AbstractAudioReader.h>
#include <QVector>
#include <samplerate.h>


class ResampleAudioReader : public AbstractAudioReader
{

public:
	ResampleAudioReader(QString filename, int converter_type);
	~ResampleAudioReader();
	
	int get_num_channels();
	nframes_t get_length();
	int get_rate();
	bool seek(nframes_t start);
	nframes_t read(audio_sample_t** buffer, nframes_t frameCount);
	
protected:
	void init(int converter_type);
	void reset();
	
	nframes_t song_to_file_frame(nframes_t frame);
	nframes_t file_to_song_frame(nframes_t frame);
	
	bool			m_valid;
	AbstractAudioReader*	m_reader;
	QVector<SRC_STATE*>	m_srcStates;
	SRC_DATA		m_srcData;
	QVector<audio_sample_t*> m_fileBuffers;
	long			m_fileBufferLength;
};

#endif
