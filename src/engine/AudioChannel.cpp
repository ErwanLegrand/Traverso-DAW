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

    $Id: AudioChannel.cpp,v 1.6 2006/08/25 11:13:17 r_sijrier Exp $
*/

#include "AudioChannel.h"

#ifdef USE_MLOCK
#include <sys/mman.h>
#endif /* USE_MLOCK */

#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

AudioChannel::AudioChannel( QString busName, QString audioType, int flags, uint channelNumber )
{
        m_name = busName;
        m_audioType = audioType;
        m_flags = flags;
        m_number = channelNumber;
        hasData = false;
        monitoring = false;
        buf = 0;
        bufSize = 0;
        mlocked = 0;

        peaks = new RingBuffer(150);
        peaks->reset();
}

AudioChannel::~ AudioChannel( )
{
        PENTERDES2;

#ifdef USE_MLOCK

        if (mlocked) {
                munlock (buf, bufSize);
        }
#endif /* USE_MLOCK */

        delete [] buf;
        delete peaks;

}

void AudioChannel::set_latency( uint latency )
{
        m_latency = latency;
}

void AudioChannel::set_buffer_size( nframes_t size )
{
#ifdef USE_MLOCK
        if (mlocked) {
                if (munlock (buf, bufSize) == -1) {
                	PERROR("Couldn't lock buffer into memory");
				}
                mlocked = 0;
        }
#endif /* USE_MLOCK */

        if (buf) {
                delete [] buf;
        }

        buf = new audio_sample_t[size];
        bufSize = size;
        silence_buffer(size);

#ifdef USE_MLOCK
        if (mlock (buf, size) == -1) {
        	PERROR("Couldn't lock buffer into memory");
        }
        mlocked = 1;
#endif /* USE_MLOCK */
}


audio_sample_t AudioChannel::get_peak_value( )
{
        float peak = 0;
        audio_sample_t result = 0;
        int read = peaks->read_space() /  sizeof(audio_sample_t);

        while (read != 0) {
                read = peaks->read((char*)&peak, 1 * sizeof(audio_sample_t));
                if (peak > result)
                        result = peak;
        }

        return result;
}

void AudioChannel::set_monitor_peaks( bool monitor )
{
	monitoring = monitor;
}
//eof
