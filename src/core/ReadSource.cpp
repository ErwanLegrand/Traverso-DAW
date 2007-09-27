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

#include "ReadSource.h"
#include "ResampleAudioReader.h"

#include "ProjectManager.h"
#include "Project.h"
#include "AudioClip.h"
#include "DiskIO.h"
#include "Utils.h"
#include "Song.h"
#include "AudioDevice.h"
#include <QFile>
#include "Config.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/**
 *	\class ReadSource
	\brief A class for (buffered) reading of audio files.
 */

// This constructor is called for existing (recorded/imported) audio sources
ReadSource::ReadSource(const QDomNode node)
	: AudioSource()
{
	
	set_state(node);
	
	private_init();
	
	Project* project = pm().get_project();
	
	// FIXME The check below no longer makes sense!!!!!
	// Check if the audiofile exists in our project audiosources dir
	// and give it priority over the dir as given by the project.tpf file
	// This makes it possible to move project directories without Traverso being
	// unable to find it's audiosources!
	if ( QFile::exists(project->get_root_dir() + "/audiosources/" + m_name) || 
	     QFile::exists(project->get_root_dir() + "/audiosources/" + m_name + "-ch0.wav") ) {
		set_dir(project->get_root_dir() + "/audiosources/");
	}
	
	m_silent = (m_channelCount == 0);
}	

// constructor for file import
ReadSource::ReadSource(const QString& dir, const QString& name)
	: AudioSource(dir, name)
{
	private_init();
	
	AbstractAudioReader* reader = AbstractAudioReader::create_audio_reader(m_fileName);

	if (reader) {
		m_channelCount = reader->get_num_channels();
		delete reader;
	} else {
		m_channelCount = 0;
	}

	m_silent = false;
}


// Constructor for recorded audio.
ReadSource::ReadSource(const QString& dir, const QString& name, int channelCount)
	: AudioSource(dir, name)
{
	private_init();
	
	m_channelCount = channelCount;
	m_silent = false;
	m_name = name  + "-" + QString::number(m_id);
	m_fileName = m_dir + m_name;
	m_rate = pm().get_project()->get_rate();
	m_wasRecording = true;
	m_shortName = m_name.left(m_name.length() - 20);
}


// Constructor for silent clips
ReadSource::ReadSource()
	: AudioSource("", tr("Silence"))
{
	private_init();
	
	m_channelCount = 0;
	m_silent = true;
}


void ReadSource::private_init()
{
	m_refcount = 0;
	m_error = 0;
	m_clip = 0;
	m_audioReader = 0;
	m_bufferstatus = 0;
}


ReadSource::~ReadSource()
{
	PENTERDES;
	for(int i=0; i<m_buffers.size(); ++i) {
		delete m_buffers.at(i);
	}
	
	if (m_audioReader) {
		delete m_audioReader;
	}
	
	if (m_bufferstatus) {
		delete m_bufferstatus;
	}
}

QDomNode ReadSource::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("Source");
	node.setAttribute("channelcount", m_channelCount);
	node.setAttribute("origsheetid", m_origSongId);
	node.setAttribute("dir", m_dir);
	node.setAttribute("id", m_id);
	node.setAttribute("name", m_name);
	node.setAttribute("origbitdepth", m_origBitDepth);
	node.setAttribute("wasrecording", m_wasRecording);
	node.setAttribute("length", m_length.universal_frame());
	node.setAttribute("rate", m_rate);
	node.setAttribute("decoder", m_decodertype);

	return node;
}


int ReadSource::set_state( const QDomNode & node )
{
	PENTER;
	
	QDomElement e = node.toElement();
	m_channelCount = e.attribute("channelcount", "0").toInt();
	m_origSongId = e.attribute("origsheetid", "0").toLongLong();
	set_dir( e.attribute("dir", "" ));
	m_id = e.attribute("id", "").toLongLong();
	m_rate = e.attribute("rate", "0").toUInt();
	bool ok;
	m_length = TimeRef(e.attribute("length", "0").toLongLong(&ok));
	m_origBitDepth = e.attribute("origbitdepth", "0").toInt();
	m_wasRecording = e.attribute("wasrecording", "0").toInt();
	m_decodertype = e.attribute("decoder", "");
	
	// For older project files, this should properly detect if the 
	// audio source was a recording or not., in fact this should suffice
	// and the flag wasrecording would be unneeded, but oh well....
	if (m_origSongId != 0) {
		m_wasRecording = true;
	}
	
	set_name( e.attribute("name", "No name supplied?" ));
	
	return 1;
}


