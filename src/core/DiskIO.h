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

$Id: DiskIO.h,v 1.2 2006/06/12 20:12:28 r_sijrier Exp $
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


class DiskIO : public QObject
{
	Q_OBJECT

public:
	DiskIO();
	~DiskIO();

	void prepare_for_seek();

	void register_read_source(ReadSource* source);
	void register_write_source(WriteSource* source);

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
	QTimer			bufferFillStatusTimer;
	int			bufferFillStatus;
	RingBuffer*		cpuTimeBuffer;
	trav_time_t		cycleStartTime;
	trav_time_t		lastCpuReadTime;

	void update_time_usage();
	int stop();

	friend class DiskIOThread;

public slots:
	void seek(uint position);
	void do_work();
	void update_buffer_fill_status();

signals:
	void seekFinished();
	void bufferFillStatusChanged(int status);
	void outOfSync();

};

#endif

//eof
