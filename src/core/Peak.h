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
#include <QFile>
#include <QHash>
#include <QPair>

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
	static const int ZOOM_LEVELS = 22;
	static const int SAVING_ZOOM_FACTOR = 8;
	static const int MAX_ZOOM_USING_SOURCEFILE = SAVING_ZOOM_FACTOR - 1;
	// Use ~ 1/4 the range of peak_data_t (== short) so we have headroom
	// for samples in the range [-4, +4] or + 12 dB
	static const int MAX_DB_VALUE = 8000;
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
	int calculate_peaks(int chan, float** buffer, TimeRef startlocation, int peakDataCount, qreal framesPerPeak, qreal& scaleFactor);

	void close();
	
	void start_peak_loading();

	audio_sample_t get_max_amplitude(TimeRef startlocation, TimeRef endlocation);
	
	static QHash<int, int>* cache_index_lut();
	static int max_zoom_value();

private:
	ReadSource* 	m_source;
	bool 		m_peaksAvailable;
	bool		m_permanentFailure;
	bool		m_interuptPeakBuild;
	static QHash<int, int> chacheIndexLut;
	
	struct ProcessData {
		ProcessData() {
			normValue = peakUpperValue = peakLowerValue = 0;
			processBufferSize = progress = normProcessedFrames = normDataCount = 0;
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
		int headerSize;
		int normValuesDataOffset;
		int peakDataOffsets[ZOOM_LEVELS - SAVING_ZOOM_FACTOR];
		int peakDataSizeForLevel[ZOOM_LEVELS - SAVING_ZOOM_FACTOR];
		char label[6];	//TPFxxx -> Traverso Peak File version x.x.x
		int version[2];
	};

	struct ChannelData {
		ChannelData() {
			memory = 0;
			peakdataDecodeBuffer = 0;
		}
		~ChannelData();
		QString		fileName;
		QString		normFileName;
		QFile 		file;
		QFile		normFile;
		PeakHeaderData	headerdata;
		PeakDataReader*	peakreader;
		ProcessData* 	pd;
		DecodeBuffer*	peakdataDecodeBuffer;
		uchar*		memory;
		QHash<uchar *, QPair<int /*offset*/, int /*handle|len*/> > maps;
	};
	
	QList<ChannelData* >	m_channelData;
	
	int create_from_scratch();
	int read_header();
	int write_header(ChannelData* data);
	static void calculate_lut_data();

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

inline QHash< int, int > * Peak::cache_index_lut()
{
	if(chacheIndexLut.isEmpty()) {
		calculate_lut_data();
	}
	return &chacheIndexLut;
}


#endif

//eof
