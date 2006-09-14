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

$Id: DiskIO.h,v 1.5 2006/09/14 10:49:39 r_sijrier Exp $
*/

#ifndef DISKIO_H
#define DISKIO_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QList>
#include <QTimer>

#include "defines.h"

class ReadSource;
class WriteSource;
class DiskIO;
class RingBuffer;


class DiskIOThread : public QThread
{
public:
	DiskIOThread(DiskIO* diskio);

	void become_realtime(bool realtime);
	bool realtime;

protected:
	void run();

private:
	DiskIO*		m_diskio;

};

/** DiskIO handles all the read's and write's of AudioSources in it's private thread.
 *  Each Song class has it's own DiskIO instance. 
 * The DiskIO manages all the AudioSources
 * related to a Song, and makes sure the RingBuffers from the AudioSources are processed
 * in time.
 */
class DiskIO : public QObject
{
	Q_OBJECT

public:
	DiskIO();
	~DiskIO();

	/**
	 *        Interupts any pending AudioSource's buffer processing, and returns from do_work().
	 *	Use this before calling seek() to shorten the seek process.
	 */
	void prepare_for_seek();

	/**
	 *        Registers the ReadSource. The source's RingBuffer will be initalized at this point.
	 *
	 *	This function is thread save. 
	 * @param source The ReadSource to register
	 */
	void register_read_source(ReadSource* source);
	/**
	 *        Registers the WriteSource. The source's RingBuffer will be initalized at this point.
	 *
	 *	This function is thread save. 
	 * @param source The WriteSource to register
	 */
	void register_write_source(WriteSource* source);
	
	void unregister_read_source(ReadSource* source);

	/**
	 *
	 * @return Returns the CPU time consumed by the DiskIO work thread 
	 */
	trav_time_t get_cpu_time();

private:
	audio_sample_t 		framebuffer[131072];
	bool			stopWork;
	bool			seeking;
	QList<ReadSource*>	readSources;
	QList<WriteSource*>	writeSources;
	QTimer			workTimer;
	DiskIOThread*		diskThread;
	QMutex			mutex;
	int			bufferFillStatus;
	RingBuffer*		cpuTimeBuffer;
	trav_time_t		cycleStartTime;
	trav_time_t		lastCpuReadTime;

	void update_time_usage();
	int stop();

	friend class DiskIOThread;

public slots:
	/**
	 *        Seek's all the ReadSource's readbuffers to the new position.
	 *	Call prepare_seek() first, to interupt do_work() if it was running.
	 * @param position 
	 */
	void seek(uint position);
	
private slots:
	void do_work();

signals:
	void seekFinished();
	void bufferFillStatusChanged(int status);
	void outOfSync();

};

#endif

//eof
