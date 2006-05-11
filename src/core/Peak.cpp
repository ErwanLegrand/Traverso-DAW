/*
Copyright (C) 2005-2006 Remon Sijrier 

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

$Id: Peak.cpp,v 1.4 2006/05/11 13:56:24 r_sijrier Exp $
*/

#include "libtraversocore.h"

#include "Peak.h"
#include "AudioSource.h"
#include "ReadSource.h"
#include "defines.h"
#include "Mixer.h"

#include "Debugger.h"

/* Store for each zoomStep the upper and lower sample to hard disk as a
* unsigned char. The top-top resolution is then 512 pixels, which should do
* Painting the waveform will be as simple as painting a line starting from the
* lower value to the upper value.
* We only store the buffer from zoomStep == 64. This way the stored buffer on hard
* disk doesn't consume to much space. Since all zoomSteps are a power of 2, we can
* generate all zoomSteps > 64 from the buffer stored on hard disk
* zoomSteps < 64 are for sure in Micro View mode, so the file is scanned to generate
* the waveform.
*/

const int Peak::MAX_DB_VALUE			= 120;
const int SAVING_ZOOM_FACTOR 			= 6;
const uint INITIAL_PROCESSBUFFER_SIZE 		= 3000000;

const int Peak::MAX_ZOOM_USING_SOURCEFILE	= SAVING_ZOOM_FACTOR - 1;

int Peak::zoomStep[ZOOM_LEVELS] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
				8192, 16384, 32768, 65536, 131072};

Peak::Peak(AudioSource* source)
		: m_source(source)
{
	peaksAvailable = false;
	processBuffer = 0;
	preparedBuffer = new PeakBuffer();
	peakBuildThread = 0;
}

Peak::~Peak()
{
	PENTERDES;
	free_buffer_memory();
}

int Peak::load_peaks()
{
	PENTER;
	if (!peakBuildThread) {
// 		PWARN("Starting peak build thread");
		peakBuildThread = new PeakBuildThread(this);
		peakBuildThread->start();
	}
	return 1;
}

int Peak::prepare_buffer_for_zoomlevel(int zoomLevel, nframes_t startPos, nframes_t nframes)
{
	if(!peaksAvailable) {
		if (load_peaks()) {
			// Somehow the caller has to know peak buffers are created by a seperate thread.
			// Returning 0 means peaks are created. Returning -1 means, something went wrong!
			return 0;
		}
	}
	// Macro View mode
	if (zoomLevel > MAX_ZOOM_USING_SOURCEFILE) {
		preparedBufferPointer = bufferList.value(zoomLevel);
		nframes_t offset = startPos / zoomStep[zoomLevel];
		if (nframes > (preparedBufferPointer->get_size() - offset)) {
			nframes = (preparedBufferPointer->get_size() - offset);
		}
		return nframes;
	}
	// Micro view mode
	else {
		nframes_t readFrames, toRead;
		toRead = nframes * zoomStep[zoomLevel];
		audio_sample_t buf[toRead];

		if ( (readFrames = static_cast<ReadSource*>(m_source)->file_read(buf, startPos, toRead))
				!= toRead) {
			PWARN("Unable to read nframes %d (only %d available)", nframes, readFrames);
			if (readFrames == 0)
				return -1;
			nframes = readFrames;
		}

		preparedBuffer->set_size(nframes);
		preparedBuffer->set_microview_buffer_size(nframes);
		nframes_t count = 0;
		nframes_t pos = 0;
		audio_sample_t valueMax, valueMin, sample;
		short value;

		do {
			valueMax = valueMin = 0;

			for(int i=0; i < zoomStep[zoomLevel]; i++) {
				if (pos > readFrames)
					break;
				sample = buf[pos];
				if (sample > valueMax)
					valueMax = sample;
				if (sample < valueMin)
					valueMin = sample;
				pos++;
			}

			if (valueMax > (valueMin * -1)) {
				value = (short)(valueMax * MAX_DB_VALUE);
				preparedBuffer->set_microview_buffer_value(value, count);
			} else {
				value = (short)(valueMin * MAX_DB_VALUE);
				preparedBuffer->set_microview_buffer_value(value, count);
			}
			count++;
		} while(count < nframes);

		preparedBufferPointer = preparedBuffer;

		return count;
	}

	return 0;
}

void Peak::prepare_process_buffer()
{
	processBuffer = new PeakBuffer();
	processBuffer->set_size(INITIAL_PROCESSBUFFER_SIZE);
	peakUpperValue = peakLowerValue = processBufferPos = processedFrames = 0;
}

void Peak::finish_process_buffer()
{
	// It's likely there is a remaining part of the processed recording buffer
	// which largest value isn't stored, so check for it, and append to the processBuffer
	if (processedFrames != 0) {
		unsigned char* upperHalfBuffer = processBuffer->get_upper_half_buffer();
		unsigned char* lowerHalfBuffer = processBuffer->get_lower_half_buffer();
		upperHalfBuffer[processBufferPos] = (unsigned char)(peakUpperValue * MAX_DB_VALUE);
		lowerHalfBuffer[processBufferPos] = (unsigned char)(peakLowerValue * MAX_DB_VALUE);
	}

	processBuffer->set_recording_size(processBufferPos + 1);

	// It should be save now to create the buffers for all cached levels.
	// We do so by calling:
	// 	load_peaks();
}

