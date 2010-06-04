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

$Id: AudioBus.h,v 1.7 2007/06/04 20:47:16 r_sijrier Exp $
*/


#ifndef AUDIOBUS_H
#define AUDIOBUS_H

#include <QObject>
#include <QList>
#include <QString>
#include "defines.h"
#include "AudioChannel.h"
#include "APILinkedList.h"

class AudioBus : public QObject, public APILinkedListNode
{
	Q_OBJECT

public:
        AudioBus(const BusConfig& config);
	~AudioBus();


	void add_channel(AudioChannel* chan);
        void add_channel(const QString& channel);
        void audiodevice_params_changed();
        int get_channel_count()	{return m_channelCount;}
        QStringList get_channel_names() const;
        QString get_name() {return m_name;}
	
	AudioChannel* get_channel(int channelNumber);

	/**
	 *        Get a pointer to the buffer associated with AudioChannel \a channel 
	 * @param channel The channel number to get the buffer from
	 * @param nframes The buffer size to get
	 * @return 
	 */
        audio_sample_t* get_buffer(int channel, nframes_t nframes) {
                return m_channels.at(channel)->get_buffer(nframes);
	}

        void set_monitoring(bool monitor);
        bool is_input() {return m_type == ChannelIsInput;}
        bool is_output() {return m_type == ChannelIsOutput;}
        int get_type() const {return m_type;}
        int get_bus_type() const {return m_busType;}
        qint64 get_id() const {return m_id;}
        void set_id(qint64 id) {m_id = id;}
        void set_name(const QString& name) {m_name = name;}

        void process_monitoring() {
                for (int i=0; i<m_channels.size(); ++i) {
                        m_channels.at(i)->process_monitoring();
		}
	}

        void process_monitoring(VUMonitors vumonitors) {
                for (int i=0; i<m_channels.size(); ++i) {
                        m_channels.at(i)->process_monitoring(vumonitors.at(i));
                }
        }

        /**
	 *        Zero all AudioChannels buffers for
	 * @param nframes size of the buffer
	 */
	void silence_buffers(nframes_t nframes)
	{
                for (int i=0; i<m_channels.size(); ++i) {
                        m_channels.at(i)->silence_buffer(nframes);
		}
	}

        bool is_smaller_then(APILinkedListNode* node) {return true;}

private:
        QList<AudioChannel* >	m_channels;
        QStringList             m_channelNames;
	QString			m_name;
	
        bool			m_isMonitoring;
        bool                    m_isInternalBus;
        int 			m_channelCount;
        int                     m_type;
        int                     m_busType;
        qint64                  m_id;

signals:
	void monitoringPeaksStarted();
	void monitoringPeaksStopped();
	
};


/**
 * Get the AudioChannel associated with \a channelNumber 
 * @param channelNumber The channelNumber associated with this AudioBus's AudioChannel 
 * @return The AudioChannel on succes, 0 on failure
 */
inline AudioChannel * AudioBus::get_channel( int channelNumber )
{
        if (channelNumber < m_channelCount) {
                return m_channels.at(channelNumber);
        }
        return 0;
}


#endif

//eof
