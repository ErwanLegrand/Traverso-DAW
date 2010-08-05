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
#include "Sheet.h"
#include "AudioDevice.h"
#include <QFile>
#include "Config.h"
#include <limits.h>

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
	node.setAttribute("origsheetid", m_origSheetId);
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
	m_origSheetId = e.attribute("origsheetid", "0").toLongLong();
	set_dir( e.attribute("dir", "" ));
	m_id = e.attribute("id", "").toLongLong();
	m_rate = m_outputRate = e.attribute("rate", "0").toUInt();
	bool ok;
	m_length = TimeRef(e.attribute("length", "0").toLongLong(&ok));
	m_origBitDepth = e.attribute("origbitdepth", "0").toInt();
	m_wasRecording = e.attribute("wasrecording", "0").toInt();
	m_decodertype = e.attribute("decoder", "");
	
	// For older project files, this should properly detect if the 
	// audio source was a recording or not., in fact this should suffice
	// and the flag wasrecording would be unneeded, but oh well....
	if (m_origSheetId != 0) {
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
	if (project) {
		m_rate = m_outputRate = project->get_rate();
	} else {
		m_rate = 44100;
	}
	
	if (m_silent) {
		m_length = TimeRef(LLONG_MAX);
		m_channelCount = 0;
		m_origBitDepth = 16;
		m_bufferstatus->fillStatus =  100;
		m_bufferstatus->needSync = false;
		return 1;
	}
	
	if (m_channelCount == 0) {
		PERROR("ReadSource channel count is 0");
		return (m_error = INVALID_CHANNEL_COUNT);
	}
	
	if ( ! QFile::exists(m_fileName)) {
		return (m_error = FILE_DOES_NOT_EXIST);
	}
	
	m_rbReady = 0;
	m_needSync = 1;
	m_syncInProgress = 0;
	m_bufferUnderRunDetected = m_wasActivated = 0;
	m_active = 0;
	
	// There should be another config option for ConverterType to use for export (higher quality)
	//converter_type = config().get_property("Conversion", "ExportResamplingConverterType", 0).toInt();
	m_audioReader = new ResampleAudioReader(m_fileName, m_decodertype);
	
	if (!m_audioReader->is_valid()) {
		PERROR("ReadSource:: audio reader is not valid! (reader channel count: %d, nframes: %d", m_audioReader->get_num_channels(), m_audioReader->get_nframes());
		delete m_audioReader;
		m_audioReader = 0;
		return (m_error = COULD_NOT_OPEN_FILE);
	}
	
	int converter_type = config().get_property("Conversion", "RTResamplingConverterType", DEFAULT_RESAMPLE_QUALITY).toInt();
	m_audioReader->set_converter_type(converter_type);
	
	set_output_rate(m_audioReader->get_file_rate());
	
	// (re)set the decoder type
	m_decodertype = m_audioReader->decoder_type();
	m_channelCount = m_audioReader->get_num_channels();
	
	// @Ben: I thought we support any channel count now ??
	if (m_channelCount > 2) {
		PERROR("ReadAudioSource: file contains %d channels; only 2 channels are supported", m_channelCount);
		delete m_audioReader;
		m_audioReader = 0;
		return (m_error = INVALID_CHANNEL_COUNT);
	}

	// Never reached, it's allready checked in AbstractAudioReader::is_valid() which was allready called!
	if (m_channelCount == 0) {
		PERROR("ReadAudioSource: not a valid channel count: %d", m_channelCount);
		delete m_audioReader;
		m_audioReader = 0;
		return (m_error = ZERO_CHANNELS);
	}
	
	m_rate = m_audioReader->get_file_rate();
	m_length = m_audioReader->get_length();
	
	return 1;
}


void ReadSource::set_output_rate(int rate)
{
	Q_ASSERT(rate > 0);
	
	if (! m_audioReader) {
		printf("ReadSource::set_output_rate: No audioreader!\n");
		return;
	}
	
	bool useResampling = config().get_property("Conversion", "DynamicResampling", true).toBool();
	if (useResampling) {
		m_audioReader->set_output_rate(rate);
	} else {
		m_audioReader->set_output_rate(m_audioReader->get_file_rate());
	}

	m_outputRate = rate;
	
	// The length could have become slightly smaller/larger due
	// rounding issues involved with converting to one samplerate to another.
	// Should be at the order of one - two samples at most, but for reading purposes we 
	// need sample accurate information!
	m_length = m_audioReader->get_length();
}


int ReadSource::file_read(DecodeBuffer* buffer, const TimeRef& start, nframes_t cnt) const
{
//	PROFILE_START;
	Q_ASSERT(m_audioReader);
	nframes_t result = m_audioReader->read_from(buffer, start, cnt);
//	PROFILE_END("ReadSource::fileread");
	return result;
}


