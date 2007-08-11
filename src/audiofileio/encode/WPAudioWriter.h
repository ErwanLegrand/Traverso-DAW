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

#ifndef WPAUDIOWRITER_H
#define WPAUDIOWRITER_H

#include "AbstractAudioWriter.h"

#include "defines.h"
#include "wavpack/wavpack.h"

class QString;

class WPAudioWriter : public AbstractAudioWriter
{
	Q_OBJECT
	
public:
	WPAudioWriter();
	~WPAudioWriter();
	
	const char* get_default_extension();
	
protected:
	bool open_private();
	nframes_t write_private(void* buffer, nframes_t frameCount);
	void close_private();
	int write_to_file(void *lpBuffer, uint32_t nNumberOfBytesToWrite, uint32_t *lpNumberOfBytesWritten);
	
	static int write_block(void *id, void *data, int32_t length);
	
	WavpackConfig	m_config;
	WavpackContext*	m_wp;
	int32_t m_bytesWritten;
	FILE*		m_file;
};

#endif
