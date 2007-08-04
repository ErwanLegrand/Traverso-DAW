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

#include <QObject>

#include "defines.h"

class QString;

class AbstractAudioReader : public QObject
{
	Q_OBJECT
	
public:
	AbstractAudioReader(const QString& filename);
	~AbstractAudioReader();
	
	int get_num_channels();
	nframes_t get_length();
	int get_file_rate();
	bool eof();
	nframes_t pos();
	
	nframes_t read_from(audio_sample_t** buffer, nframes_t start, nframes_t count);
	bool seek(nframes_t start);
	nframes_t read(audio_sample_t** buffer, nframes_t frameCount);
	
	static AbstractAudioReader* create_audio_reader(const QString& filename);
	
protected:
	virtual bool seek_private(nframes_t start) = 0;
	virtual nframes_t read_private(audio_sample_t** buffer, nframes_t frameCount) = 0;
	
	QString		m_fileName;

	nframes_t	m_readPos;
	int		m_channels;
	nframes_t	m_length;
	int		m_rate;
};

#endif
