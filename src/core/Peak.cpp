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

#include "libtraversocore.h"

#include "Peak.h"
#include "ReadSource.h"
#include "defines.h"
#include "Mixer.h"
#include <QFileInfo>
#include <QDateTime>
#include <QMutexLocker>

#include "Debugger.h"

/* Store for each zoomStep the upper and lower sample to hard disk as a
* unsigned char. The top-top resolution is then 512 pixels, which should do
* Painting the waveform will be as simple as painting a line starting from the
* lower value to the upper value.
*/

const int Peak::MAX_DB_VALUE			= 120;
const int SAVING_ZOOM_FACTOR 			= 6;
const int Peak::MAX_ZOOM_USING_SOURCEFILE	= SAVING_ZOOM_FACTOR - 1;

#define NORMALIZE_CHUNK_SIZE	10000
#define PEAKFILE_MAJOR_VERSION	0
#define PEAKFILE_MINOR_VERSION	8

int Peak::zoomStep[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
				8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576};

Peak::Peak(AudioSource* source, int channel)
	: m_channel(channel)
{
	PENTERCONS;
	
	ReadSource* rs = qobject_cast<ReadSource*>(source);
	if (rs) {
		m_source = rs;
	}
	 
	if (source->get_channel_count() > 1) {
		PMESG("Peak channel count is %d", source->get_channel_count());
		m_fileName = pm().get_project()->get_root_dir() + "/peakfiles/" + source->get_name() + "-ch" + QByteArray::number(m_channel) + ".peak";
	} else {
		m_fileName = pm().get_project()->get_root_dir() + "/peakfiles/" + source->get_name() + ".peak";
	}
	
	peaksAvailable = permanentFailure = interuptPeakBuild = false;
	peakBuildThread = 0;
	m_file = m_normFile = 0;
}

Peak::~Peak()
{
	PENTERDES;
	
	if (peakBuildThread) {
		if (peakBuildThread->isRunning()) {
			interuptPeakBuild = true;
			peakBuildThread->wait();
		}
		
		delete peakBuildThread;
	}

	if (m_file) {
		fclose(m_file);
	}
	
	if (m_normFile) {
		fclose(m_normFile);
		remove(m_normFileName.toAscii().data());
	}
}


int Peak::read_header()
{
	PENTER;
	
	Q_ASSERT(m_source);
	
	m_file = fopen(m_fileName.toUtf8().data(),"rb");
	
	if (! m_file) {
		PERROR("Couldn't open peak file for reading! (%s)", m_fileName.toAscii().data());
		return -1;
	}
	
	QFileInfo file(m_source->get_filename());
	QFileInfo peakFile(m_fileName);
	
	QDateTime fileModTime = file.lastModified();
	QDateTime peakModTime = peakFile.lastModified();
	
	if (fileModTime > peakModTime) {
		PERROR("Source and Peak file modification time do not match");
		printf("SourceFile modification time is %s\n", fileModTime.toString().toAscii().data());
		printf("PeakFile modification time is %s\n", peakModTime.toString().toAscii().data());
		return -1;
	}
	
	
	fseek(m_file, 0, SEEK_SET);

	fread(m_data.label, sizeof(m_data.label), 1, m_file);
	fread(m_data.version, sizeof(m_data.version), 1, m_file);

	if ((m_data.label[0]!='T') ||
		(m_data.label[1]!='R') ||
		(m_data.label[2]!='A') ||
		(m_data.label[3]!='V') ||
		(m_data.label[4]!='P') ||
		(m_data.label[5]!='F') ||
		(m_data.version[0] != PEAKFILE_MAJOR_VERSION) ||
		(m_data.version[1] != PEAKFILE_MINOR_VERSION)) {
			printf("This file either isn't a Traverso Peak file, or the version doesn't match!\n");
			fclose(m_file);
			m_file = 0;
			return -1;
	}
	
	fread(m_data.peakDataLevelOffsets, sizeof(m_data.peakDataLevelOffsets), 1, m_file);
	fread(m_data.peakDataSizeForLevel, sizeof(m_data.peakDataSizeForLevel), 1, m_file);
	fread(&m_data.normValuesDataOffset, sizeof(m_data.normValuesDataOffset), 1, m_file);
	fread(&m_data.peakDataOffset, sizeof(m_data.peakDataOffset), 1, m_file);

	peaksAvailable = true;
	
	return 1;
}