int ReadSource::file_read(DecodeBuffer * buffer, nframes_t start, nframes_t cnt)
{
	Q_ASSERT(m_audioReader);
	return m_audioReader->read_from(buffer, start, cnt);
}


ReadSource * ReadSource::deep_copy( )
{
	PENTER;
	
	QDomDocument doc("ReadSource");
	QDomNode rsnode = get_state(doc);
	ReadSource* source = new ReadSource(rsnode);
	return source;
}

void ReadSource::set_audio_clip(AudioClip* clip)
{
	PENTER;
	Q_ASSERT(clip);
	m_clip = clip;
}

nframes_t ReadSource::get_nframes( ) const
{
	if (!m_audioReader) {
		return 0;
	}
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
	if (m_channelCount == 0) {
		return count;
	}
	
	if ( ! m_rbReady ) {
// 		printf("ringbuffer not ready\n");
		return 0;
	}

	TimeRef diff = m_rbRelativeFileReadPos - start;
	// In universal frame positioning, it is possible (somehow) that the start 
	// location and m_rbRelativeFileReadPos differ very very slighly, and when
	// converted to frames, the difference is much smaller then 1 frame.
	// To catch these kind of I think 'rounding issues' we convert to frames here
	// and see if the diff in frames == 0, in which case it should be fine to make 
	// our m_rbRelativeFileReadPos equal to start, and thus avoiding unwanted resync actions!
	if ( (diff.universal_frame() > 0) && (diff.to_frame(m_outputRate) == 0) ) {
		m_rbRelativeFileReadPos = start;
	}
	
	if (start != m_rbRelativeFileReadPos) {
		
		TimeRef availabletime(nframes_t(m_buffers.at(0)->read_space()), m_outputRate);
/*		printf("rb_read:: m_rbRelativeFileReadPos, start: %lld, %lld\n", m_rbRelativeFileReadPos.universal_frame(), start.universal_frame());
		printf("rb_read:: availabletime %d\n", availabletime.to_frame(m_outputRate));*/
		
		if ( (start > m_rbRelativeFileReadPos) && ((m_rbRelativeFileReadPos + availabletime) > (start + TimeRef(count, m_outputRate))) ) {
			
			TimeRef advance = start - m_rbRelativeFileReadPos;
			if (availabletime < advance) {
				printf("available < advance !!!!!!!\n");
			}
			for (int i=m_buffers.size()-1; i>=0; --i) {
				m_buffers.at(i)->increment_read_ptr(advance.to_frame(m_outputRate));
			}
			
			m_rbRelativeFileReadPos += advance;
/*			printf("rb_read:: advance %d\n", advance.to_frame(m_outputRate));
			printf("rb_read:: m_rbRelativeFileReadPos after advance %d\n", m_rbRelativeFileReadPos.to_frame(m_outputRate));*/
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
			PMESG("readcount, count: %d, %d", readcount, count);
		// Hmm, not sure what to do in this case....
		}
		
	}

	m_rbRelativeFileReadPos.add_frames(readcount, m_outputRate);
	
	return readcount;
}


int ReadSource::rb_file_read(DecodeBuffer* buffer, nframes_t cnt)
{
	nframes_t readFrames = file_read(buffer, m_rbFileReadPos, cnt);
	if (readFrames == cnt) {
		m_rbFileReadPos.add_frames(readFrames, m_outputRate);
	} else {
		// We either passed the end of the file, or our audio reader
		// is doing weird things, is broken, invalid or something else
                // Set the rinbuffer file readpos to m_length so processing stops here!
		m_rbFileReadPos = m_length;
	}

	return readFrames;
}


void ReadSource::rb_seek_to_file_position(TimeRef& position)
{
	Q_ASSERT(m_clip);
	
// 	printf("rb_seek_to_file_position:: seeking to %d\n", position);
	
	// calculate position relative to the file!
	TimeRef fileposition = position - m_clip->get_track_start_location() - m_clip->get_source_start_location();
	
	// Do nothing if we are allready at the seek position
	if (m_rbFileReadPos == fileposition) {
// 		printf("ringbuffer allready at position %d\n", position);
		return;
	}

	// check if the clip's start position is within the range
	// if not, fill the buffer from the earliest point this clip
	// will come into play.
	if (fileposition < TimeRef()) {
// 		printf("not seeking to %ld, but too %d\n\n", fileposition,m_clip->get_source_start_location()); 
		fileposition = m_clip->get_source_start_location();
	}
	
// 	printf("rb_seek_to_file_position:: seeking to relative pos: %d\n", fileposition);
	
	// The content of our buffers is no longer valid, so we empty them
	for (int i=0; i<m_buffers.size(); ++i) {
		m_buffers.at(i)->reset();
	}
	
	m_rbFileReadPos = fileposition;
	m_rbRelativeFileReadPos = fileposition;
// 	printf("rb_seek_to_file_position:: m_rbRelativeFileReadPos, synclocation: %d, %d\n", m_rbRelativeFileReadPos.to_frame(m_outputRate), fileposition.to_frame(m_outputRate));
}


