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

#include "AbstractAudioReader.h" // Needed for DecodeBuffer declaration
#include "ReadSource.h"
#include "ResourcesManager.h"
#include "defines.h"
#include "Mixer.h"
#include <QFileInfo>
#include <QDateTime>
#include <QMutexLocker>

#include "Debugger.h"

/* Store for each zoomStep the upper and lower sample to hard disk as a
* peak_data_t. The top-top resolution is then 512 pixels, which should do
* Painting the waveform will be as simple as painting a line starting from the
* lower value to the upper value.
*/

#define NORMALIZE_CHUNK_SIZE	10000
#define PEAKFILE_MAJOR_VERSION	1
#define PEAKFILE_MINOR_VERSION	0

int Peak::zoomStep[] = {
	// non-cached zoomlevels.
	1, 2, 4, 8, 16, 32,
 	// Cached zoomlevels
 	64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576 
};

Peak::Peak(AudioSource* source)
{
	PENTERCONS;
	
	m_peaksAvailable = m_permanentFailure = m_interuptPeakBuild = false;
	
	QString sourcename = source->get_name();
	QString path;
	Project* project = pm().get_project();
	if (project) {
		path = project->get_root_dir() + "/peakfiles/";
	} else {
		path = source->get_dir();
		path = path.replace("audiosources", "peakfiles");
	}
	
	for (uint chan = 0; chan < source->get_channel_count(); ++ chan) {
		ChannelData* data = new Peak::ChannelData;
		
		data->fileName = sourcename + "-ch" + QByteArray::number(chan) + ".peak";
		data->fileName.prepend(path);
		data->pd = 0;
		
		m_channelData.append(data);
	}
	
	ReadSource* rs = qobject_cast<ReadSource*>(source);
	
	if (rs) {
		// This Peak object was created by AudioClip, meant for reading peak data
		m_source = resources_manager()->get_readsource(rs->get_id());
		m_source->set_output_rate(44100, true);
	} else {
		// No ReadSource object? Then it's created by WriteSource for on the fly
		// peak data creation, no m_source needed!
		m_source = 0;
	}
}

Peak::~Peak()
{
	PENTERDES;
	
	if (m_source) {
		delete m_source;
	}
	
	foreach(ChannelData* data, m_channelData) {
		if (data->normFile.isOpen()) {
			QFile::remove(data->normFileName);
		}
#if QT_VERSION < 0x040400
#if defined Q_WS_X11 || Q_WS_MAC
		if (data->memory) {
			uchar *start = data->memory - data->maps[data->memory].first;
			int len = data->maps[data->memory].second;
			if (-1 == munmap(start, len)) {
			}
			data->maps.remove(data->memory);
		}
#endif
#endif
		delete data;
	}
}

void Peak::close()
{
	pp().free_peak(this);
}

