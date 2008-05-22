/*
Copyright (C) 2007 Remon Sijrier 

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

#include "AudioFileCopyConvert.h"
#include <QFile>
#include <QMutexLocker>
#include <QFileInfo>

#include "Export.h"
#include "AbstractAudioReader.h"
#include "ReadSource.h"
#include "WriteSource.h"
#include "Peak.h"
#include "defines.h"

AudioFileCopyConvert::AudioFileCopyConvert()
{
	m_stopProcessing = false;
	moveToThread(this);
	start();
	connect(this, SIGNAL(dequeueTask()), this, SLOT(dequeue_tasks()), Qt::QueuedConnection);
}

void AudioFileCopyConvert::enqueue_task(ReadSource * source, const QString& dir, const QString & outfilename, int tracknumber)
{
	QFileInfo fi(outfilename);

	CopyTask task;
	task.readsource = source;
	task.outFileName = fi.completeBaseName();
	task.extension = fi.suffix();
	task.tracknumber = tracknumber;
	task.dir = dir;
	
	m_mutex.lock();
	m_tasks.enqueue(task);
	m_mutex.unlock();
	
	emit dequeueTask();
}

void AudioFileCopyConvert::dequeue_tasks()
{
	m_mutex.lock();
	if (m_tasks.size()) {
		CopyTask task = m_tasks.dequeue();
		m_mutex.unlock();
		process_task(task);
		return;
	}
	m_mutex.unlock();
}

void AudioFileCopyConvert::process_task(CopyTask task)
{
	QString name = task.readsource->get_name();
	int length = name.length();
	emit taskStarted(name.left(length-28));
	uint buffersize = 16384;
	DecodeBuffer decodebuffer;
	
	ExportSpecification* spec = new ExportSpecification();
	spec->startLocation = TimeRef();
	spec->endLocation = task.readsource->get_length();
	spec->totalTime = spec->endLocation;
	spec->pos = TimeRef();
	spec->isRecording = false;
	
	spec->exportdir = task.dir;
	spec->writerType = "sndfile";
	spec->extraFormat["filetype"] = "wav";
	spec->data_width = 1;	// 1 means float
	spec->channels = task.readsource->get_channel_count();
	spec->sample_rate = task.readsource->get_rate();
	spec->blocksize = buffersize;
	spec->name = task.outFileName;
	spec->dataF = new audio_sample_t[buffersize * 2];
	
	WriteSource* writesource = new WriteSource(spec);
	if (writesource->prepare_export() == -1) {
		delete writesource;
		delete [] spec->dataF;
		delete spec;
		return;
	}
	// Enable on the fly generation of peak data to speedup conversion 
	// (no need to re-read all the audio files to generate peaks)
	writesource->set_process_peaks(true);
	
	int oldprogress = 0;
	do {
		// if the user asked to stop processing, jump out of this 
		// loop, and cleanup any resources in use.
		if (m_stopProcessing) {
			goto out;
		}
			
		nframes_t diff = (spec->endLocation - spec->pos).to_frame(task.readsource->get_rate());
		nframes_t this_nframes = std::min(diff, buffersize);
		nframes_t nframes = this_nframes;
		
		memset (spec->dataF, 0, sizeof (spec->dataF[0]) * nframes * spec->channels);
		
		task.readsource->file_read(&decodebuffer, spec->pos, nframes);
			
		for (uint x = 0; x < nframes; ++x) {
			for (int y = 0; y < spec->channels; ++y) {
				spec->dataF[y + x*spec->channels] = decodebuffer.destination[y][x];
			}
		}
		
		// due the fact peak generating does _not_ happen in writesource->process
		// but in a function used by DiskIO, we have to hack the peak processing 
		// in here.
		for (int y = 0; y < spec->channels; ++y) {
			writesource->get_peak()->process(y, decodebuffer.destination[y], nframes);
		}
		
		// Process the data, and write to disk
		writesource->process(buffersize);
		
		spec->pos.add_frames(nframes, task.readsource->get_rate());
		
		int currentprogress = int(double(spec->pos.universal_frame()) / double(spec->totalTime.universal_frame()) * 100);
		if (currentprogress > oldprogress) {
			oldprogress = currentprogress;
			emit progress(currentprogress);
		}
			
	} while (spec->pos != spec->totalTime);
		
	
	out:
	writesource->finish_export();
	delete writesource;
	delete [] spec->dataF;
	delete spec;
	
	//  The user asked to stop processing, exit the event loop
	// and signal we're done.
	if (m_stopProcessing) {
		exit(0);
		wait(1000);
		m_tasks.clear();
		emit processingStopped();
		return;
	}
	
	emit taskFinished(task.dir + "/" + task.outFileName + ".wav", task.tracknumber);
}

void AudioFileCopyConvert::stop_merging()
{
	m_stopProcessing = true;
}

