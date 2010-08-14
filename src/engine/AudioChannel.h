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
                m_flag = 0;
        }
        ~VUMonitor() {}

        bool is_smaller_then(APILinkedListNode* /*node*/) {return true;}

        void process(float peakValue) {
                if (m_flag) {
                        m_peak = 0.0f;
                        m_flag = 0;
                }

                if (peakValue > m_peak) {
                        m_peak = peakValue;
                }
        }

        audio_sample_t get_peak_value();
        inline void set_read() {m_flag = 1;}

private:
        int    m_flag;
        audio_sample_t m_peak;
};

class AudioChannel : public QObject
{
        Q_OBJECT

public:
        AudioChannel(const QString& name, uint channelNumber, int type, qint64 id=0);
        ~AudioChannel();

        audio_sample_t* get_buffer(nframes_t ) {
                return m_buffer;
	}

	void set_latency(unsigned int latency);

        void silence_buffer(nframes_t nframes) {
                memset (m_buffer, 0, sizeof (audio_sample_t) * nframes);
	}

	void set_buffer_size(nframes_t size);
        void set_monitoring(bool monitor);
        void process_monitoring(VUMonitor* monitor=0);

        void add_monitor(VUMonitor* monitor);
        void remove_monitor(VUMonitor* monitor);

        QString get_name() const {return m_name;}
        uint get_number() const {return m_number;}
        uint get_buffer_size() const {return m_bufferSize;}
        int get_type() const {return m_type;}
        qint64 get_id() const {return m_id;}

private:
        APILinkedList           m_monitors;
        audio_sample_t* 	m_buffer;
        uint 			m_bufferSize;
	uint 			m_latency;
	uint 			m_number;
        qint64                  m_id;
        int                     m_type;
	bool			mlocked;
        bool			m_monitoring;
	QString 		m_name;

	friend class JackDriver;
	friend class AlsaDriver;
	friend class PADriver;
	friend class PulseAudioDriver;
        friend class TAudioDriver;
	friend class CoreAudioDriver;

        void read_from_hardware_port(audio_sample_t* buf, nframes_t nframes);

private slots:
        void private_add_monitor(VUMonitor* monitor);
        void private_remove_monitor(VUMonitor* monitor);
};

#endif

//eof
