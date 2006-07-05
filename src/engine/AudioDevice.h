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

$Id: AudioDevice.h,v 1.7 2006/07/05 11:11:15 r_sijrier Exp $
*/

#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QStringList>
#include <QByteArray>

#include "RingBuffer.h"
#include "defines.h"

#include <sys/time.h>

class AudioDeviceThread;
class Driver;
class Client;
class AudioChannel;
class AudioBus;


class AudioDevice : public QObject
{
	Q_OBJECT

public:
	AudioChannel* register_capture_channel(QByteArray busName, QString audioType, int flags, uint bufferSize, uint channel );
	AudioChannel* register_playback_channel(QByteArray busName, QString audioType, int flags, uint bufferSize, uint channel );

	void unregister_capture_channel(QByteArray name);
	void unregister_playback_channel(QByteArray name);

	void set_parameters(int rate, nframes_t bufferSize, QString driverType);

	void add_client(Client* client);
	void remove_client(Client* client);
	
	AudioChannel* get_playback_channel(QByteArray name);
	AudioChannel* get_capture_channel(QByteArray name);

	AudioBus* get_playback_bus(QByteArray name)
	{
		return playbackBuses.value(name);
	}
	AudioBus* get_capture_bus(QByteArray name)
	{
		return captureBuses.value(name);
	}

	QStringList get_capture_buses_names();
	QStringList get_playback_buses_names();

	QString get_device_name();
	QString get_device_longname();
	QString get_driver_type();

	QStringList get_available_drivers();

	uint get_sample_rate();
	uint get_bit_depth();

	QList<Client *> get_clients() const
	{
		return clients;
	}
	
	Driver* get_driver() const
	{
		return driver;
	}
	
	nframes_t get_buffer_size()
	{
		return m_bufferSize;
	}

	bool run_audio_thread() const
	{
		return runAudioThread;
	}

	void show_descriptors();

	void transport_cycle_start(trav_time_t time)
	{
		cycleStartTime = time;
	}

	void transport_cycle_end(trav_time_t time)
	{
		audio_sample_t runcycleTime = time - cycleStartTime;
		cpuTimeBuffer->write((char*)&runcycleTime, 1 * sizeof(audio_sample_t));
	}

	int shutdown();
	int run_cycle(nframes_t nframes, float delayed_usecs);
	void delay(float delay);
	uint capture_buses_count();
	uint playback_buses_count();


	trav_time_t get_cpu_time();
 

private:
	AudioDevice();
	~AudioDevice();
	AudioDevice(const AudioDevice&) : QObject()
	{}

	// allow this function to create one instance
	friend AudioDevice& audiodevice();

	friend class AlsaDriver;
	friend class JackDriver;
	friend class Driver;


	Driver* 				driver;
	AudioDeviceThread* 			audioThread;
	QList<Client *> 			clients;
	QHash<QByteArray, AudioChannel* >	playbackChannels;
	QHash<QByteArray, AudioChannel* >	captureChannels;
	QHash<QByteArray, AudioBus* >		playbackBuses;
	QHash<QByteArray, AudioBus* >		captureBuses;
	QStringList				availableDrivers;

	bool 			running;
	bool 			runAudioThread;
	RingBuffer*		cpuTimeBuffer;
	trav_time_t		cycleStartTime;
	trav_time_t		lastCpuReadTime;
	uint 			m_bufferSize;
	uint 			m_rate;
	uint			m_bitdepth;
	QString			m_driverType;
	
	int run_one_cycle(nframes_t nframes, float delayed_usecs);
	int create_driver(QString driverType);
	
	void setup_buses();
	void post_process();
	void free_memory();

	// These are reserved for Driver Objects only!!
	void set_buffer_size(uint size);
	void set_sample_rate(uint rate);
	void set_bit_depth(uint depth);

signals:
	void stopped();
	void started();
	void driverParamsChanged();
	void xrun();
	void add_client_Signal();
	void clientRemoved(Client* );
	
private slots:
	void private_add_client(Client* client);
	void private_remove_client(Client* client);
};

static inline unsigned int is_power_of_two (unsigned int n)
{
	return !(n & (n - 1));
}

static inline trav_time_t get_microseconds()
{
	struct timeval now;
	gettimeofday(&now, 0);
	trav_time_t time = (now.tv_sec * 1000000.0 + now.tv_usec);
	return time;
}


// use this function to get the audiodevice object
AudioDevice& audiodevice();

#endif

//eof