int Peak::write_header()
{
	PENTER;
	
	fseek (m_file, 0, SEEK_SET);
	
	m_data.label[0] = 'T';
	m_data.label[1] = 'R';
	m_data.label[2] = 'A';
	m_data.label[3] = 'V';
	m_data.label[4] = 'P';
	m_data.label[5] = 'F';
	m_data.version[0] = PEAKFILE_MAJOR_VERSION;
	m_data.version[1] = PEAKFILE_MINOR_VERSION;
	
	fwrite(m_data.label, sizeof(m_data.label), 1, m_file);
	fwrite(m_data.version, sizeof(m_data.version), 1, m_file);
	fwrite(m_data.peakDataLevelOffsets, sizeof(m_data.peakDataLevelOffsets), 1, m_file);
	fwrite(m_data.peakDataSizeForLevel, sizeof(m_data.peakDataSizeForLevel), 1, m_file);
	fwrite((void*) &m_data.normValuesDataOffset, sizeof(m_data.normValuesDataOffset), 1, m_file);
	fwrite((void*) &m_data.peakDataOffset, sizeof(m_data.peakDataOffset), 1, m_file);
	
	return 1;
}


void Peak::start_peak_loading()
{
	peakbuilder().queue_task(this);
}


static int
cnt_bits(unsigned long val, int & highbit)
{
	int cnt = 0;
	highbit = 0;
	while (val) {
		if (val & 1) cnt++;
		val>>=1;
		highbit++;
	}
	return cnt;
}

// returns the next power of two greater or equal to val
static unsigned long
nearest_power_of_two(unsigned long val)
{
	int highbit;
	if (cnt_bits(val, highbit) > 1) {
		return 1<<highbit;
	}
	return val;
}


int Peak::calculate_peaks(void* buffer, int zoomLevel, nframes_t startPos, int pixelcount )
{
	PENTER3;
	if (permanentFailure) {
		return PERMANENT_FAILURE;
	}
	
	if(!peaksAvailable) {
		if (read_header() < 0) {
			return NO_PEAK_FILE;
		}
	}
	
// #define profile

#if defined (profile)
	trav_time_t starttime = get_microseconds();
#endif
	
	// Macro view mode
	if (false) {
		unsigned long val = nearest_power_of_two(256);
		printf("Using VAL %ld measurements\n", val);
		
		int zoomlevel = zoomStep[zoomLevel];
		
		int offset = (startPos / zoomStep[zoomLevel-1]) * 2;
		int nearestlevel = zoomlevel / 2;
		int toread = pixelcount * 2;
			
		unsigned char readbuffer[toread];
			
			// Seek to the correct position in the buffer on hard disk
		fseek(m_file, m_data.peakDataLevelOffsets[zoomLevel - 1 - SAVING_ZOOM_FACTOR] + offset, SEEK_SET);
			
		// Read in the pixelcount of peakdata
		int read = fread(readbuffer, sizeof(unsigned char), toread, m_file);
			
			
		float max = 0;
		int bufpos = 0;
		int totalnearestlevel = 0;
		int totalinterpolatedlevel = 0;
		uchar* bufpointer = (uchar*)buffer;
			
		for (int i=0; i<read; ++i) {
			max = f_max(max, readbuffer[i]);
			totalnearestlevel += nearestlevel;
				
			if ( (totalnearestlevel + zoomlevel/2) > (totalinterpolatedlevel + zoomlevel)) {
				bufpointer[bufpos++] = (uchar)max;
				max = 0;
				totalinterpolatedlevel += zoomlevel;
			}
		}
		
		return pixelcount;
	 
	} else if ( zoomLevel > MAX_ZOOM_USING_SOURCEFILE) {
		
		int offset = (startPos / zoomStep[zoomLevel]) * 2;
		
		// Check if this zoom level has as many data as requested.
		if ( (pixelcount + offset) > m_data.peakDataSizeForLevel[zoomLevel - SAVING_ZOOM_FACTOR]) {
// 			PERROR("pixelcount exceeds available data size! (pixelcount is: %d, available is %d", pixelcount, m_data.peakDataSizeForLevel[zoomLevel - SAVING_ZOOM_FACTOR] - offset); 
			pixelcount = m_data.peakDataSizeForLevel[zoomLevel - SAVING_ZOOM_FACTOR] - offset;
		}
		
		// Seek to the correct position in the buffer on hard disk
		fseek(m_file, m_data.peakDataLevelOffsets[zoomLevel - SAVING_ZOOM_FACTOR] + offset, SEEK_SET);
		
		// Read in the pixelcount of peakdata
		int read = fread(buffer, sizeof(unsigned char), pixelcount, m_file);
		
		if (read != pixelcount) {
			PERROR("Could not read in all peak data, pixelcount is %d, read count is %d", pixelcount, read);
		}
		
#if defined (profile)
		int processtime = (int) (get_microseconds() - starttime);
		printf("Process time: %d useconds\n\n", processtime);
#endif

		if (read == 0) {
			return NO_PEAKDATA_FOUND;
		}

		return read;
		
	// Micro view mode
	} else {
		nframes_t readFrames, toRead;
		toRead = pixelcount * zoomStep[zoomLevel];
		audio_sample_t buf[toRead];

		if ( (readFrames = m_source->file_read(m_channel, buf, startPos, toRead)) != toRead) {
			PWARN("Unable to read nframes %d (only %d available)", toRead, readFrames);
			if (readFrames == 0) {
				return NO_PEAKDATA_FOUND;
			}
			pixelcount = readFrames / zoomStep[zoomLevel];
		}

		int count = 0;
		nframes_t pos = 0;
		audio_sample_t valueMax, valueMin, sample;
		short* writeBuffer = (short*)buffer;

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
				writeBuffer[count] = (short)(valueMax * MAX_DB_VALUE);
			} else {
				writeBuffer[count] = (short)(valueMin * MAX_DB_VALUE);
			}
			
			count++;
		
		} while(count < pixelcount);


