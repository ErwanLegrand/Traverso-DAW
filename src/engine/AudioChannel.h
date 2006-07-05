/*
Copyright (C) 2005-2006 Remon Sijrier 

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

$Id: AudioChannel.h,v 1.3 2006/07/05 11:11:15 r_sijrier Exp $
*/

#ifndef AUDIOCHANNEL_H
#define AUDIOCHANNEL_H

#include "defines.h"
#include <QString>
#include "Mixer.h"
#include "RingBuffer.h"

class RingBuffer;

class AudioChannel
{

public:
	AudioChannel(QString busName, QString audioType, int flags, uint channelNumber );
	~AudioChannel();

	audio_sample_t* get_buffer(nframes_t )
	{
		hasData = true;
		return buf;
	}

	audio_sample_t* get_data()
	{
		hasData = false;
		monitor_peaks();
		return buf;
	}

	void set_latency(unsigned int latency);

	void silence_buffer(nframes_t nframes)
	{
		memset (buf, 0, sizeof (audio_sample_t) * nframes);
	}

	void set_buffer_size(nframes_t size);
	void set_monitor_peaks(bool monitor);
	
	int has_data()
	{
		return hasData || monitoring;
	}
	
	uint get_number() const
	{
		return m_number;
	}

	void monitor_peaks()
	{
		float peakValue = 0;

		peakValue = Mixer::compute_peak( buf, bufSize, peakValue );
		peaks->write( (char*)&peakValue, 1 * sizeof(audio_sample_t));
	}

	audio_sample_t get_peak_value();

	QString get_name()
	{
		return m_name;
	}

private:
	audio_sample_t* 	buf;
	RingBuffer*		peaks;
	uint 			bufSize;
	uint 			m_latency;
	uint 			m_number;
	int 			m_flags;
	bool 			hasData;
	bool			mlocked;
	bool			monitoring;
	QString 		m_name;
	QString 		m_audioType;
};

#endif

//eof
