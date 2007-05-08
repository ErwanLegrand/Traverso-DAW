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

$Id: AudioDeviceThread.cpp,v 1.17 2007/05/08 14:48:31 r_sijrier Exp $
*/

#include "AudioDeviceThread.h"

#include "AudioDevice.h"
#include "Driver.h"
#include <Information.h>

#if defined (Q_WS_X11)
#include <sys/resource.h>
#include <sched.h>
#endif

#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

class WatchDogThread : public QThread
{
	AudioDeviceThread* guardedThread;

public:
	WatchDogThread(AudioDeviceThread* thread)
	{
		guardedThread = thread;
	}

protected:
	void run()
	{
#if defined (Q_WS_X11) || defined (Q_WS_MAC)
		struct sched_param param;
		param.sched_priority = 90;
		if (pthread_setschedparam (pthread_self(), SCHED_FIFO, &param) != 0) {}
#endif


		while(true) {
			sleep(5);
//                         printf("Checking watchdogCheck...\n");

			if (guardedThread->watchdogCheck == 0) {
				qCritical("WatchDog timed out!");
//				guardedThread->terminate();
#if defined (Q_WS_X11) || defined (Q_WS_MAC)
				kill (-getpgrp(), SIGABRT);
#endif
			}

			guardedThread->watchdogCheck = 0;
		}
	}
};

AudioDeviceThread::AudioDeviceThread(AudioDevice* device)
{
	m_device = device;
	m_realTime = true;
	setTerminationEnabled(true);

#ifndef Q_WS_MAC
	setStackSize(1000000);
#endif

	watchdogCheck = 1;
}


void AudioDeviceThread::run()
{
	run_on_cpu( 0 );

	
	WatchDogThread watchdog(this);
	watchdog.start();

	become_realtime(m_realTime);
	
	if (m_device->driver->start() < 0) {
		watchdog.terminate();
		watchdog.wait();
		return;
	}

	while (m_device->run_audio_thread()) {
		if (m_device->get_driver()->run_cycle() < 0) {
			PERROR("Driver cycle error, exiting!");
			break;
		}
		watchdogCheck = 1;
	}
	
	watchdog.terminate();
	watchdog.wait();
}


int AudioDeviceThread::become_realtime( bool realtime )
{
	m_realTime = realtime;
#if defined (Q_WS_X11) || defined (Q_WS_MAC)

	/* RTC stuff */
	if (realtime) {
		struct sched_param param;
		param.sched_priority = 70;
		if (pthread_setschedparam (pthread_self(), SCHED_FIFO, &param) != 0) {
			info().critical(tr("Unable to set Audiodevice Thread to realtime priority!!!"
				"This most likely results in unreliable playback/capture and "
				"lots of buffer underruns (== sound drops)."
				"In the worst case the program can even malfunction!"
				"Please make sure you run this program with realtime privileges!!!"));
			return -1;
		} else {
			printf("AudioThread: Running with realtime priority\n");
			return 1;
		}
	}

#endif

	return -1;
}

void AudioDeviceThread::run_on_cpu( int cpu )
{
#if defined (USE_CPU_AFFINITY)
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	if (sched_setaffinity(0, sizeof(mask), &mask)) {
		PWARN("Unable to set CPU affinity");
	} else {
		PMESG("Running AudioDeviceThread on CPU %d", cpu);
	}
#endif
}

//eof