void Peak::process(audio_sample_t* buffer, nframes_t nframes)
{
	audio_sample_t sample;
	unsigned char* upperHalfBuffer = processBuffer->get_upper_half_buffer();
	unsigned char* lowerHalfBuffer = processBuffer->get_lower_half_buffer();
	for (uint i=0; i < nframes; i++) {
		sample = buffer[i];
		if (sample > peakUpperValue)
			peakUpperValue = sample;
		if (sample < peakLowerValue)
			peakLowerValue = sample;
		if (processedFrames == 64) {
			
			upperHalfBuffer[processBufferPos] = (unsigned char) (peakUpperValue * MAX_DB_VALUE );
			lowerHalfBuffer[processBufferPos] = (unsigned char) ((-1) * (peakLowerValue * MAX_DB_VALUE ));

			peakUpperValue = 0.0;
			peakLowerValue = 0.0;
			// FIXME If processBufferPos is larger then processBuffer->get_size(),
			// we get a segfault here. How to handle this situation....
			processBufferPos++;
			processedFrames = 0;
		}
		processedFrames++;
	}
}

int Peak::create_buffers()
{
	nframes_t nframes = m_source->get_nframes();
	m_progress = 0;

	for (int i=SAVING_ZOOM_FACTOR; i < ZOOM_LEVELS; i++) {
		PeakBuffer* buf = new PeakBuffer();
		buf->set_size( (nframes / zoomStep[i]) );
		bufferList.insert(i, buf);
	}

	// If no processBuffer, try to load prepared buffers from hard disk
	if (!processBuffer) {
		if (load() < 0) {
			if (create_from_scratch() < 0) {
				PWARN("not able to build peaks from scratch, something weird is going on!");
				return -1;
			}
		}
	}

	// Now the peak buffer for zoomStep 64 should be available, either from hard disk
	// or created during record.

	// If there is a processBuffer, it's save now to copy it to the bufferList's buffer for zoomStep 64
	// Else, the buffer is filled from hard disk
	if (processBuffer) {
		unsigned char* upperHalfBuffer = processBuffer->get_upper_half_buffer();
		unsigned char* lowerHalfBuffer = processBuffer->get_lower_half_buffer();
		int level = SAVING_ZOOM_FACTOR;
		nframes_t size = bufferList.value(level)->get_size();
		nframes_t otherSize = processBuffer->get_size();
		if (size != otherSize)
			PWARN("size is %d, processBufferSize is %d", size, otherSize);
		memcpy(bufferList.value(level)->get_upper_half_buffer(), upperHalfBuffer, size);
		memcpy(bufferList.value(level)->get_lower_half_buffer(), lowerHalfBuffer, size);
	}

	nframes_t bufPos, prevBufPos, bufferSize;

	for (int hzoom=SAVING_ZOOM_FACTOR+1; hzoom<ZOOM_LEVELS; hzoom++) {
		unsigned char* upperHalfBuffer = bufferList.value(hzoom)->get_upper_half_buffer();
		unsigned char* lowerHalfBuffer = bufferList.value(hzoom)->get_lower_half_buffer();
		unsigned char* prevUpperHalfBuffer = bufferList.value(hzoom - 1)->get_upper_half_buffer();
		unsigned char* prevLowerHalfBuffer = bufferList.value(hzoom - 1)->get_lower_half_buffer();
		bufferSize = bufferList.value(hzoom)->get_size();
		bufPos = 0;
		prevBufPos = 0;
		do {
			upperHalfBuffer[bufPos] = (unsigned char) f_max(prevUpperHalfBuffer[prevBufPos], prevUpperHalfBuffer[prevBufPos + 1]);
			lowerHalfBuffer[bufPos] = (unsigned char) f_max(prevLowerHalfBuffer[prevBufPos], prevLowerHalfBuffer[prevBufPos + 1]);
			prevBufPos += 2;
			bufPos++;
		} while (bufPos < bufferSize);
	}

	// OK, we're done on recording, delete the processBuffer!
	if (processBuffer) {
		save();
		delete processBuffer;
		processBuffer = 0;
	}

	peaksAvailable = true;
	emit finished();

	return 1;
}

int Peak::save()
{
	QString fileName;
	// Imported audio can have more then one channel. Use a different naming to
	// avoid peak file overwriting! This won't happen for single channel audio source,
	// what we use internally
	if (m_source->get_channel_count() > 1) {
		QString channelNumber = "-" + QByteArray::number(m_source->get_channel());
		fileName = pm().get_project()->get_root_dir() + "/peakfiles/" + m_source->get_name() + ".peak" + channelNumber;
	} else {
		fileName = pm().get_project()->get_root_dir() + "/peakfiles/" + m_source->get_name() + ".peak";
	}

	FILE* file = fopen(fileName.toAscii().data(),"w+");

	if (!file) {
		PERROR("Cannot save Peak image to %s", fileName.toAscii().data());
		return -1;
	}

	PeakBuffer* buf = bufferList.value(SAVING_ZOOM_FACTOR);
	if (!buf) {
		PERROR("No PeakBuffer for this zoomLevel (%d)", SAVING_ZOOM_FACTOR);
		return -1;
	}

	fwrite(buf->get_upper_half_buffer(), sizeof(unsigned char), buf->get_size(), file);
	fwrite(buf->get_lower_half_buffer(), sizeof(unsigned char), buf->get_size(), file);
	fclose(file);
	return 1;
}

