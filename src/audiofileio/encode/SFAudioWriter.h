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

#ifndef SFAUDIOWRITER_H
#define SFAUDIOWRITER_H

#include "AbstractAudioWriter.h"

#include "defines.h"
#include "sndfile.h"

class QString;

class SFAudioWriter : public AbstractAudioWriter
{
	Q_OBJECT
	
public:
	SFAudioWriter(const QString& filename);
	~SFAudioWriter();
	
	bool is_valid_format();
	void set_format(int format);
	
protected:
	bool open_private();
	nframes_t write_private(void* buffer, nframes_t frameCount);
	void close_private();
	
	int		m_format;	
	SNDFILE*	m_sf;
	SF_INFO 	m_sfinfo;
};

#endif