#if defined (profile)
		int processtime = (int) (get_microseconds() - starttime);
		printf("Process time: %d useconds\n\n", processtime);
#endif
		return count;
	}

	
	return 1;
}


int Peak::prepare_processing()
{
	PENTER;
	
	m_normFileName = m_fileName;
	m_normFileName.append(".norm");
	
	// Create read/write enabled file
	m_file = fopen(m_fileName.toUtf8().data(),"wb+");
	
	if (! m_file) {
		PWARN("Couldn't open peak file for writing! (%s)", m_fileName.toAscii().data());
		permanentFailure  = true;
		return -1;
	}
	
	// Create the temporary normalization data file
	m_normFile = fopen(m_normFileName.toUtf8().data(), "wb+");
	
	if (! m_normFile) {
		PWARN("Couldn't open normalization data file for writing! (%s)", m_normFileName.toAscii().data());
		permanentFailure  = true;
		return -1;
	}
	
	// We need to know the peakDataOffset.
	m_data.peakDataOffset = sizeof(m_data.label) + 
				sizeof(m_data.version) + 
				sizeof(m_data.peakDataLevelOffsets) + 
				sizeof(m_data.peakDataSizeForLevel) +
				sizeof(m_data.normValuesDataOffset) + 
				sizeof(m_data.peakDataOffset);
				
	// Now seek to the start position, so we can write the peakdata to it in the process function
	fseek(m_file, m_data.peakDataOffset, SEEK_SET);
	
	normValue = peakUpperValue = peakLowerValue = 0;
	processBufferSize = processedFrames = m_progress = normProcessedFrames = normDataCount = 0;
	
	return 1;
}


