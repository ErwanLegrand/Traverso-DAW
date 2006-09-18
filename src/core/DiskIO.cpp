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

$Id: DiskIO.cpp,v 1.16 2006/09/18 18:30:14 r_sijrier Exp $
*/

#include "DiskIO.h"

#include "AudioSource.h"
#include "ReadSource.h"
#include "WriteSource.h"
#include "AudioDevice.h"
#include <QSettings>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/************** DISKIO THREAD ************/

DiskIOThread::DiskIOThread(DiskIO* diskio)
{
	realtime = false;
	m_diskio = diskio;

#ifndef MAC_OS_BUILD
// 	setStackSize(20000);
#endif
}

void DiskIOThread::run()
{
// 	become_realtime(true);
	exec();
	m_diskio->workTimer.stop();
}

void DiskIOThread::become_realtime( bool becomerealtime )
{
#if defined (LINUX_BUILD) || defined (MAC_OS_BUILD)
	if (becomerealtime) {
		struct sched_param param;
		param.sched_priority = 50;
		if (pthread_setschedparam (pthread_self(), SCHED_RR, &param) != 0) {
			PWARN("Unable to set SCHED_RR priority!");
		} else {
			printf("Running with SCHED_RR value 50\n");
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
#endif
}


/************** END DISKIO THREAD ************/


DiskIO::DiskIO()
{
	diskThread = new DiskIOThread(this);
	// Set the thread stack size. 0.2 MB should do IMHO
	seeking = false;
	stopWork = false;
	cpuTimeBuffer = new RingBuffer(2048);
	lastCpuReadTime = get_microseconds();
	QSettings settings;
	m_preBufferSize = settings.value("HardWare/PreBufferSize").toInt();

	// Move both this instance and the workTimer to the
	// diskThread, since they both have to live there....
	moveToThread(diskThread);
	workTimer.moveToThread(diskThread);

	// Start the diskThread
	diskThread->start();

	// due the fact both this instance and workTimer live in the same thread
	// we can connect the timeout signal to a slot of this instance.
	connect(&workTimer, SIGNAL(timeout()), this, SLOT(do_work()));
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
	PMESG2("DiskIO :: Entering seek");
	PMESG2("DiskIO :: thread id is: %ld", QThread::currentThreadId ());

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

	workTimer.start(20);

	emit seekFinished();

	PMESG2("DiskIO :: Leaving seek");
}

void DiskIO::do_work( )
{

	// 	printf("DiskIO :: Entering do_work\n");
	//  	printf("DiskIO :: thread id is: %ld\n", QThread::currentThreadId ());

	mutex.lock();
	
	int space = 0;
	audio_sample_t framebuffer[m_preBufferSize];
	
	/* Process WriteSources */
	if (writeSources.size() > 0) {	
		cycleStartTime = get_microseconds();
		qSort(writeSources.begin(), writeSources.end(), AudioSource::greater);
		
		for (int i=0; i<writeSources.size(); i++) {
	
			space = std::max(writeSources.at(i)->process_ringbuffer(framebuffer), space);
			
			if ( space == 1) {
				writeSources.removeAt( i );
			} else {
				bufferFillStatus = std::max(space, bufferFillStatus);
			}
	
		}
		update_time_usage();
	}
	/* END Process WriteSources */


	/* Process ReadSources */
	cycleStartTime = get_microseconds();
	
	qSort(readSources.begin(), readSources.end(), AudioSource::greater);

	foreach(ReadSource* source, readSources) {

		if (stopWork) {
			workTimer.stop();
			update_time_usage();
			mutex.unlock();
			return;
		}

		if ( ! source->is_active() )
			continue;

		if (source->need_sync()) {
			source->sync();
		} else {
			space = std::max(source->process_ringbuffer(framebuffer), space);
			bufferFillStatus = std::max(space, bufferFillStatus);
		}

	}
	
	update_time_usage();
	/* END Process ReadSources */
	
	if (space > (m_preBufferSize * 0.4)) {
		printf("restarting do_work()\n");
		mutex.unlock();
		return do_work();
	}

	mutex.unlock();
	

	// 	printf("DiskIO :: Leaving do_work\n\n");
}

int DiskIO::stop( )
{
	PENTER;
	int res = 0;

	// Stop any processing in do_work()
	stopWork = true;
// 	workTimer.stop();

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
	
	source->prepare_buffer();
	
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

void DiskIO::update_time_usage( )
{
	audio_sample_t runcycleTime = get_microseconds() - cycleStartTime;
	cpuTimeBuffer->write((char*)&runcycleTime, 1 * sizeof(audio_sample_t));

}

trav_time_t DiskIO::get_cpu_time( )
{
	trav_time_t currentTime = get_microseconds();
	float totaltime = 0;
	float value = 0;
	int read = cpuTimeBuffer->read_space() / sizeof(audio_sample_t);

	while (read != 0) {
		read = cpuTimeBuffer->read((char*)&value, 1 * sizeof(audio_sample_t));
		totaltime += value;
	}

	audio_sample_t result = ( (totaltime  / (currentTime - lastCpuReadTime) ) * 100 );

	lastCpuReadTime = currentTime;

	if (result > 95) {
		qWarning("DiskIO :: consuming more then 95 Percent CPU !!");
	}

	return result;
}

void DiskIO::unregister_read_source( ReadSource * source )
{
	QMutexLocker locker(&mutex);
	
	readSources.removeAll(source);
}

int DiskIO::get_buffer_fill_status( )
{
	QMutexLocker locker(&mutex);
	
	int status = (int) (((float)(m_preBufferSize - bufferFillStatus) / m_preBufferSize) * 100);
	bufferFillStatus = 0;
	
	return status;
}

//eof