int Peak::load()
{
	QString fileName;
	if (m_source->get_channel_count() > 1) {
		QString channelNumber = "-" + QByteArray::number(m_source->get_channel());
		fileName = pm().get_project()->get_root_dir() + "/peakfiles/" + m_source->get_name() + ".peak" + channelNumber;
	} else {
		fileName = pm().get_project()->get_root_dir() + "/peakfiles/" + m_source->get_name() + ".peak";
	}

	FILE* file = fopen(fileName.toAscii().data(),"r+");

	if (!file) {
		PERROR("Cannot open Peak image file for reading: %s", fileName.toAscii().data());
		return -1;
	}

	PeakBuffer* buf = bufferList.value(SAVING_ZOOM_FACTOR);
	nframes_t readUpperFrames, readLowerFrames;
	readUpperFrames = fread(buf->get_upper_half_buffer(), sizeof(char), buf->get_size(), file);
	readLowerFrames = fread(buf->get_lower_half_buffer(), sizeof(char), buf->get_size(), file);
	if ((readUpperFrames + readLowerFrames) != (buf->get_size() * 2)) {
		PWARN("Unable to read complete peak buffer from hard disk");
		return -1;
	}

	return 1;
}

int Peak::create_from_scratch()
{
	PENTER;
	prepare_process_buffer();
	nframes_t readFrames = 0;
	nframes_t totalReadFrames = 0;
	nframes_t bufferSize = 65536;
	int cycles = m_source->get_nframes() / bufferSize;
	int counter = 0;
	int p = 0;
	audio_sample_t* buf = new audio_sample_t[bufferSize];
	
	if (m_source->get_nframes() == 0) {
		qWarning("Peak::create_from_scratch() : m_source (%s) has length 0", m_source->get_name().toAscii().data());
		return -1;
	}
	
	if (cycles == 0) {
		bufferSize = 64;
		cycles = m_source->get_nframes() / bufferSize;
		if (cycles == 0) {
			qDebug("source length is too short to display one pixel of the audio wave form in macro view");
			return -1;
		}
	}
	
	do {
		readFrames = static_cast<ReadSource*>(m_source)->file_read(buf, totalReadFrames, bufferSize);
		process(buf, readFrames);
		totalReadFrames += readFrames;
		counter++;
		p = (int) (counter*100) / cycles;
		if ( p > m_progress) {
			m_progress = p;
			emit progress(m_progress);
		}
	} while(totalReadFrames != m_source->get_nframes());

	delete []  buf;

	finish_process_buffer();
	return 1;
}

void Peak::free_buffer_memory( )
{
	peaksAvailable = false;

	foreach(PeakBuffer* buffer, bufferList)
		delete buffer;

	delete preparedBuffer;
}

void Peak::set_audiosource( AudioSource * source )
{
	m_source = source;
}

/******** PEAK BUILD THREAD CLASS **********/
/******************************************/

PeakBuildThread::PeakBuildThread(Peak* peak)
{
	m_peak = peak;
}

void PeakBuildThread::run()
{
	if(m_peak->create_buffers() < 1) {
		PWARN("Failed to create peak buffers");
	}
}


/******** PEAK BUFFER CLASS **********/
/************************************/

PeakBuffer::PeakBuffer()
{
	bufferSize = microBufferSize = 0;
	upperHalfBuffer = lowerHalfBuffer = 0;
	microViewBuffer = 0;
}

PeakBuffer::~PeakBuffer()
{
	PENTERDES3;
	if (upperHalfBuffer) {
		delete [] upperHalfBuffer;
	}

	if (lowerHalfBuffer)
		delete [] lowerHalfBuffer;

	if (microViewBuffer)
		delete [] microViewBuffer;
}

void PeakBuffer::set_size(nframes_t size)
{
	// It could be possible due roundoff errors that size = 0;
	// This is of course bad, a PeakBuffer should at least have size 1!!
	if (size == 0)
		size = 1;

	if (size > bufferSize) {
		if (upperHalfBuffer)
			delete [] upperHalfBuffer;
		if (lowerHalfBuffer)
			delete [] lowerHalfBuffer;
		upperHalfBuffer = new unsigned char[size];
		lowerHalfBuffer = new unsigned char[size];
		bufferSize = size;
	}
}

void PeakBuffer::set_microview_buffer_size(nframes_t size)
{
	if (size > microBufferSize) {
		if (microViewBuffer)
			delete [] microViewBuffer;
		microViewBuffer = new short[size];
		microBufferSize = size;
	}
}

void PeakBuffer::set_recording_size(nframes_t size)
{
	bufferSize = size;
}

//eof
