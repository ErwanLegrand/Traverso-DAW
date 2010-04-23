/*
    Copyright (C) 2005-2010 Remon Sijrier

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

#include "AudioChannel.h"

#include "Tsar.h"

#ifdef USE_MLOCK
#include <sys/mman.h>
#endif /* USE_MLOCK */

#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
 * \class AudioChannel
 * \brief AudioChannel wraps one 'real' audiochannel into an easy to use class
 *
 * An AudioChannel has a audio_sample_t* buffer, a name and some functions for setting the buffer size, 
 * and monitoring the highest peak value, which is handy to use by for example a VU meter. 
 */


AudioChannel::AudioChannel(const QString& name, uint channelNumber, int type)
{
        m_name = name;
        m_number = channelNumber;
        m_type = type;
        hasData = false;
        monitoring = false;
        buf = 0;
        bufSize = 0;
        mlocked = 0;
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


void AudioChannel::monitor_peaks(VUMonitor* monitor)
{
                Q_ASSERT(bufSize > 0);
                float peakValue = 0;
                peakValue = Mixer::compute_peak( buf, bufSize, peakValue );

                if (monitor) {
                        monitor->write_peak(peakValue);
                }

                apill_foreach(VUMonitor* internalmonitor, VUMonitor, m_monitors) {
                        internalmonitor->write_peak(peakValue);
                }
}

void AudioChannel::set_monitor_peaks( bool monitor )
{
	monitoring = monitor;
}


void AudioChannel::private_add_monitor(VUMonitor *monitor)
{
        m_monitors.append(monitor);
}

void AudioChannel::private_remove_monitor(VUMonitor *monitor)
{
        if (!m_monitors.remove(monitor)) {
                printf("AudioChannel:: VUMonitor was not in monitors list, failed to remove it!\n");
        }
}

void AudioChannel::add_monitor(VUMonitor *monitor)
{
        THREAD_SAVE_INVOKE(this, monitor, private_add_monitor(VUMonitor*));
}

void AudioChannel::remove_monitor(VUMonitor *monitor)
{
        THREAD_SAVE_INVOKE(this, monitor, private_remove_monitor(VUMonitor*));
}


/**
 *
 * @return The highest peak value since the previous call to this function,
 *		 call this at least 10 times each second to keep data consistent
 */
audio_sample_t VUMonitor::get_peak_value( )
{
        float peak = 0;
        audio_sample_t result = 0;
        int read = m_peaks->read_space() /  sizeof(audio_sample_t);

        while (read != 0) {
                read = m_peaks->read((char*)&peak, 1 * sizeof(audio_sample_t));
                if (peak > result)
                        result = peak;
        }

        return result;
}

//eof