void ReadSource::process_ringbuffer(DecodeBuffer* buffer, bool seeking)
{
	if (m_channelCount == 0) {
		return;
	}
	
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
		nframes_t available = (m_length - m_rbFileReadPos).to_frame(m_outputRate);
		if (available <= m_chunkSize) {
			toRead = available;
		} else {
			printf("ReadSource:: chunkCount == 0, but not at end of file, this shouldn't happen!!\n");
			return;
		}
	}
	
	// Check if the resample quality has changed, it's a safe place here
	// to reconfigure the audioreaders resample quality.
	// This allows on the fly changing of the resample quality :)
	if (m_diskio->get_resample_quality() != m_audioReader->get_convertor_type()) {
		m_audioReader->set_converter_type(m_diskio->get_resample_quality());
	}
	
	// Read in the samples from source
	nframes_t toWrite = rb_file_read(buffer, toRead);
	
	// and write it to the ringbuffer
	if (toWrite) {
		for (int i=m_buffers.size()-1; i>=0; --i) {
			m_buffers.at(i)->write(buffer->destination[i], toWrite);
		}
	}
}


void ReadSource::start_resync(TimeRef& position)
{
// 	printf("starting resync!\n");
	if (m_needSync || m_syncInProgress) {
// 		printf("start_resync still in progress!\n");
		return;
	}
		
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
// 	printf("source::sync: %s\n", QS_C(m_fileName));
	
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



void ReadSource::prepare_rt_buffers( )
{
	PENTER;
	
	Q_ASSERT(m_clip);
	
	for (int i=0; i<m_buffers.size();++i) {
		delete m_buffers.at(i);
	}
	
	m_buffers.clear();

	float size = config().get_property("Hardware", "readbuffersize", 1.0).toDouble();

        m_bufferSize = (int) (size * m_outputRate);

        // TODO: reading is done in chunkSizes, mayb it's more performant to
        // have chunck sizes that are multiples of 4KB ?
        m_chunkSize = m_bufferSize / DiskIO::bufferdividefactor;

	for (int i=0; i<m_channelCount; ++i) {
		m_buffers.append(new RingBufferNPT<float>(m_bufferSize));
	}

        // FIXME: does this really make sense to do still ? :
        TimeRef synclocation = m_clip->get_sheet()->get_transport_location();
        start_resync(synclocation);
}

BufferStatus* ReadSource::get_buffer_status()
{
	if (m_channelCount == 0) {
		return m_bufferstatus;
	}
	
	int freespace = m_buffers.at(0)->write_space();
	
// 	printf("m_rbFileReadPos, m_length %lld, %lld\n", m_rbFileReadPos.universal_frame(), m_length.universal_frame());
	TimeRef transport = m_clip->get_sheet()->get_transport_location();
	TimeRef syncstartlocation = m_clip->get_track_start_location();
	bool transportBeforeSyncStartLocation = transport < (syncstartlocation - (3 * UNIVERSAL_SAMPLE_RATE));
	bool transportAfterClipEndLocation = transport > (m_clip->get_track_end_location() + (3 * UNIVERSAL_SAMPLE_RATE));
			
	if (m_rbFileReadPos >= m_length || !m_active || transportBeforeSyncStartLocation || transportAfterClipEndLocation) {
		m_bufferstatus->fillStatus =  100;
		freespace = 0;
		m_bufferstatus->needSync = false;
	} else {
		m_bufferstatus->fillStatus = (int) (((float)freespace / m_bufferSize) * 100);
		m_bufferstatus->needSync = m_needSync;
	}
	
	m_bufferstatus->bufferUnderRun = m_bufferUnderRunDetected;
	m_bufferstatus->priority = (int) (freespace / m_chunkSize);
	
	return m_bufferstatus;
}

void ReadSource::set_active(bool active)
{
        if (active) {
		m_active = 1;
	} else {
		m_active = 0;
	}
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

void ReadSource::set_diskio(DiskIO * diskio)
{
	m_diskio = diskio;
	set_output_rate(m_diskio->get_output_rate());
	
	if (m_audioReader) {
		m_audioReader->set_resample_decode_buffer(m_diskio->get_resample_decode_buffer());
		m_audioReader->set_converter_type(m_diskio->get_resample_quality());
	}
	
	prepare_rt_buffers();
}

QString ReadSource::get_error_string() const
{
	switch(m_error) {
		case COULD_NOT_OPEN_FILE: return tr("Could not open file");
		case INVALID_CHANNEL_COUNT: return tr("Invalid channel count");
		case ZERO_CHANNELS: return tr("File has zero channels");
		case FILE_DOES_NOT_EXIST: return tr("The file does not exist!");
		default: return tr("No ReadSource error set");
	}
	return tr("No ReadSource error set");
}

