/*
Copyright (C) 2005-2007 Remon Sijrier 

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


#include "AudioSource.h"
#include "Sheet.h"
#include "Peak.h"
#include "Export.h"
#include "Utils.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

// QMutex mutex;


// This constructor is called at file import or recording
AudioSource::AudioSource(const QString& dir, const QString& name)
	: m_dir(dir)
	, m_name(name)
	, m_shortName(name)
	, m_wasRecording (false)
{
	PENTERCONS;
	m_fileName = m_dir + m_name;
	m_id = create_id();
}


// This constructor is called for existing (recorded/imported) audio sources
AudioSource::AudioSource()
	: m_dir("")
	, m_name("")
	, m_fileName("")
	, m_wasRecording(false)
{
}


AudioSource::~AudioSource()
{
	PENTERDES;
}

void AudioSource::set_name(const QString& name)
{
	m_name = name;
	if (m_wasRecording) {
		m_shortName = m_name.left(m_name.length() - 20);
	} else {
		m_shortName = m_name;
	}
	m_fileName = m_dir + m_name;
}


void AudioSource::set_dir(const QString& dir)
{
	m_dir = dir;
	m_fileName = m_dir + m_name;
}


int AudioSource::get_rate( ) const
{
	return m_rate;
}

void AudioSource::set_original_bit_depth( uint bitDepth )
{
	m_origBitDepth = bitDepth;
}

void AudioSource::set_created_by_sheet(qint64 id)
{
	m_origSheetId = id;
}

QString AudioSource::get_filename( ) const
{
	return m_fileName;
}

QString AudioSource::get_dir( ) const
{
	return m_dir;
}

QString AudioSource::get_name( ) const
{
	return m_name;
}

int AudioSource::get_bit_depth( ) const
{
	return m_origBitDepth;
}

QString AudioSource::get_short_name() const
{
	return m_shortName;
}

// eof
