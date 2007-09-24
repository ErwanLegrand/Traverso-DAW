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
	AbstractAudioWriter();
	~AbstractAudioWriter();
	
	virtual const char* get_extension() = 0;
	
	void set_num_channels(int channels);
	void set_bits_per_sample(int bits);
	void set_rate(int rate);
	virtual bool set_format_attribute(const QString& key, const QString& value);
	nframes_t pos();
	
	bool open(const QString& filename);
	nframes_t write(void* buffer, nframes_t frameCount);
	bool close();
	
	static AbstractAudioWriter* create_audio_writer(const QString& type);
	
protected:
	virtual bool open_private() = 0;
	virtual nframes_t write_private(void* buffer, nframes_t frameCount) = 0;
	virtual bool close_private() = 0;
	
	QString		m_fileName;
	bool		m_isOpen;
	int		m_sampleWidth;
	nframes_t	m_writePos;
	int		m_channels;
	int		m_rate;
};

#endif
