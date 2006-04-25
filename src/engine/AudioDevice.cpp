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

$Id: AudioDevice.cpp,v 1.2 2006/04/25 16:50:29 r_sijrier Exp $
*/

#include "AudioDevice.h"
#include "AudioDeviceThread.h"

#include "AlsaDriver.h"
#include "JackDriver.h"
#include "Driver.h"
#include "Client.h"
#include "AudioChannel.h"
#include "AudioBus.h"
#include <sys/mman.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

AudioDevice& audiodevice()
{
	static AudioDevice device;
	return device;
}

AudioDevice::AudioDevice()
{
	runAudioThread = false;
	driver = 0;
	audioThread = 0;
	processClientRequest = 0;
	cpuTimeBuffer = new RingBuffer(4096);
	
	clientRequestsRetryTimer.setSingleShot( true );
	connect(&clientRequestsRetryTimer, SIGNAL(timeout()), this, SLOT (process_client_request()));
	

	m_driverType = tr("No Driver Loaded");
#if defined (JACK_SUPPORT)

	availableDrivers << "Jack";
#endif

#if defined (ALSA_SUPPORT)

	availableDrivers << "ALSA";
#endif

	availableDrivers << "Null Driver";
}

AudioDevice::~AudioDevice()
{
	if (driver)
		delete driver;

	if (audioThread)
		delete audioThread;

	delete cpuTimeBuffer;

	free_memory();
}

void AudioDevice::free_memory()
{
	foreach(AudioBus* bus, captureBuses)
		delete bus;

	foreach(AudioBus* bus, playbackBuses)
		delete bus;

	captureChannels.clear();
	playbackChannels.clear();
	captureBuses.clear();
	playbackBuses.clear();
}

void AudioDevice::show_descriptors( )
{}

void AudioDevice::set_buffer_size( nframes_t size )
{
	m_bufferSize = size;
}

void AudioDevice::set_sample_rate( nframes_t rate )
{
	m_rate = rate;
}

void AudioDevice::set_bit_depth( uint depth )
{
	m_bitdepth = depth;
}

int AudioDevice::run_cycle( nframes_t nframes, float delayed_usecs )
{
	nframes_t left;

	if (nframes != m_bufferSize) {
		printf ("late driver wakeup: nframes to process = %ld\n", (long)nframes);
	}


	/* run as many cycles as it takes to consume nframes (Should be 1 cycle!!)*/
	for (left = nframes; left >= m_bufferSize; left -= m_bufferSize) {
		if (run_one_cycle (m_bufferSize, delayed_usecs) < 0) {
			qCritical ("cycle execution failure, exiting");
			return EIO;
		}
	}

	post_process();
	
	return 1;
}

int AudioDevice::run_one_cycle( nframes_t nframes, float  )
{

	if (driver->read(nframes) < 0) {
		qDebug("driver read failed!");
		return -1;
	}

	foreach(Client* client, clients) {
		if (client->process(nframes) < 0) {}
	}
	if (driver->write(nframes) < 0) {
		qDebug("driver write failed!");
		return -1;
	}


	return 0;
}

void AudioDevice::delay( float  )
{
}

uint AudioDevice::capture_buses_count( )
{
	return captureChannels.size();
}

uint AudioDevice::playback_buses_count( )
{
	return playbackChannels.size();
}

void AudioDevice::set_parameters( int rate, nframes_t bufferSize, QString driverType )
{
	PENTER;
	
	m_rate = rate;
	m_bufferSize = bufferSize;

	if (driver) {
		
		emit stopped();
		
		if (audioThread) {
			shutdown();
		}
		delete driver;
		driver = 0;
	}


	if (create_driver(driverType) > 0) {
		driver->attach();
		setup_buses();

		emit driverParamsChanged();

		driver->start();

		if ((driverType == "ALSA") || (driverType == "Null Driver")) {
			printf("Starting AudioDeviceThread..... ");
			runAudioThread = true;
			if (!audioThread)
				audioThread = new AudioDeviceThread(this);

			// cycleStartTime/EndTime are set before/after the first cycle.
			// to avoid a "100%" cpu usage value during audioThread startup, set the
			// cycleStartTime here!
			cycleStartTime = get_microseconds();

			audioThread->start();
			if (audioThread->isRunning()) {
				printf("Running!\n");
			}
			running = true;
		}
		
		emit started();
	} else {
		set_parameters(rate, bufferSize, "Null Driver");
	}
}

int AudioDevice::create_driver( QString driverType )
{

#if defined (JACK_SUPPORT)
	if (driverType == "Jack") {
		driver = new JackDriver(this, m_rate, m_bufferSize);
		if (driver->setup() < 0) {
			printf("Jack Driver creation failed\n");
			delete driver;
			driver = 0;
			return -1;
		}
		m_driverType = driverType;
		return 1;
	}
#endif

#if defined (ALSA_SUPPORT)
	if (driverType == "ALSA") {
		driver =  new AlsaDriver(this, m_rate, m_bufferSize);
		if (driver->setup() < 0) {
			printf("ALSA driver creation failed\n");
			delete driver;
			driver = 0;
			return -1;
		}
		m_driverType = driverType;
		return 1;
	}
#endif

	if (driverType == "Null Driver") {
		printf("Creating Null Driver...\n");
		driver = new Driver(this, m_rate, m_bufferSize);
		m_driverType = driverType;
		return 1;
	}

	return -1;
}


AudioChannel* AudioDevice::register_capture_channel(QByteArray busName, QString audioType, int flags, uint , uint channel )
{
	AudioChannel* bus = new AudioChannel(busName, audioType, flags, channel);
	captureChannels.insert(busName, bus);
	return bus;
}

