/*
Copyright (C) 2006-2007 Remon Sijrier 

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

#ifndef DISKIO_H
#define DISKIO_H

#include <QMutex>
#include <QList>
#include <QTimer>

#include "defines.h"

class ReadSource;
class WriteSource;
class AudioSource;
class RingBuffer;
class DiskIOThread;
class Song;
struct DecodeBuffer;

struct BufferStatus {
	int	fillStatus;
	int	priority;
	bool	bufferUnderRun;
	bool	needSync;
};

class DiskIO : public QObject
{
	Q_OBJECT

public:
	DiskIO(Song* song);
	~DiskIO();
	
	static const int writebuffertime = 5;
	static const int bufferdividefactor = 8;

	void prepare_for_seek();
	void output_rate_changed();

	void register_read_source(ReadSource* source);
	void register_write_source(WriteSource* source);
	
	void unregister_read_source(ReadSource* source);
	void unregister_write_source(WriteSource* source);

	trav_time_t get_cpu_time();
	int get_write_buffers_fill_status();
	int get_read_buffers_fill_status();

private:
	Song* 			m_song;
	volatile size_t		m_stopWork;
	QList<ReadSource*>	m_readSources;
	QList<WriteSource*>	m_writeSources;
	QList<ReadSource*>	m_processableReadSources;
	QList<WriteSource*>	m_processableWriteSources;
	DiskIOThread*		m_diskThread;
	QTimer			m_workTimer;
	QMutex			mutex;
	volatile int		m_readBufferFillStatus;
	volatile int		m_writeBufferFillStatus;
	RingBuffer*		cpuTimeBuffer;
	trav_time_t		cycleStartTime;
	trav_time_t		lastCpuReadTime;
	bool			m_seeking;
	bool			m_sampleRateChanged;
	int			m_hardDiskOverLoadCounter;
	audio_sample_t*		framebuffer[2];
	audio_sample_t*		m_readbuffer;
	DecodeBuffer*		m_decodebuffer;

	
	void update_time_usage();
	
	int stop();
	int there_are_processable_sources();

	friend class DiskIOThread;

public slots:
	void seek(uint position);
	void start_io();
	void stop_io();
	
private slots:
	void do_work();

signals:
	void seekFinished();
	void readSourceBufferUnderRun();
	void writeSourceBufferOverRun();

};

#endif

//eof
