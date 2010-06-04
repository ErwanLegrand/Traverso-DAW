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

#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QObject>
#include <QList>
#include <QVector>
#include <QHash>
#include <QStringList>
#include <QByteArray>
#include <QTimer>
#include <QVariant>


#include "RingBufferNPT.h"
#include "APILinkedList.h"
#include "defines.h"

class AudioDeviceThread;
class Driver;
class AudioDeviceClient;
class AudioChannel;
class AudioBus;
#if defined (JACK_SUPPORT)
class JackDriver;
#endif

#if defined (COREAUDIO_SUPPORT)
class CoreAudioDriver;
#endif


class AudioDevice : public QObject
{
	Q_OBJECT

public:
        void set_parameters(AudioDeviceSetup ads);

	void add_client(AudioDeviceClient* client);
	void remove_client(AudioDeviceClient* client);
	
	void transport_start(AudioDeviceClient* client);
        void transport_stop(AudioDeviceClient* client, TimeRef location);
	int transport_seek_to(AudioDeviceClient* client, TimeRef location);

        AudioDeviceSetup get_device_setup() {return m_setup;}

        AudioChannel* create_channel(const QString& name, int channelNumber, int type);
        AudioChannel* get_playback_channel_by_name(const QString& name);
        AudioChannel* get_capture_channel_by_name(const QString& name);

        void delete_channel(AudioChannel* channel);

        void set_master_out_bus(AudioBus* bus);
        void send_to_master_out(AudioChannel* channel, nframes_t nframes);

	QStringList get_capture_channel_names() const;
	QStringList get_playback_channel_names() const;
	
        QList<AudioChannel*> get_channels() const;
        QList<AudioChannel*> get_playback_channels() const;
        QList<AudioChannel*> get_capture_channels() const;
        int add_jack_channel(AudioChannel* channel);
	
	QString get_device_name() const;
	QString get_device_longname() const;
	QString get_driver_type() const;

	QStringList get_available_drivers() const;

	uint get_sample_rate() const;
	uint get_bit_depth() const;
	TimeRef get_buffer_latency();

	/**
	 * 
	 * @return The period buffer size, as used by the Audio Driver.
	 */
	nframes_t get_buffer_size() const
	{
		return m_bufferSize;
	}


	void show_descriptors();
	void set_driver_properties(QHash<QString, QVariant>& properties);

	int shutdown();
	
	trav_time_t get_cpu_time();


private:
	AudioDevice();
	~AudioDevice();
	AudioDevice(const AudioDevice&) : QObject() {}

	// allow this function to create one instance
	friend AudioDevice& audiodevice();

	friend class AlsaDriver;
	friend class PADriver;
	friend class Driver;
	friend class PulseAudioDriver;
	friend class AudioDeviceThread;
#if defined (COREAUDIO_SUPPORT)
	friend class CoreAudioDriver;
#endif

        AudioDeviceSetup        m_setup;
        AudioDeviceSetup        m_fallBackSetup;
        AudioBus*               m_masterOutBus;
        Driver* 		m_driver;
        AudioDeviceThread* 	m_audioThread;
        APILinkedList		m_clients;
        QList<AudioChannel* >   m_channels;
        QList<BusConfig>        m_busConfigs;
        QList<ChannelConfig>    m_channelConfigs;
        QStringList		m_availableDrivers;
        QTimer			m_xrunResetTimer;
#if defined (JACK_SUPPORT)
        QTimer			jackShutDownChecker;
	JackDriver* slaved_jack_driver();
	friend class JackDriver;
#endif

	RingBufferNPT<trav_time_t>*	m_cpuTime;
	volatile size_t		m_runAudioThread;
	trav_time_t		m_cycleStartTime;
	trav_time_t		m_lastCpuReadTime;
	uint 			m_bufferSize;
	uint 			m_rate;
	uint			m_bitdepth;
	uint			m_xrunCount;
	QString			m_driverType;
	QString			m_ditherShape;
	QHash<QString, QVariant> m_driverProperties;

	int run_one_cycle(nframes_t nframes, float delayed_usecs);
	int create_driver(QString driverType, bool capture, bool playback, const QString& cardDevice);
	int transport_control(transport_state_t state);

	void post_process();

	// These are reserved for Driver Objects only!!
	AudioChannel* register_capture_channel(const QByteArray& busName, const QString& audioType, int flags, uint bufferSize, uint channel );
	AudioChannel* register_playback_channel(const QByteArray& busName, const QString& audioType, int flags, uint bufferSize, uint channel );
	
	int run_cycle(nframes_t nframes, float delayed_usecs);
	
	void set_buffer_size(uint size);
	void set_sample_rate(uint rate);
	void set_bit_depth(uint depth);
	void delay(float delay);

	void transport_cycle_start(trav_time_t time)
	{
		m_cycleStartTime = time;
	}

	void transport_cycle_end(trav_time_t time)
	{
		trav_time_t runcycleTime = time - m_cycleStartTime;
		m_cpuTime->write(&runcycleTime, 1);
	}

        Driver* get_driver() const {return m_driver;}

	void mili_sleep(int msec);
	void xrun();
	
	size_t run_audio_thread() const;
	
	enum {
		INFO = 0,
		WARNING = 1,
  		CRITICAL = 2
	};
	
	QVariant get_driver_property(const QString& property, QVariant defaultValue);

signals:
	/**
	 *      The stopped() signal is emited just before the AudioDeviceThread will be stopped.
	 *	Connect this signal to all Objects that have a pointer to an AudioBus (For example a VU meter),
	 *	since all he Buses will be deleted, and new ones created when the AudioDevice re-inits
	 *	the AudioDriver.
	 */
	void stopped();
	
	/**
	 *      The started() signal is emited ones the AudioThread and AudioDriver have been succesfully
	 *	setup.
	 */
	void started();
	
	/**
	 *      The driverParamsChanged() signal is emited just before the started() signal, you should 
	 *	connect all objects to this signal who need a pointer to one of the AudioBuses supplied by 
	 *	the AudioDevice!
	 */
	void driverParamsChanged();
	
	/**
	 *        Connect this signal to any Object who need to be informed about buffer under/overruns
	 */
	void bufferUnderRun();
	
	/**
	 *        This signal will be emited after succesfull Client removal from within the GUI Thread!
	 * @param  The Client \a client which as been removed from the AudioDevice
	 */
	void clientRemoved(AudioDeviceClient*);
	
	void xrunStormDetected();
	
	void message(QString, int);

        void busConfigChanged();

private slots:
	void private_add_client(AudioDeviceClient* client);
	void private_remove_client(AudioDeviceClient* client);
	void audiothread_finished();
	void switch_to_null_driver();
	void reset_xrun_counter() {m_xrunCount = 0;}
	void check_jack_shutdown();
};


// use this function to get the audiodevice object
AudioDevice& audiodevice();


inline size_t AudioDevice::run_audio_thread( ) const {return m_runAudioThread;}


#endif

//eof
