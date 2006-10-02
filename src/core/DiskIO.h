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

$Id: DiskIO.h,v 1.7 2006/10/02 19:04:38 r_sijrier Exp $
*/

#ifndef DISKIO_H
#define DISKIO_H

#include <QMutex>
#include <QList>

#include "defines.h"

class ReadSource;
class WriteSource;
class AudioSource;
class RingBuffer;
class DiskIOThread;
class Song;


class DiskIO : public QObject
{
	Q_OBJECT

public:
	DiskIO(Song* song);
	~DiskIO();

	void prepare_for_seek();

	void register_read_source(ReadSource* source);
	void register_write_source(WriteSource* source);
	
	void unregister_read_source(ReadSource* source);
	void unregister_write_source(WriteSource* source);

	trav_time_t get_cpu_time();
	int get_write_buffers_fill_status();
	int get_read_buffers_fill_status();
	int get_buffer_size() const;
	int get_chunk_size() const;

private:
	Song* 			m_song;
	volatile size_t		m_stopWork;
	QList<ReadSource*>	m_readSources;
	QList<AudioSource*>	m_processableSources;
	QList<WriteSource*>	m_writeSources;
	DiskIOThread*		m_diskThread;
	QMutex			mutex;
	volatile size_t		m_readBufferFillStatus;
	volatile size_t		m_writeBufferFillStatus;
	int			m_bufferSize;
	uint			m_minBufStatus;
	RingBuffer*		cpuTimeBuffer;
	trav_time_t		cycleStartTime;
	trav_time_t		lastCpuReadTime;
	bool			m_seeking;
	int			m_hardDiskOverLoadCounter;

	
	void update_time_usage();
	
	int stop();
	int there_are_processable_sources();

	friend class DiskIOThread;

public slots:
	void seek(uint position);
	
private slots:
	void do_work();

signals:
	void seekFinished();
	void readSourceBufferUnderRun();
	void writeSourceBufferOverRun();

};

inline int DiskIO::get_buffer_size() const
{
	return m_bufferSize;
}

inline int DiskIO::get_chunk_size() const
{
	return m_bufferSize / 8;
}

#endif

//eof
