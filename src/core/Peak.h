/*
Copyright (C) 2005-2007 Remon Sijrier

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

#ifndef PEAK_H
#define PEAK_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QQueue>

#include "defines.h"

class ReadSource;
class AudioSource;
class Peak;


class PeakBuildThread : public QThread
{
	Q_OBJECT
public:
	
	void queue_task(Peak* peak);
	
private:
	QMutex m_mutex;
	
	void run();
	
	int m_runningTasks;
		
	QQueue<Peak* > m_queue;
	
	PeakBuildThread();
	PeakBuildThread(const PeakBuildThread&);
	// allow this function to create one instance
	friend PeakBuildThread& peakbuilder();
	
private slots:
	void start_task(Peak* peak);
	
signals:
	void newTask(Peak* peak);

};

// use this function to access the PeakBuildThread
PeakBuildThread& peakbuilder();


struct PeakData {
	int peakDataOffset;
	int normValuesDataOffset;
	int peakDataLevelOffsets[20 - 6];  // FIXME: Magic Numbers!
	int peakDataSizeForLevel[20 - 6];
	char label[6];	//TPFxxx -> Traverso Peak File version x.x.x
	int version[2];

};

class Peak : public QObject
{
	Q_OBJECT

public:
	static const int ZOOM_LEVELS = 20;
	static const int MAX_ZOOM_USING_SOURCEFILE;
	static const int MAX_DB_VALUE;
	static int zoomStep[ZOOM_LEVELS + 1];

	Peak(AudioSource* source, int channel = -1);
	~Peak();

	enum { 	NO_PEAKDATA_FOUND = -1,
		NO_PEAK_FILE = -2,
  		PERMANENT_FAILURE = -3
	};
		
	void process(audio_sample_t* buffer, nframes_t frames);
	int prepare_processing();
	int finish_processing();
	int calculate_peaks(void* buffer, int zoomLevel, nframes_t startPos, int count);

	void free_buffer_memory();
	
	void start_peak_loading();

	audio_sample_t get_max_amplitude(nframes_t startframe, nframes_t endframe);

private:
	ReadSource* 		m_source;
	bool 			peaksAvailable;
	bool			permanentFailure;
	bool			interuptPeakBuild;
	nframes_t		processedFrames;
	nframes_t		normProcessedFrames;
	int			processBufferSize;
	int			normDataCount;
	int 			m_progress;
	int			m_channel;
	audio_sample_t		peakUpperValue;
	audio_sample_t		peakLowerValue;
	audio_sample_t		normValue;

	PeakData		m_data;
	FILE* 			m_file;
	FILE*			m_normFile;
	QString			m_fileName;
	QString			m_normFileName;

	int create_from_scratch();

	int read_header();
	int write_header();

	friend class PeakBuildThread;

signals:
	void finished(Peak*);
	void progress(int m_progress);
};


#endif

//eof