int ReadSource::init( )
{
	PENTER;
	
	Q_ASSERT(m_refcount);
	
	Project* project = pm().get_project();
	
	m_bufferstatus = new BufferStatus;
	
	// Fake the samplerate, until it's set by an AudioReader!
	m_rate = project->get_rate();
	
	if (m_silent) {
		m_length = TimeRef(LONG_LONG_MAX);
		m_channelCount = 0;
		m_origBitDepth = project->get_bitdepth();
		return 1;
	}
	
	if (m_channelCount == 0) {
		PERROR("ReadSource channel count is 0");
		return (m_error = INVALID_CHANNEL_COUNT);
	}
	
	m_rbReady = 0;
	m_needSync = 0;
	m_syncInProgress = 0;
	m_bufferUnderRunDetected = m_wasActivated = 0;
	
	bool useResampling = config().get_property("Conversion", "DynamicResampling", false).toBool();
	
	if (useResampling) {
		int converter_type;
		converter_type = config().get_property("Conversion", "RTResamplingConverterType", 2).toInt();
		
		// There should be another config option for ConverterType to use for export (higher quality)
		//converter_type = config().get_property("Conversion", "ExportResamplingConverterType", 0).toInt();
		m_audioReader = new ResampleAudioReader(m_fileName, m_decodertype);
		
		if (m_audioReader->is_valid()) {
			set_output_rate(audiodevice().get_sample_rate());
			m_audioReader->set_converter_type(converter_type);
		} else {
			delete m_audioReader;
			m_audioReader = 0;
		}
	}
	
	if (m_audioReader == 0) {
		return COULD_NOT_OPEN_FILE;
	}
	
	// (re)set the decoder type
	m_decodertype = m_audioReader->decoder_type();
	m_channelCount = m_audioReader->get_num_channels();
	
	// @Ben: I thought we support any channel count now ??
	if (m_channelCount > 2) {
		PERROR("ReadAudioSource: file contains %d channels; only 2 channels are supported", m_channelCount);
		delete m_audioReader;
		m_audioReader = 0;
		return INVALID_CHANNEL_COUNT;
	}

	// Never reached, it's allready checked in AbstractAudioReader::is_valid() which was allready called!
	if (m_channelCount == 0) {
		PERROR("ReadAudioSource: not a valid channel count: %d", m_channelCount);
		delete m_audioReader;
		m_audioReader = 0;
		return ZERO_CHANNELS;
	}
	
	m_rate = m_audioReader->get_file_rate();
	m_length = m_audioReader->get_length();
	
	return 1;
}


void ReadSource::set_output_rate(int rate)
{
	Q_ASSERT(rate > 0);
	
	m_audioReader->set_output_rate(rate);
	
	// The length could have become slightly smaller/larger due
	// rounding issues involved with converting to one samplerate to another.
	// Should be at the order of one sample at most, but for reading purposes we 
	// need sample accurate information!
	m_length = m_audioReader->get_length();
}


int ReadSource::file_read(DecodeBuffer* buffer, TimeRef& start, nframes_t cnt) const
{
//	PROFILE_START;
	int rate = m_audioReader->get_output_rate();
	nframes_t result = m_audioReader->read_from(buffer, start.to_frame(rate), cnt);
//	PROFILE_END("ReadSource::fileread");
	return result;
}


ReadSource * ReadSource::deep_copy( )
{
	PENTER;
	
	QDomDocument doc("ReadSource");
	QDomNode rsnode = get_state(doc);
	ReadSource* source = new ReadSource(rsnode);
	return source;
}

void ReadSource::set_audio_clip( AudioClip * clip )
{
	PENTER;
	Q_ASSERT(clip);
	m_clip = clip;
}

const nframes_t ReadSource::get_nframes( ) const
{
	Q_ASSERT(m_audioReader);
	return m_audioReader->get_nframes();
}

int ReadSource::set_file(const QString & filename)
{
	PENTER;
	
	Q_ASSERT(m_clip);

	m_error = 0;
	
	int splitpoint = filename.lastIndexOf("/") + 1;
	int length = filename.length();
	
	QString dir = filename.left(splitpoint - 1) + "/";
	QString name = filename.right(length - splitpoint);
		
	set_dir(dir);
	set_name(name);
	
	if (init() < 0) {
		return -1;
	}
	
	set_audio_clip(m_clip);
	
	emit stateChanged();
	
	return 1;
}




