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

$Id: AudioDeviceThread.cpp,v 1.7 2006/08/25 11:13:17 r_sijrier Exp $
*/

#include "AudioDeviceThread.h"

#include "AudioDevice.h"
#include "Driver.h"

#if defined (LINUX_BUILD)
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
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
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
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
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
	transfer = false;
	realTime = true;
	setTerminationEnabled(true);

#ifndef MAC_OS_BUILD
	setStackSize(100000);
#endif

	watchdogCheck = 1;
}


void AudioDeviceThread::run()
{
	run_on_cpu( 0 );
	become_realtime(true);
	int err =0;

	WatchDogThread watchdog(this);
	watchdog.start();

	while (m_device->run_audio_thread()) {
		if ((err = m_device->get_driver()->run_cycle()) < 0) {
			PERROR("Driver cycle error, exiting!");
			break;
		}
		watchdogCheck = 1;
	}
	watchdog.terminate();
	watchdog.wait();
}


int AudioDeviceThread::transfer_start( )
{
	transfer = true;
	return 1;
}

int AudioDeviceThread::transfer_stop( )
{
	transfer = false;
	return 1;
}

void AudioDeviceThread::start_transfering( )
{}

int AudioDeviceThread::become_realtime( bool realtime )
{
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)

	/* RTC stuff */
	if (realtime) {
		struct sched_param param;
		param.sched_priority = 70;
		if (pthread_setschedparam (pthread_self(), SCHED_FIFO, &param) != 0) {
			qWarning("Unable to set Audiodevice Thread to realtime priority!!!\n"
				 "This most likely results in unreliable playback/capture and\n"
				 "lots of buffer underruns (== sound drops).\n"
				 "In the worst case the program can even abort!\n"
				 "Please make sure you run this program with realtime privileges!!!\n");
			return -1;
		} else {
			qDebug("Running realtime");
			return 1;
		}
	}

#endif

	return -1;
}

void AudioDeviceThread::run_on_cpu( int cpu )
{
#if defined (LINUX_BUILD)
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
