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

#ifndef ABSTRACTAUDIOWRITER_H
#define ABSTRACTAUDIOWRITER_H

#include <QObject>

#include "defines.h"

class QString;

class AbstractAudioWriter : public QObject
{
	Q_OBJECT
	
public:
	AbstractAudioWriter(const QString& filename);
	~AbstractAudioWriter();
	
	void set_num_channels(int channels);
	void set_bits_per_sample(int bits);
	void set_rate(int rate);
	nframes_t pos();
	
	bool open();
	nframes_t write(void* buffer, nframes_t frameCount);
	void close();
	
protected:
	virtual bool is_valid_format() = 0;
	virtual bool open_private() = 0;
	virtual nframes_t write_private(void* buffer, nframes_t frameCount) = 0;
	virtual void close_private() = 0;
	
	QString		m_fileName;
	bool		m_isOpen;
	int		m_sampleWidth;
	nframes_t	m_writePos;
	nframes_t	m_channels;
	nframes_t	m_rate;
};

#endif