int Peak::read_header()
{
	PENTER;
	
	Q_ASSERT(m_source);
	
	foreach(ChannelData* data, m_channelData) {
		
		data->file.setFileName(data->fileName.toUtf8().data());
		
		if (! data->file.open(QIODevice::ReadOnly)) {
			PERROR("Couldn't open peak file for reading! (%s)", data->fileName.toAscii().data());
			return -1;
		}

#if QT_VERSION >= 0x040400
		data->memory = data->file.map(0, data->file.size());
		if (data->memory) {
			PMESG3("Peak:: sucessfully mapped data into memory (%s)\n", QS_C(data->fileName));
		}
#else if defined Q_WS_X11 || Q_WS_MAC
		int offset = 0;
		int size = data->file.size();
		int pagesSize = getpagesize();
		int realOffset = offset / pagesSize;
		int extra = offset % pagesSize;

		void *mapAddress = mmap((void*)0, (size_t)size + extra,
					 PROT_READ, MAP_SHARED, data->file.handle(), realOffset * pagesSize);
		if (MAP_FAILED != mapAddress) {
			uchar *address = extra + static_cast<uchar*>(mapAddress);
			data->memory = address;
			data->maps[address] = QPair<int,int>(extra, size);
		}
#endif
		
		QFileInfo file(m_source->get_filename());
		QFileInfo peakFile(data->fileName);
		
		QDateTime fileModTime = file.lastModified();
		QDateTime peakModTime = peakFile.lastModified();
		
		if (fileModTime > peakModTime) {
			PERROR("Source and Peak file modification time do not match");
			printf("SourceFile modification time is %s\n", fileModTime.toString().toAscii().data());
			printf("PeakFile modification time is %s\n", peakModTime.toString().toAscii().data());
			return -1;
		}
		
		
		data->file.seek(0);
	
		data->file.read(data->headerdata.label, sizeof(data->headerdata.label));
		data->file.read((char*)data->headerdata.version, sizeof(data->headerdata.version));
	
		if (	(data->headerdata.label[0]!='T') ||
			(data->headerdata.label[1]!='R') ||
			(data->headerdata.label[2]!='A') ||
			(data->headerdata.label[3]!='V') ||
			(data->headerdata.label[4]!='P') ||
			(data->headerdata.label[5]!='F') ||
			(data->headerdata.version[0] != PEAKFILE_MAJOR_VERSION) ||
			(data->headerdata.version[1] != PEAKFILE_MINOR_VERSION)) {
				printf("This file either isn't a Traverso Peak file, or the version doesn't match!\n");
				data->file.close();
				return -1;
		}
		
		data->file.read((char*)data->headerdata.peakDataLevelOffsets, sizeof(data->headerdata.peakDataLevelOffsets));
		data->file.read((char*)data->headerdata.peakDataSizeForLevel, sizeof(data->headerdata.peakDataSizeForLevel));
		data->file.read((char*)&data->headerdata.normValuesDataOffset, sizeof(data->headerdata.normValuesDataOffset));
		data->file.read((char*)&data->headerdata.peakDataOffset, sizeof(data->headerdata.peakDataOffset));
		
		data->peakreader = new PeakDataReader(data);
		data->peakdataDecodeBuffer = new DecodeBuffer;
	}
	
	m_peaksAvailable = true;
		
	return 1;
}

int Peak::write_header(ChannelData* data)
{
	PENTER;
	
	data->file.seek(0);

	data->headerdata.label[0] = 'T';
	data->headerdata.label[1] = 'R';
	data->headerdata.label[2] = 'A';
	data->headerdata.label[3] = 'V';
	data->headerdata.label[4] = 'P';
	data->headerdata.label[5] = 'F';
	data->headerdata.version[0] = PEAKFILE_MAJOR_VERSION;
	data->headerdata.version[1] = PEAKFILE_MINOR_VERSION;
	
	data->file.write((char*)data->headerdata.label, sizeof(data->headerdata.label));
	data->file.write((char*)data->headerdata.version, sizeof(data->headerdata.version));
	data->file.write((char*)data->headerdata.peakDataLevelOffsets, sizeof(data->headerdata.peakDataLevelOffsets));
	data->file.write((char*)data->headerdata.peakDataSizeForLevel, sizeof(data->headerdata.peakDataSizeForLevel));
	data->file.write((char*) &data->headerdata.normValuesDataOffset, sizeof(data->headerdata.normValuesDataOffset));
	data->file.write((char*) &data->headerdata.peakDataOffset, sizeof(data->headerdata.peakDataOffset));
	
	return 1;
}


