/*
Copyright (C) 2006 Remon Sijrier 

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

$Id: DiskIO.cpp,v 1.2 2006/04/25 17:24:24 r_sijrier Exp $
*/

#include "DiskIO.h"

#include "AudioSource.h"
#include "ReadSource.h"
#include "WriteSource.h"
#include "AudioDevice.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/************** DISKIO THREAD ************/

DiskIOThread::DiskIOThread( )
{
	realtime = false;
}

void DiskIOThread::run()
{
	exec();
}

void DiskIOThread::become_realtime( bool becomerealtime )
{
	if (becomerealtime) {
		struct sched_param param;
		param.sched_priority = 50;
		if (pthread_setschedparam (pthread_self(), SCHED_RR, &param) != 0) {
			PWARN("Unable to set SCHED_RR priority!");
		} else {
			PWARN("Running with SCHED_RR value 50");
			realtime = true;
		}
	} else {
		struct sched_param param;
		param.sched_priority = 0;
		if (pthread_setschedparam (pthread_self(), SCHED_OTHER, &param) != 0) {
			PWARN("Unable to set SCED_OTHER priority!");
		} else {
			realtime = false;
			PWARN("Running with SCED_OTHER value 0");
		}
	}
}


/************** END DISKIO THREAD ************/


DiskIO::DiskIO()
{
	diskThread = new DiskIOThread();
	seeking = false;
	stopWork = false;
	cpuTimeBuffer = new RingBuffer(2048);
	lastCpuReadTime = get_microseconds();

	// Move both this instance and the workTimer to the
	// diskThread, since they both have to live there....
	moveToThread(diskThread);
	workTimer.moveToThread(diskThread);

	// Start the diskThread
	diskThread->start();

	// due the fact both this instance and workTimer live in the same thread
	// we can connect the timeout signal to a slot of this instance.
	connect(&workTimer, SIGNAL(timeout()), this, SLOT(do_work()));
	connect(&bufferFillStatusTimer, SIGNAL(timeout()), this, SLOT (update_buffer_fill_status()));

	bufferFillStatusTimer.start(1000);

}

DiskIO::~DiskIO()
{
	PENTERDES;
	stop();
	delete cpuTimeBuffer;
	delete diskThread;
}

void DiskIO::seek( nframes_t position )
{
	printf("DiskIO :: Entering seek\n");
	printf("DiskIO :: thread id is: %ld\n", QThread::currentThreadId ());

	stopWork = false;
	seeking = true;

	// First, reset all buffers, and set new readPos.
	
	mutex.lock();
	
	foreach(ReadSource* source, readSources) {
		if (source->need_sync())
			continue;
		source->rb_seek_to_file_position(position);
	}

	mutex.unlock();
	
	// Now, fill the buffers like normal
	do_work();

	seeking = false;

	emit seekFinished();

	printf("DiskIO :: Leaving seek\n\n");
}

void DiskIO::do_work( )
{
	QMutexLocker locker(&mutex);
	
	// 	printf("DiskIO :: Entering do_work\n");
	//  	printf("DiskIO :: thread id is: %ld\n", QThread::currentThreadId ());

	/* Process WriteSources */
	
	cycleStartTime = get_microseconds();
	
	for (int i=0; i<writeSources.size(); i++) {
		
		if (writeSources.at(i)->process_ringbuffer(framebuffer) == 1) {
			writeSources.removeAt( i );
		}
		
	}
	
	update_time_usage();

	/* END Process WriteSources */
	

	/* Process ReadSources */
	
	cycleStartTime = get_microseconds();

	foreach(ReadSource* source, readSources) {
		
		if (stopWork) {
			workTimer.stop();
			update_time_usage();
			return;
		}
		
		if ( ! source->is_active() )
			continue;

		if (source->need_sync()) {
			source->sync();
			source->process_ringbuffer(framebuffer);
			source->set_rb_ready(true);
			continue;
		}
		
		source->process_ringbuffer(framebuffer);
	}
	
	update_time_usage();
	
	/* END Process ReadSources */

	workTimer.start(20);

	// 	printf("DiskIO :: Leaving do_work\n\n");
}

int DiskIO::stop( )
{
	PENTER;
	int res = 0;

	// Stop any processing in do_work()
	stopWork = true;
	workTimer.stop();

	// Exit the event loop
	diskThread->exit(0);

	// Wait for the Thread to return from event loop. 1000 ms should be enough,
	// if not, terminate this thread and print a warning!

	if ( ! diskThread->wait(1000) ) {
		qWarning("DiskIO :: Still running after 1 second wait, terminating!");
		diskThread->terminate();
		res = -1;
	}

	return res;
}

void DiskIO::register_read_source (ReadSource* source )
{
	PENTER2;
	QMutexLocker locker(&mutex);
	
	readSources.append(source);
}

void DiskIO::register_write_source( WriteSource * source )
{
	PENTER2;
	QMutexLocker locker(&mutex);
	
	writeSources.append(source);
}

void DiskIO::prepare_for_seek( )
{
	PENTER;
	// Stop any processing in do_work()
	stopWork = true;
}

void DiskIO::update_buffer_fill_status( )
{
	// 	printf("buffer status is: %d\n", bufferFillStatus);
	int status = bufferFillStatus;
	bufferFillStatus = 100;
	emit bufferFillStatusChanged(status);
}

void DiskIO::update_time_usage( )
{
	audio_sample_t runcycleTime = get_microseconds() - cycleStartTime;
	cpuTimeBuffer->write(&runcycleTime, 1);

}

trav_time_t DiskIO::get_cpu_time( )
{
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

	if (result > 95) {
		qWarning("DiskIO :: consuming more then 95 Percent CPU !!");
	}

	return result;
}

//eof