int ReadSource::rb_read(audio_sample_t** dst, TimeRef& start, nframes_t count)
{

	if ( ! m_rbReady ) {
// 		printf("ringbuffer not ready\n");
		return 0;
	}

	int devicerate = audiodevice().get_sample_rate();
	
	if (start != m_rbRelativeFileReadPos) {
		
		TimeRef availabletime(m_buffers.at(0)->read_space(), devicerate);
		
		if ( (start > m_rbRelativeFileReadPos) && (m_rbRelativeFileReadPos + availabletime) > (start + TimeRef(count, devicerate))) {
			
			TimeRef advance = start - m_rbRelativeFileReadPos;
			if (availabletime < advance) {
				printf("available < advance !!!!!!!\n");
			}
			for (int i=m_buffers.size()-1; i>=0; --i) {
				m_buffers.at(i)->increment_read_ptr(advance.to_frame(devicerate));
			}
			
			m_rbRelativeFileReadPos += advance;
		} else {
			TimeRef synclocation = start + m_clip->get_track_start_location() + m_clip->get_source_start_location();
			start_resync(synclocation);
			return 0;
		}
	}

	nframes_t readcount = 0;
	
	for (int chan=0; chan<m_channelCount; ++chan) {
		
		readcount = m_buffers.at(chan)->read(dst[chan], count);

		if (readcount != count) {
		// Hmm, not sure what to do in this case....
		}
		
	}

	m_rbRelativeFileReadPos.add_frames(readcount, devicerate);
	
	return readcount;
}


int ReadSource::rb_file_read(DecodeBuffer* buffer, nframes_t cnt)
{
	int readFrames = file_read(buffer, m_rbFileReadPos, cnt);
	m_rbFileReadPos.add_frames(readFrames, audiodevice().get_sample_rate());

	return readFrames;
}


void ReadSource::rb_seek_to_file_position(TimeRef& position)
{
	Q_ASSERT(m_clip);
	
// 	printf("rb_seek_to_file_position:: seeking to %d\n", position);
	
	// calculate position relative to the file!
	TimeRef fileposition = position - m_clip->get_track_start_location() - m_clip->get_source_start_location();
	
	if (m_rbFileReadPos == fileposition) {
// 		printf("ringbuffer allready at position %d\n", position);
		return;
	}

	// check if the clip's start position is within the range
	// if not, fill the buffer from the earliest point this clip
	// will come into play.
	if (fileposition < 0) {
// 		printf("not seeking to %ld, but too %d\n\n", fileposition,m_clip->get_source_start_location()); 
		// Song's start from 0, this makes a period start from
		// 0 - 1023 for example, the nframes is 1024!
		// Setting a songs new position is on 1024, and NOT 
		// 1023.. Hmm, something isn't correct here, but at least substract 1
		// to make this thing work!
		// TODO check if this is still needed!
		fileposition = m_clip->get_source_start_location() - TimeRef(1, audiodevice().get_sample_rate());
	}
	
// 	printf("rb_seek_to_file_position:: seeking to relative pos: %d\n", fileposition);
	for (int i=0; i<m_buffers.size(); ++i) {
		m_buffers.at(i)->reset();
	}
	m_rbFileReadPos = fileposition;
	m_rbRelativeFileReadPos = fileposition;
}


void ReadSource::process_ringbuffer(DecodeBuffer* buffer, bool seeking)
{
	// Do nothing if we passed the lenght of the AudioFile.
	if (m_rbFileReadPos >= m_length) {
// 		printf("returning, m_rbFileReadPos > m_length! (%d >  %d)\n", m_rbFileReadPos.to_frame(get_rate()), m_audioReader->get_nframes());
		if (m_syncInProgress) {
			finish_resync();
		}
		return;
	}
	
	// Calculate the number of samples we can write into the buffer
	int writeSpace = m_buffers.at(0)->write_space();

	// The amount of chunks which can be 'read'
	int chunkCount = (int)(writeSpace / m_chunkSize);
	
	int toRead = m_chunkSize;
	
	if (seeking) {
		toRead = writeSpace;
// 		printf("doing a full seek buffer fill\n");
	} else if (m_syncInProgress) {
		// Currently, we fill the buffer completely.
		// For some reason, filling it with 1/4 at a time
		// doesn't fill it consitently, and thus giving audible artifacts.
		/*		toRead = m_chunkSize * 2;*/
		toRead = writeSpace;
	} else if (chunkCount == 0) {
		// If we are nearing the end of the source file it could be possible
		// we only need to read the last samples which is smaller in size then 
		// chunksize. If so, set toRead to m_source->m_length - rbFileReasPos
		nframes_t available = (m_length - m_rbFileReadPos).to_frame(audiodevice().get_sample_rate());
		if (available <= m_chunkSize) {
			toRead = available;
		} else {
			printf("ReadSource:: chunkCount == 0, but not at end of file, this shouldn't happen!!\n");
			return;
		}
	}
	
	// Read in the samples from source
	nframes_t toWrite = rb_file_read(buffer, toRead);
	
	// and write it to the ringbuffer
	for (int i=m_buffers.size()-1; i>=0; --i) {
		m_buffers.at(i)->write(buffer->destination[i], toWrite);
	}
}