void Peak::start_peak_loading()
{
	pp().queue_task(this);
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


int Peak::calculate_peaks(int chan, float** buffer, int zoomLevel, TimeRef startlocation, int pixelcount)
{
	PENTER3;
	
	if (m_permanentFailure) {
		return PERMANENT_FAILURE;
	}
	
	if(!m_peaksAvailable) {
		if (read_header() < 0) {
			return NO_PEAK_FILE;
		}
	}
	
	if (pixelcount <= 0) {
		return NO_PEAKDATA_FOUND;
	}
	
	ChannelData* data = m_channelData.at(chan);
	
// #define profile

#if defined (profile)
	trav_time_t starttime = get_microseconds();
#endif
	
	// Macro view mode
	if ( zoomLevel > MAX_ZOOM_USING_SOURCEFILE) {
		nframes_t startPos = startlocation.to_frame(44100);
		
		int offset = (startPos / zoomStep[zoomLevel]) * 2;
		
		// Check if this zoom level has as many data as requested.
		if ( (pixelcount + offset) > data->headerdata.peakDataSizeForLevel[zoomLevel - SAVING_ZOOM_FACTOR]) {
			// YES we know that sometimes we ommit the very last 'pixel' to avoid painting artifacts...
//  			PERROR("pixelcount exceeds available data size! (pixelcount is: %d, available is %d", pixelcount, data->headerdata.peakDataSizeForLevel[zoomLevel - SAVING_ZOOM_FACTOR] - offset); 
// 			pixelcount = data->headerdata.peakDataSizeForLevel[zoomLevel - SAVING_ZOOM_FACTOR] - offset;
		}
		
		nframes_t readposition = data->headerdata.peakDataLevelOffsets[zoomLevel - SAVING_ZOOM_FACTOR] + offset;
		int read = data->peakreader->read_from(data->peakdataDecodeBuffer, readposition, pixelcount);
		
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
		
		*buffer = data->peakdataDecodeBuffer->destination[0];

		return read;
		
	// Micro view mode
	} else {
		// Calculate the amount of frames to be read
		nframes_t toRead = pixelcount * zoomStep[zoomLevel];
		
		nframes_t readFrames = m_source->file_read(data->peakdataDecodeBuffer, startlocation, toRead);

		if (readFrames == 0) {
			return NO_PEAKDATA_FOUND;
		}
		
		if ( readFrames != toRead) {
			PWARN("Unable to read nframes %d (only %d available)", toRead, readFrames);
			pixelcount = readFrames / zoomStep[zoomLevel];
		}

		int count = 0;
		nframes_t pos = 0;
		audio_sample_t valueMax, valueMin, sample;
		
		// MicroView needs a buffer to store the calculated peakdata
		// our decodebuffer's readbuffer is large enough for this purpose
		// and it's no problem to use it at this point in the process chain.
		float* peakdata = data->peakdataDecodeBuffer->readBuffer;

		do {
			valueMax = valueMin = 0;

			for(int i=0; i < zoomStep[zoomLevel]; i++) {
				Q_ASSERT(pos <= readFrames);
				sample = data->peakdataDecodeBuffer->destination[chan][pos];
				if (sample > valueMax)
					valueMax = sample;
				if (sample < valueMin)
					valueMin = sample;
				pos++;
			}

			if (valueMax > fabs(valueMin)) {
				peakdata[count] = valueMax;
			} else {
				peakdata[count] = valueMin;
			}
			
			count++;
		
		} while(count < pixelcount);


#if defined (profile)
		int processtime = (int) (get_microseconds() - starttime);
		printf("Process time: %d useconds\n\n", processtime);
#endif
		
		// Assign the supplied buffer to the 'real' peakdata buffer.
		*buffer = peakdata;
		
		return count;
	}

	
	return 1;
}


int Peak::prepare_processing(int rate)
{
	PENTER;
	
	foreach(ChannelData* data, m_channelData) {
		
		data->normFileName = data->fileName;
		data->normFileName.append(".norm");
		
		// Create read/write enabled file
		data->file.setFileName(data->fileName);
		
		if (! data->file.open(QIODevice::ReadWrite)) {
			PWARN("Couldn't open peak file for writing! (%s)", data->fileName.toAscii().data());
			m_permanentFailure  = true;
			return -1;
		}
		
		// Create the temporary normalization data file
		data->normFile.setFileName(data->normFileName);
		
		if (! data->normFile.open(QIODevice::ReadWrite)) {
			PWARN("Couldn't open normalization data file for writing! (%s)", data->normFileName.toAscii().data());
			m_permanentFailure  = true;
			return -1;
		}
		
		// We need to know the peakDataOffset.
		data->headerdata.peakDataOffset = 
					sizeof(data->headerdata.label) + 
					sizeof(data->headerdata.version) + 
					sizeof(data->headerdata.peakDataLevelOffsets) + 
					sizeof(data->headerdata.peakDataSizeForLevel) +
					sizeof(data->headerdata.normValuesDataOffset) + 
					sizeof(data->headerdata.peakDataOffset);
					
		// Now seek to the start position, so we can write the peakdata to it in the process function
		data->file.seek(data->headerdata.peakDataOffset);
		
		data->pd = new Peak::ProcessData;
		data->pd->stepSize = TimeRef(1, rate);
	}
	
	
	return 1;
}


int Peak::finish_processing()
{
	PENTER;
	
	foreach(ChannelData* data, m_channelData) {
		
		if (data->pd->processLocation < data->pd->nextDataPointLocation) {
			peak_data_t peakvalue = (peak_data_t)(data->pd->peakUpperValue * MAX_DB_VALUE);
			data->file.write((char*)&peakvalue, sizeof(peak_data_t));
			peakvalue = (peak_data_t)(-1 * data->pd->peakLowerValue * MAX_DB_VALUE);
			data->file.write((char*)&peakvalue, sizeof(peak_data_t));
			data->pd->processBufferSize += 2;
		}
		
		int totalBufferSize = 0;
		
		data->headerdata.peakDataSizeForLevel[0] = data->pd->processBufferSize;
		totalBufferSize += data->pd->processBufferSize;
		
		for( int i = SAVING_ZOOM_FACTOR + 1; i < ZOOM_LEVELS+1; ++i) {
			data->headerdata.peakDataSizeForLevel[i - SAVING_ZOOM_FACTOR] = data->headerdata.peakDataSizeForLevel[i - SAVING_ZOOM_FACTOR - 1] / 2;
			totalBufferSize += data->headerdata.peakDataSizeForLevel[i - SAVING_ZOOM_FACTOR];
		}
		
		
		data->file.seek(data->headerdata.peakDataOffset);
		
		// The routine below uses a different total buffer size calculation
		// which might end up with a size >= totalbufferSize !!!
		// Need to look into that, for now + 2 seems to work...
		peak_data_t* saveBuffer = new peak_data_t[totalBufferSize + 1*sizeof(peak_data_t)];
		
		int read = data->file.read((char*)saveBuffer, sizeof(peak_data_t) * data->pd->processBufferSize) / sizeof(peak_data_t);
		
		if (read != data->pd->processBufferSize) {
			PERROR("couldn't read in all saved data?? (%d read)", read);
		}
		
		
		int prevLevelBufferPos = 0;
		int nextLevelBufferPos;
		data->headerdata.peakDataSizeForLevel[0] = data->pd->processBufferSize;
		data->headerdata.peakDataLevelOffsets[0] = data->headerdata.peakDataOffset;
		
		for (int i = SAVING_ZOOM_FACTOR+1; i < ZOOM_LEVELS+1; ++i) {
		
			int prevLevelSize = data->headerdata.peakDataSizeForLevel[i - SAVING_ZOOM_FACTOR - 1];
			data->headerdata.peakDataLevelOffsets[i - SAVING_ZOOM_FACTOR] = data->headerdata.peakDataLevelOffsets[i - SAVING_ZOOM_FACTOR - 1] + prevLevelSize;
			prevLevelBufferPos = data->headerdata.peakDataLevelOffsets[i - SAVING_ZOOM_FACTOR - 1] - data->headerdata.peakDataOffset;
			nextLevelBufferPos = data->headerdata.peakDataLevelOffsets[i - SAVING_ZOOM_FACTOR] - data->headerdata.peakDataOffset;
			
			
			int count = 0;
			
			do {
				Q_ASSERT(nextLevelBufferPos <= totalBufferSize);
				saveBuffer[nextLevelBufferPos] = (peak_data_t) f_max(saveBuffer[prevLevelBufferPos], saveBuffer[prevLevelBufferPos + 2]);
				saveBuffer[nextLevelBufferPos + 1] = (peak_data_t) f_max(saveBuffer[prevLevelBufferPos + 1], saveBuffer[prevLevelBufferPos + 3]);
				nextLevelBufferPos += 2;
				prevLevelBufferPos += 4;
				count+=4;
			}
			while (count < prevLevelSize);
		}
		
		data->file.seek(data->headerdata.peakDataOffset);
		
		int written = data->file.write((char*)saveBuffer, sizeof(peak_data_t) * totalBufferSize) / sizeof(peak_data_t);
		
		if (written != totalBufferSize) {
			PERROR("could not write complete buffer! (only %d)", written);
	// 		return -1;
		}
		
		data->normFile.seek(0);
		
		read = data->normFile.read((char*)saveBuffer, sizeof(audio_sample_t) * data->pd->normDataCount) / sizeof(audio_sample_t);
		
		if (read != data->pd->normDataCount) {
			PERROR("Could not read in all (%d) norm. data, only %d", data->pd->normDataCount, read);
		}
		
		data->headerdata.normValuesDataOffset = data->headerdata.peakDataOffset + totalBufferSize;
		
		data->normFile.close();
		
		if (!QFile::remove(data->normFileName)) {
			PERROR("Failed to remove temp. norm. data file! (%s)", data->normFileName.toAscii().data()); 
		}
		
		written = data->file.write((char*)saveBuffer, sizeof(audio_sample_t) * read) / sizeof(audio_sample_t);
		
		write_header(data);
		
		data->file.close();
		
		delete [] saveBuffer;
		delete data->pd;
		data->pd = 0;
		
	}
	
	emit finished();
	
	return 1;
	
}


void Peak::process(uint channel, audio_sample_t* buffer, nframes_t nframes)
{
	ChannelData* data = m_channelData.at(channel);
	ProcessData* pd = data->pd;

	for (uint i=0; i < nframes; i++) {
		
		pd->processLocation += pd->stepSize;
		
		audio_sample_t sample = buffer[i];
		
		pd->normValue = f_max(pd->normValue, fabsf(sample));
		
		if (sample > pd->peakUpperValue) {
			pd->peakUpperValue = sample;
		}
		
		if (sample < pd->peakLowerValue) {
			pd->peakLowerValue = sample;
		}
		
		if (pd->processLocation >= pd->nextDataPointLocation) {
		
			peak_data_t peakbuffer[2];

			peakbuffer[0] = (peak_data_t) (pd->peakUpperValue * MAX_DB_VALUE );
			peakbuffer[1] = (peak_data_t) ((-1) * (pd->peakLowerValue * MAX_DB_VALUE ));
			
			int written = data->file.write((char*)peakbuffer, sizeof(peak_data_t) * 2) / sizeof(peak_data_t);
			
			if (written != 2) {
				PWARN("couldnt write peak data, only (%d)", written);
			}

			pd->peakUpperValue = 0.0;
			pd->peakLowerValue = 0.0;
			
			pd->processBufferSize+=2;
			pd->nextDataPointLocation += pd->processRange;
		}
		
		if (pd->normProcessedFrames == NORMALIZE_CHUNK_SIZE) {
			int written = data->normFile.write((char*)&pd->normValue, sizeof(audio_sample_t)) / sizeof(audio_sample_t);
			
			if (written != 1) {
				PWARN("couldnt write norm data, only (%d)", written);
			}
 
			pd->normValue = 0.0;
			pd->normProcessedFrames = 0;
			pd->normDataCount++;
		}
		
		pd->normProcessedFrames++;
	}
}


int Peak::create_from_scratch()
{
	PENTER;
	
#define profile

#if defined (profile)
	trav_time_t starttime = get_microseconds();
#endif
	int ret = -1;
	
	if (prepare_processing(m_source->get_file_rate()) < 0) {
		return ret;
	}
	
	nframes_t readFrames = 0;
	nframes_t totalReadFrames = 0;

	nframes_t bufferSize = 65536;

	int progression = 0;

	if (m_source->get_length() == 0) {
		qWarning("Peak::create_from_scratch() : m_source (%s) has length 0", m_source->get_name().toAscii().data());
		return ret;
	}

	if (m_source->get_nframes() < bufferSize) {
		bufferSize = 64;
		if (m_source->get_nframes() < bufferSize) {
			qDebug("source length is too short to display one pixel of the audio wave form in macro view");
			return ret;
		}
	}

	m_source->set_output_rate(m_source->get_file_rate(), true);
	
	DecodeBuffer decodebuffer;
	
	do {
		if (m_interuptPeakBuild) {
			ret = -1;
			goto out;
		}
		
		readFrames = m_source->file_read(&decodebuffer, totalReadFrames, bufferSize);
		
		if (readFrames <= 0) {
			PERROR("readFrames < 0 during peak building");
			break;
		}
		
		for (uint chan = 0; chan < m_source->get_channel_count(); ++ chan) {
			process(chan, decodebuffer.destination[chan], readFrames);
		}
		
		totalReadFrames += readFrames;
		progression = (int) ((float)totalReadFrames / ((float)m_source->get_nframes() / 100.0));
		
		ChannelData* data = m_channelData.at(0);
		
		if ( progression > data->pd->progress) {
			emit progress(progression);
			data->pd->progress = progression;
		}
		
	} while (totalReadFrames < m_source->get_nframes());


	if (finish_processing() < 0) {
		ret = -1;
		goto out;
	}
	
	ret = 1;
	
out:
	 
#if defined (profile)
	long processtime = (long) (get_microseconds() - starttime);
	printf("Process time: %d seconds\n\n", (int)(processtime/1000));
#endif
	
	m_source->set_output_rate(44100, true);

	return ret;
}


audio_sample_t Peak::get_max_amplitude(TimeRef startlocation, TimeRef endlocation)
{
	foreach(ChannelData* data, m_channelData) {
		if (!data->file.isOpen() || !m_peaksAvailable) {
			return 0.0f;
		}
	}
	int rate = m_source->get_file_rate();
	m_source->set_output_rate(rate, true);
	nframes_t startframe = startlocation.to_frame(rate);
	nframes_t endframe = endlocation.to_frame(rate);
	int startpos = startframe / NORMALIZE_CHUNK_SIZE;
	uint count = (endframe / NORMALIZE_CHUNK_SIZE) - startpos;
	
	uint buffersize = count < NORMALIZE_CHUNK_SIZE*2 ? NORMALIZE_CHUNK_SIZE*2 : count;
	audio_sample_t* readbuffer =  new audio_sample_t[buffersize];
	
	audio_sample_t maxamp = 0;
	DecodeBuffer decodebuffer;
	// Read in the part not fully occupied by a cached normalize value
	// at the left hand part and run compute_peak on it.
	if (startframe != 0) {
		startpos += 1;
		int toRead = (int) ((startpos * NORMALIZE_CHUNK_SIZE) - startframe);
		
		int read = m_source->file_read(&decodebuffer, startframe, toRead);
		
		for (uint chan = 0; chan < m_source->get_channel_count(); ++ chan) {
			maxamp = Mixer::compute_peak(decodebuffer.destination[chan], read, maxamp);
		}
	}
	
	
	// Read in the part not fully occupied by a cached normalize value
	// at the right hand part and run compute_peak on it.
	float f = (float) endframe / NORMALIZE_CHUNK_SIZE;
	int endpos = (int) f;
	int toRead = (int) ((f - (endframe / NORMALIZE_CHUNK_SIZE)) * NORMALIZE_CHUNK_SIZE);
	int read = m_source->file_read(&decodebuffer, endframe - toRead, toRead);
	
	if (read > 0) {
		for (uint chan = 0; chan < m_source->get_channel_count(); ++ chan) {
			maxamp = Mixer::compute_peak(decodebuffer.destination[chan], read, maxamp);
		}
	}
	
	// Now that we have covered both boundary situations,
	// read in the cached normvalues, and calculate the highest value!
	count = endpos - startpos;
	
	foreach(ChannelData* data, m_channelData) {
		data->file.seek(data->headerdata.normValuesDataOffset + (startpos * sizeof(audio_sample_t)));
	
		int read = data->file.read((char*)readbuffer, sizeof(audio_sample_t) * count) / sizeof(audio_sample_t);
	
		if (read != (int)count) {
			printf("could only read %d, %d requested\n", read, count);
		}
	
		maxamp = Mixer::compute_peak(readbuffer, read, maxamp);
	}
	
	delete [] readbuffer;
	
	m_source->set_output_rate(44100, true);
	
	return maxamp;
}




/******** PEAK BUILD THREAD CLASS **********/
/******************************************/

PeakProcessor& pp()
{
	static PeakProcessor processor;
	return processor;
}


PeakProcessor::PeakProcessor()
{
	m_ppthread = new PPThread(this);
	m_taskRunning = false;
	m_runningPeak = 0;

	moveToThread(m_ppthread);
	
	m_ppthread->start();
	
	connect(this, SIGNAL(newTask()), this, SLOT(start_task()), Qt::QueuedConnection);
}


PeakProcessor::~ PeakProcessor()
{
	m_ppthread->exit(0);
	
	if (!m_ppthread->wait(1000)) {
		m_ppthread->terminate();
	}
	
	delete m_ppthread;
}


void PeakProcessor::start_task()
{
	m_runningPeak->create_from_scratch();
	
	QMutexLocker locker(&m_mutex);
	
	m_taskRunning = false;
	
	if (m_runningPeak->m_interuptPeakBuild) {
		PMESG("PeakProcessor:: Deleting interrupted Peak!");
		delete m_runningPeak;
		m_runningPeak = 0;
		m_wait.wakeAll();
		return;
	}
	
	foreach(Peak* peak, m_queue) {
		if (m_runningPeak->m_source->get_filename() == peak->m_source->get_filename()) {
			m_queue.removeAll(peak);
			emit peak->finished();
		}
	}
	
	m_runningPeak = 0;
	
	dequeue_queue();
}

void PeakProcessor::queue_task(Peak * peak)
{
	QMutexLocker locker(&m_mutex);
	
	m_queue.enqueue(peak);
	
	if (!m_taskRunning) {
		dequeue_queue();
	}
}

void PeakProcessor::dequeue_queue()
{
	if (!m_queue.isEmpty()) {
		m_taskRunning = true;
		m_runningPeak = m_queue.dequeue();
		emit newTask();
	}
}

void PeakProcessor::free_peak(Peak * peak)
{
	m_mutex.lock();
	
	m_queue.removeAll(peak);
	
	if (peak == m_runningPeak) {
		PMESG("PeakProcessor:: Interrupting running build process!");
		peak->m_interuptPeakBuild =  true;
		
		PMESG("PeakProcessor:: Waiting GUI thread until interrupt finished");
		m_wait.wait(&m_mutex);
		PMESG("PeakProcessor:: Resuming GUI thread");
		
		dequeue_queue();
		
		m_mutex.unlock();
		
		return;
	}
	
	m_mutex.unlock();
	
	delete peak;
}


PPThread::PPThread(PeakProcessor * pp)
{
	m_pp = pp;
}

void PPThread::run()
{
	exec();
}



PeakDataReader::PeakDataReader(Peak::ChannelData* data)
{
	m_d = data;
	m_nframes = m_d->file.size();
}


nframes_t PeakDataReader::read_from(DecodeBuffer* buffer, nframes_t start, nframes_t count)
{
// 	printf("read_from:: before_seek from %d, framepos is %d\n", start, m_readPos);
	
	if (!seek(start)) {
		return 0;
	}
	
	return read(buffer, count);
}


bool PeakDataReader::seek(nframes_t start)
{
	if (m_readPos != start) {
		Q_ASSERT(m_d->file.isOpen());
	
	
		if (start >= m_nframes) {
			return false;
		}
	
		// only seek if we didn't mmap 
		if (!m_d->memory) {
			if (!m_d->file.seek(start)) {
				PERROR("PeakDataReader: could not seek to data point %d within %s", start, m_d->fileName.toUtf8().data());
				return false;
			}
		}
		
		m_readPos = start;
	}
	
	return true;
}


nframes_t PeakDataReader::read(DecodeBuffer* buffer, nframes_t count)
{
	if ( ! (count && (m_readPos < m_nframes)) ) {
		return 0;
	}
		
	// Make sure the read buffer is big enough for this read
	buffer->check_buffers_capacity(count*2, 1);
	
	Q_ASSERT(m_d->file.isOpen());
	
	int framesRead = 0;
	peak_data_t* readbuffer;
	
	if (m_d->memory) {
// 		printf("using memory mapped read\n");
		readbuffer = (peak_data_t*)(m_d->memory + m_readPos*sizeof(peak_data_t));
		framesRead = count;
	} else {
		framesRead = m_d->file.read((char*)buffer->readBuffer, sizeof(peak_data_t) * count) / sizeof(peak_data_t);
		readbuffer = (peak_data_t*)(buffer->readBuffer);
	}

	for (int f = 0; f < framesRead; f++) {
		buffer->destination[0][f] = float(readbuffer[f]);
	}

	m_readPos += framesRead;
	
	return framesRead;
}
