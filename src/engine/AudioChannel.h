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

$Id: AudioChannel.h,v 1.8 2008/11/24 21:11:04 r_sijrier Exp $
*/

#ifndef AUDIOCHANNEL_H
#define AUDIOCHANNEL_H

#include "defines.h"
#include <QString>
#include "Mixer.h"
#include "RingBuffer.h"
#include "APILinkedList.h"

class RingBuffer;
class AudioDevice;

class VUMonitor : public APILinkedListNode
{
public:
        VUMonitor() {
                m_peaks = new RingBuffer(150);
                m_peaks->reset();
        };
        ~VUMonitor() {
                delete m_peaks;
        };

        bool is_smaller_then(APILinkedListNode* /*node*/) {return true;}

        void write_peak(float peakValue) {
                m_peaks->write( (char*)&peakValue, 1 * sizeof(audio_sample_t));
        }

        audio_sample_t get_peak_value();

private:
        RingBuffer*     m_peaks;

};

class AudioChannel : public QObject
{
        Q_OBJECT

public:
        audio_sample_t* get_buffer(nframes_t ) {
		hasData = true;
		return buf;
	}

	void set_latency(unsigned int latency);

        void silence_buffer(nframes_t nframes) {
		memset (buf, 0, sizeof (audio_sample_t) * nframes);
	}

	void set_buffer_size(nframes_t size);
	void set_monitor_peaks(bool monitor);
        void monitor_peaks(VUMonitor* monitor=0);

        void add_monitor(VUMonitor* monitor);
        void remove_monitor(VUMonitor* monitor);

        QString get_name() const {return m_name;}
        uint get_number() const {return m_number;}
        uint get_buffer_size() const {return bufSize;}
        int get_type() const {return m_type;}

private:
        AudioChannel(const QString& name, uint channelNumber, int type);
        ~AudioChannel();

        friend class AudioDevice;

        APILinkedList          m_monitors;
        audio_sample_t* 	buf;
	uint 			bufSize;
	uint 			m_latency;
	uint 			m_number;
        int                     m_type;
        bool 			hasData;
	bool			mlocked;
	bool			monitoring;
	QString 		m_name;

	friend class JackDriver;
	friend class AlsaDriver;
	friend class PADriver;
	friend class PulseAudioDriver;
	friend class Driver;
	friend class CoreAudioDriver;

        int has_data()
        {
                return hasData || m_monitors.size();
        }

        audio_sample_t* get_data()
        {
                hasData = false;
                monitor_peaks();
                return buf;
        }

private slots:
        void private_add_monitor(VUMonitor* monitor);
        void private_remove_monitor(VUMonitor* monitor);
};

#endif

//eof