AudioChannel* AudioDevice::register_playback_channel(QByteArray busName, QString audioType, int flags, uint , uint channel )
{
	AudioChannel* bus = new AudioChannel(busName, audioType, flags, channel);
	playbackChannels.insert(busName, bus);
	return bus;
}

void AudioDevice::unregister_capture_channel( QByteArray name )
{
	AudioChannel* bus = captureChannels.take(name);
	if (bus)
		delete bus;
}

void AudioDevice::unregister_playback_channel( QByteArray name )
{
	AudioChannel* bus = playbackChannels.take(name);
	if (bus)
		delete bus;
}

AudioChannel * AudioDevice::get_playback_channel( QByteArray name )
{
	return playbackChannels.value(name);
}

AudioChannel * AudioDevice::get_capture_channel( QByteArray name )
{
	return captureChannels.value(name);
}

int AudioDevice::shutdown( )
{
	PENTER;
	
	if (audioThread) {
		runAudioThread = false;
		// Wait until the audioThread has finished execution. One second
		// should do, if it's still running then, the thread must have gone wild or something....
		int r = audioThread->wait(1000);

		free_memory();

		return r;
	}


	return 1;
}

QStringList AudioDevice::get_capture_buses_names( )
{
	QStringList names;
	foreach(AudioBus* bus, captureBuses) {
		names.append(bus->get_name());
	}
	return names;
}

QStringList AudioDevice::get_playback_buses_names( )
{
	QStringList names;
	foreach(AudioBus* bus, playbackBuses) {
		names.append(bus->get_name());
	}
	return names;
}

void AudioDevice::setup_buses( )
{
	int number = 1;
	QByteArray name;

	for (int i=1; i <= captureChannels.size();) {
		name = "Capture " + QByteArray::number(number++);
		AudioBus* bus = new AudioBus(name);
		bus->add_channel(captureChannels.value("capture_"+QByteArray::number(i++)));
		bus->add_channel(captureChannels.value("capture_"+QByteArray::number(i++)));
		captureBuses.insert(name, bus);
	}
// 	PWARN("Capture buses count is: %d", captureBuses.size());

	number = 1;

	for (int i=1; i <= playbackChannels.size();) {
		name = "Playback " + QByteArray::number(number++);
		AudioBus* bus = new AudioBus(name);
		bus->add_channel(playbackChannels.value("playback_"+QByteArray::number(i++)));
		bus->add_channel(playbackChannels.value("playback_"+QByteArray::number(i++)));
		playbackBuses.insert(name, bus);
	}
// 	PWARN("Playback buses count is: %d", playbackBuses.size());
}

uint AudioDevice::get_sample_rate( )
{
	return m_rate;
}

uint AudioDevice::get_bit_depth( )
{
	return m_bitdepth;
}

QString AudioDevice::get_device_name( )
{
	if (driver)
		return driver->get_device_name();
	return tr("No Device Configured");
}

QString AudioDevice::get_device_longname( )
{
	if (driver)
		return driver->get_device_longname();
	return tr("No Device Configured");
}

QStringList AudioDevice::get_avaible_drivers( )
{
	return availableDrivers;
}

QString AudioDevice::get_driver_type( )
{
	return m_driverType;
}

trav_time_t AudioDevice::get_cpu_time( )
{
	if (driver && m_driverType == "Jack")
		return ((JackDriver*)driver)->get_cpu_load();

	trav_time_t currentTime = get_microseconds();
	float totaltime = 0;
	float value = 0;
	int read = cpuTimeBuffer->read_space();

	while (read != 0) {
		read = cpuTimeBuffer->read(&value, 1);
		totaltime += value;
	}

	audio_sample_t result = ( (totaltime  / (currentTime - lastCpuReadTime) ) * 100 );

	lastCpuReadTime = currentTime;

	return result;
}

void AudioDevice::post_process( )
{
	if (! processClientRequest) {
		return;
	}
	
	PENTER;
	PMESG("AudioDevice::post_process(). Thread id:  %ld", QThread::currentThreadId ());
	
	if ( ! mutex.tryLock() ) {
		printf("AudioDevice :: Couldn't lock mutex, retrying next cycle");
		return;
	}
	
	
	for (int i=0; i<newClients.size(); ++i) {
		Client* client = newClients.at( i );
		PMESG("appending client %s", client->m_name.toAscii().data());
		clients.append( client );
	}
	
	newClients.clear();
	
	bool clientdeleted;
	
	do {
		clientdeleted = false;
		
		for (int i=0; i<clients.size(); ++i) {
			Client* client = clients.at( i );
			
			if (client->scheduled_for_deletion()) {
				delete clients.takeAt( i );
				clientdeleted = true;
			}
		
		}
	}
	while (clientdeleted);
	
	mutex.unlock();
	
	processClientRequest = 0;
	
	emit clientRequestsProcesssed();
}

void AudioDevice::process_client_request( )
{
	PENTER;
	static int retryCount;
	
	if (processClientRequest) {
		
		clientRequestsRetryTimer.start( 10 );
		
		PMESG("processClientRequest is still true, retrying in 10 ms !!");
		retryCount++;
		
		if (retryCount > 100) {
			qFatal("Cannot start AudioDevice::process_client_reques, processClientRequest remains true.\n"
					"This  happens when the audiodevice didn't handle the client request. PLEASE report me as a bug");
		}
		
		return;
	}
	
	processClientRequest = 1;
	retryCount = 0;
}

Client* AudioDevice::new_client( QString name )
{
	PENTER;
	PMESG("New client %s", name.toAscii().data());
	
	mutex.lock();
	
	Client* client = new Client(name);
	newClients.append( client );
	
	mutex.unlock();

	return client;
}

//eof