int Peak::finish_processing()
{
	PENTER;
	
	if (processedFrames != 64) { 
		fwrite(&peakUpperValue, 1, 1, m_file);
		fwrite(&peakLowerValue, 1, 1, m_file);
		processBufferSize += 2;
	}
	
	int totalBufferSize = 0;
	int dividingFactor = 2;
	
	m_data.peakDataSizeForLevel[0] = processBufferSize;
	totalBufferSize += processBufferSize;
	
	for( int i = SAVING_ZOOM_FACTOR + 1; i < ZOOM_LEVELS+1; ++i) {
		int size = processBufferSize / dividingFactor;
		m_data.peakDataSizeForLevel[i - SAVING_ZOOM_FACTOR] = size;
		totalBufferSize += size;
		dividingFactor *= 2;
	}
	
	
	fseek(m_file, m_data.peakDataOffset, SEEK_SET);
	
	// The routine below uses a different total buffer size calculation
	// which might end up with a size >= totalbufferSize !!!
	// Need to look into that, for now + 2 seems to work...
 	unsigned char* saveBuffer = new unsigned char[totalBufferSize + 2];
	
	int read = fread(saveBuffer, 1, processBufferSize, m_file);
	
	if (read != processBufferSize) {
		PERROR("couldn't read in all saved data?? (%d read)", read);
	}
	
	
	int prevLevelBufferPos = 0;
	int nextLevelBufferPos;
	m_data.peakDataSizeForLevel[0] = processBufferSize;
	m_data.peakDataLevelOffsets[0] = m_data.peakDataOffset;
	
	for (int i = SAVING_ZOOM_FACTOR+1; i < ZOOM_LEVELS+1; ++i) {
	
		int prevLevelSize = m_data.peakDataSizeForLevel[i - SAVING_ZOOM_FACTOR - 1];
		m_data.peakDataLevelOffsets[i - SAVING_ZOOM_FACTOR] = m_data.peakDataLevelOffsets[i - SAVING_ZOOM_FACTOR - 1] + prevLevelSize;
		prevLevelBufferPos = m_data.peakDataLevelOffsets[i - SAVING_ZOOM_FACTOR - 1] - m_data.peakDataOffset;
		nextLevelBufferPos = m_data.peakDataLevelOffsets[i - SAVING_ZOOM_FACTOR] - m_data.peakDataOffset;
		
		
		int count = 0;
		
		do {
			Q_ASSERT((nextLevelBufferPos + 1) < (totalBufferSize + 2));
			saveBuffer[nextLevelBufferPos] = (unsigned char) f_max(saveBuffer[prevLevelBufferPos], saveBuffer[prevLevelBufferPos + 2]);
			saveBuffer[nextLevelBufferPos + 1] = (unsigned char) f_max(saveBuffer[prevLevelBufferPos + 1], saveBuffer[prevLevelBufferPos + 3]);
			nextLevelBufferPos += 2;
			prevLevelBufferPos += 4;
			count+=4;
		}
		while (count < prevLevelSize);
	}
	
	fseek(m_file, m_data.peakDataOffset, SEEK_SET);
	
	int written = fwrite(saveBuffer, 1, totalBufferSize, m_file);
	
	if (written != totalBufferSize) {
		PERROR("could not write complete buffer! (only %d)", written);
// 		return -1;
	}
	
	fseek(m_normFile, 0, SEEK_SET);
	
	read = fread(saveBuffer, sizeof(audio_sample_t), normDataCount, m_normFile);
	
	if (read != normDataCount) {
		PERROR("Could not read in all (%d) norm. data, only %d", normDataCount, read);
	}
	
	m_data.normValuesDataOffset = m_data.peakDataOffset + totalBufferSize;
	
	fclose(m_normFile);
	m_normFile = NULL;
	
	if( remove(m_normFileName.toAscii().data()) != 0 ) {
		PERROR("Failed to remove temp. norm. data file! (%s)", m_normFileName.toAscii().data()); 
	}
	
	written = fwrite(saveBuffer, sizeof(audio_sample_t), read, m_file);
	
	write_header();
	
	fclose(m_file);
	m_file = 0;
	
	delete [] saveBuffer;
	
	emit finished(this);
	
	return 1;
	
}


void Peak::process(audio_sample_t* buffer, nframes_t nframes)
{
	for (uint i=0; i < nframes; i++) {
		
		audio_sample_t sample = buffer[i];
		
		if (sample > peakUpperValue) {
			peakUpperValue = sample;
		}
		
		if (sample < peakLowerValue) {
			peakLowerValue = sample;
		}
		
		normValue = f_max(normValue, fabsf(sample));
		
		if (processedFrames == 64) {
		
			unsigned char peakbuffer[2];

			peakbuffer[0] = (unsigned char) (peakUpperValue * MAX_DB_VALUE );
			peakbuffer[1] = (unsigned char) ((-1) * (peakLowerValue * MAX_DB_VALUE ));
			
			int written = fwrite(peakbuffer, sizeof(unsigned char), 2, m_file);
			
			if (written != 2) {
				PWARN("couldnt write data, only (%d)", written);
			}

			peakUpperValue = 0.0;
			peakLowerValue = 0.0;
			processedFrames = 0;
			
			processBufferSize+=2;
		}
		
		if (normProcessedFrames == NORMALIZE_CHUNK_SIZE) {
			int written = fwrite(&normValue, sizeof(audio_sample_t), 1, m_normFile);
			
			if (written != 1) {
				PWARN("couldnt write data, only (%d)", written);
			}

			normValue = 0.0;
			normProcessedFrames = 0;
			normDataCount++;
		}
		
		processedFrames++;
		normProcessedFrames++;
	}
}