void ReadSource::start_resync(TimeRef& position)
{
// 	printf("starting resync!\n");
	m_syncPos = position;
	m_rbReady = 0;
	m_needSync = 1;
}

void ReadSource::finish_resync()
{
//  	printf("sync finished\n");
	m_needSync = 0;
	m_bufferUnderRunDetected = 0;
	m_rbReady = 1;
	m_syncInProgress = 0;
}

void ReadSource::sync(DecodeBuffer* buffer)
{
	PENTER2;
	
	if (!m_audioReader) {
		return;
	}
	
	if (!m_needSync) {
		return;
	}
	
	if (!m_syncInProgress) {
		rb_seek_to_file_position(m_syncPos);
		m_syncInProgress = 1;
	}
	
	// Currently, we fill the buffer completely.
	// For some reason, filling it with 1/4 at a time
	// doesn't fill it consitently, and thus giving audible artifacts.
	process_ringbuffer(buffer);
	
	if (m_buffers.at(0)->write_space() == 0) {
		finish_resync();
	}
	
//         PWARN("Resyncing ringbuffer finished");
}



void ReadSource::prepare_buffer( )
{
	PENTER;
	
	Q_ASSERT(m_clip);
	
	for (int i=0; i<m_buffers.size();++i) {
		delete m_buffers.at(i);
	}
	
	m_buffers.clear();

	float size = config().get_property("Hardware", "readbuffersize", 1.0).toDouble();

	m_bufferSize = (int) (size * audiodevice().get_sample_rate());

	m_chunkSize = m_bufferSize / DiskIO::bufferdividefactor;

	for (int i=0; i<m_channelCount; ++i) {
		m_buffers.append(new RingBufferNPT<float>(m_bufferSize));
	}

	TimeRef synclocation = m_clip->get_song()->get_work_location();
	start_resync(synclocation);
}

BufferStatus* ReadSource::get_buffer_status()
{
	int freespace = m_buffers.at(0)->write_space();
	
// 	printf("m_rbFileReadPos, m_length %d, %d\n", m_rbFileReadPos.to_frame(get_rate()), m_length.to_frame(get_rate()));

	if (m_rbFileReadPos >= m_length) {
		m_bufferstatus->fillStatus =  100;
		freespace = 0;
	} else {
		m_bufferstatus->fillStatus = (int) (((float)freespace / m_bufferSize) * 100);
	}
	
	m_bufferstatus->bufferUnderRun = m_bufferUnderRunDetected;
	m_bufferstatus->needSync = m_needSync;

	m_bufferstatus->priority = (int) (freespace / m_chunkSize);
	
	return m_bufferstatus;
}

void ReadSource::set_active(bool active)
{
	if (m_active == active)
		return;

	if (active) {
		m_active = 1;
// 		m_wasActivated = 1;
// 		printf("setting private readsource %s to active\n", QS_C(m_fileName));
	} else {
// 		printf("setting private readsource %s to IN-active\n", QS_C(m_fileName));
		m_active = 0;
	}
}


int ReadSource::file_read(DecodeBuffer * buffer, nframes_t start, nframes_t cnt)
{
	TimeRef startlocation(start, m_audioReader->get_output_rate());
	return file_read(buffer, startlocation, cnt);
}


int ReadSource::get_file_rate() const
{
	if (m_audioReader) {
		return m_audioReader->get_file_rate();
	} else {
		PERROR("ReadSource::get_file_rate(), but no audioreader available!!");
	}
	
	return pm().get_project()->get_rate(); 
}

