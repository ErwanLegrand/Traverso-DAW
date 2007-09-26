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
#include <QWaitCondition>

#include "defines.h"

class ReadSource;
class AudioSource;
class Peak;
class PPThread;
class DecodeBuffer;
class PeakDataReader;

class PeakProcessor : public QObject
{
	Q_OBJECT	
	
public:
	void queue_task(Peak* peak);
	void free_peak(Peak* peak);

private:
	PPThread* m_ppthread;
	QMutex m_mutex;
	QWaitCondition m_wait;
	bool m_taskRunning;
	Peak* m_runningPeak;
		
	QQueue<Peak* > m_queue;
	
	void dequeue_queue();
	
	PeakProcessor();
	~PeakProcessor();
	PeakProcessor(const PeakProcessor&);
	// allow this function to create one instance
	friend PeakProcessor& pp();
	
private slots:
	void start_task();
	
signals:
	void newTask();

};

class PPThread : public QThread
{
public:
	PPThread(PeakProcessor* pp);
	
protected:
	void run();
	
private:
	PeakProcessor* m_pp;
};


// use this function to access the PeakBuildThread
PeakProcessor& pp();


class Peak : public QObject
{
	Q_OBJECT

public:
	static const int ZOOM_LEVELS = 20;
	static const int SAVING_ZOOM_FACTOR = 6;
	static const int MAX_ZOOM_USING_SOURCEFILE = SAVING_ZOOM_FACTOR - 1;
	static const int MAX_DB_VALUE = 120;
	static int zoomStep[ZOOM_LEVELS + 1];

	Peak(AudioSource* source);
	~Peak();

	enum { 	NO_PEAKDATA_FOUND = -1,
		NO_PEAK_FILE = -2,
  		PERMANENT_FAILURE = -3
	};
		
	void process(uint channel, audio_sample_t* buffer, nframes_t frames);
	int prepare_processing(int rate);
	int finish_processing();
	int calculate_peaks(int chan, float** buffer, int zoomLevel, TimeRef startlocation, int count);

	void close();
	
	void start_peak_loading();

	audio_sample_t get_max_amplitude(nframes_t startframe, nframes_t endframe);

private:
	ReadSource* 	m_source;
	bool 		m_peaksAvailable;
	bool		m_permanentFailure;
	bool		m_interuptPeakBuild;
	
	struct ProcessData {
		ProcessData() {
			normValue = peakUpperValue = peakLowerValue = 0;
			processBufferSize = progress = normProcessedFrames = normDataCount = 0;
			processRange = TimeRef(64, 44100);
			nextDataPointLocation = processRange;
		}
		
		audio_sample_t		peakUpperValue;
		audio_sample_t		peakLowerValue;
		audio_sample_t		normValue;
		
		TimeRef			stepSize;
		TimeRef			processRange;
		TimeRef			processLocation;
		TimeRef			nextDataPointLocation;
		
		nframes_t		normProcessedFrames;
		
		int 			progress;
		int			processBufferSize;
		int			normDataCount;
	};
	
	struct PeakHeaderData {
		int peakDataOffset;
		int normValuesDataOffset;
		int peakDataLevelOffsets[ZOOM_LEVELS - SAVING_ZOOM_FACTOR];
		int peakDataSizeForLevel[ZOOM_LEVELS - SAVING_ZOOM_FACTOR];
		char label[6];	//TPFxxx -> Traverso Peak File version x.x.x
		int version[2];
	};

	struct ChannelData {
		QString		fileName;
		QString		normFileName;
		FILE* 		file;
		FILE*		normFile;
		PeakHeaderData	headerdata;
		PeakDataReader*	peakreader;
		ProcessData* 	pd;
		DecodeBuffer*	peakdataDecodeBuffer;
	};
	
	QList<ChannelData* >	m_channelData;
	
	int create_from_scratch();
	int read_header();
	int write_header(ChannelData* data);

	friend class PeakProcessor;
	friend class PeakDataReader;

signals:
	void finished();
	void progress(int m_progress);
};

class PeakDataReader
{
public:
	PeakDataReader(Peak::ChannelData* data);
	~PeakDataReader(){};

	nframes_t read_from(DecodeBuffer* buffer, nframes_t start, nframes_t count);

private:
	Peak::ChannelData* m_d;
	nframes_t	m_readPos;
	nframes_t	m_nframes;

	bool seek(nframes_t start);
	nframes_t read(DecodeBuffer* buffer, nframes_t frameCount);
};


#endif

//eof