int Peak::create_from_scratch()
{
	PENTER;
	
	if (prepare_processing() < 0) {
		return -1;
	}
	
	nframes_t readFrames = 0;
	nframes_t totalReadFrames = 0;

	#if defined (OSX_BUILD)
		nframes_t bufferSize = 4096;
	#else
		nframes_t bufferSize = 65536;
	#endif

	int cycles = m_source->get_nframes() / bufferSize;
	int counter = 0;
	int p = 0;
	audio_sample_t buf[bufferSize];

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
		if (interuptPeakBuild) {
			return -1;
		}
		
		readFrames = m_source->file_read(m_channel, buf, totalReadFrames, bufferSize);
		process(buf, readFrames);
		totalReadFrames += readFrames;
		counter++;
		p = (int) (counter*100) / cycles;
		
		if ( p > m_progress) {
			emit progress(p - m_progress);
			m_progress = p;
		}
		
	} while(totalReadFrames != m_source->get_nframes());


	if (finish_processing() < 0) {
		return -1;
	}
	 
	return 1;
}


audio_sample_t Peak::get_max_amplitude(nframes_t startframe, nframes_t endframe)
{
	Q_ASSERT(m_file);
	
	audio_sample_t maxamp = 0;
	int startpos = startframe / NORMALIZE_CHUNK_SIZE;
	
	// Read in the part not fully occupied by a cached normalize value
	// and run compute_peak on it.
	if (startframe != 0) {
		startpos += 1;
		int toRead = (int) ((startpos * NORMALIZE_CHUNK_SIZE) - startframe);
		
		audio_sample_t buf[toRead];
		int read = m_source->file_read(m_channel, buf, startframe, toRead);
		
		maxamp = Mixer::compute_peak(buf, read, maxamp);
	}
	
	int count = (endframe / NORMALIZE_CHUNK_SIZE) - startpos;
	
	
	// Read in the part not fully occupied by a cached normalize value
	// and run compute_peak on it.
	float f = (float) endframe / NORMALIZE_CHUNK_SIZE;
	int endpos = (int) f;
	int toRead = (int) ((f - (endframe / NORMALIZE_CHUNK_SIZE)) * NORMALIZE_CHUNK_SIZE);
	audio_sample_t buf[toRead];
	int read = m_source->file_read(m_channel, buf, endframe - toRead, toRead);
	maxamp = Mixer::compute_peak(buf, read, maxamp);
	
	
	// Now that we have covered both boundary situations,
	// read in the cached normvalues, and calculate the highest value!
	count = endpos - startpos;
	audio_sample_t buffer[count];
	
	fseek(m_file, m_data.normValuesDataOffset + (startpos * sizeof(audio_sample_t)), SEEK_SET);
	
	read = fread(buffer, sizeof(audio_sample_t), count, m_file);
	
	if (read != count) {
		printf("could only read %d, %d requested\n", read, count);
	}
	
	maxamp = Mixer::compute_peak(buffer, read, maxamp);
	
	return maxamp;
}




/******** PEAK BUILD THREAD CLASS **********/
/******************************************/

PeakBuildThread& peakbuilder()
{
	static PeakBuildThread peakthread;
	static long long count;
	if (!count++) {
		peakthread.moveToThread(&peakthread);
	}
	return peakthread;
}



PeakBuildThread::PeakBuildThread()
{
	m_runningTasks = 0;
	start();
	
	connect(this, SIGNAL(newTask(Peak*)), this, SLOT(start_task(Peak*)), Qt::QueuedConnection);
#ifndef Q_WS_MAC
// 	setStackSize(20000);
#endif
}

void PeakBuildThread::run()
{
	exec();
}

void PeakBuildThread::start_task(Peak* peak)
{
	peak->create_from_scratch();
	
	m_mutex.lock();
	m_runningTasks--;
	
	if (!m_queue.isEmpty()) {
		m_runningTasks++;
		Peak* peak = m_queue.dequeue();
		m_mutex.unlock();
		emit newTask(peak);
		return;
	}
	m_mutex.unlock();
}

void PeakBuildThread::queue_task(Peak * peak)
{
	m_mutex.lock();
	
	m_queue.enqueue(peak);
	
	if (!m_runningTasks) {
		m_runningTasks++;
		Peak* peak = m_queue.dequeue();
		m_mutex.unlock();
		emit newTask(peak);
		return;
	}
	
	m_mutex.unlock();
}

