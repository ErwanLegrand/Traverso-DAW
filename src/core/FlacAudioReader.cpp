/*
Copyright (C) 2007 Ben Levitt 
 * Based on the FLAC decoder module for K3b.
 * Based on the Ogg Vorbis module for same.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 * Copyright (C) 2003-2004 John Steele Scott <toojays@toojays.net>


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

#include "FlacAudioReader.h"
#include <QFile>
#include <QString>
#include "Utils.h"

#include "FLAC++/decoder.h"

#if !defined FLACPP_API_VERSION_CURRENT || FLACPP_API_VERSION_CURRENT < 6
#define LEGACY_FLAC
#else
#undef LEGACY_FLAC
#endif


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


class FlacPrivate
#ifdef LEGACY_FLAC
  : public FLAC::Decoder::SeekableStream
#else
  : public FLAC::Decoder::Stream
#endif
{
	public:
		FlacPrivate(QString filename);
		~FlacPrivate();
		
		bool open(QString filename) {
			file = new QFile(filename);
			if (!file->open(QIODevice::ReadOnly)) {
				return false;
			}
			
			init();
			process_until_end_of_metadata();
			return true;
		}
		
		void cleanup() {
			delete internalBuffer;
			file->close();
			delete file;
			finish();
		}
		
		
		bool seek(nframes_t start);
		int read(audio_sample_t* dst, int sampleCount);
		
		uint m_channels;
		uint m_rate;
		uint m_bitsPerSample;
		uint m_samples;
		
		QVector<audio_sample_t>	*internalBuffer;
		int			bufferStart;
		
	protected:
#ifdef LEGACY_FLAC
		virtual FLAC__SeekableStreamDecoderReadStatus read_callback(FLAC__byte buffer[], unsigned *bytes);
		virtual FLAC__SeekableStreamDecoderSeekStatus seek_callback(FLAC__uint64 absolute_byte_offset);
		virtual FLAC__SeekableStreamDecoderTellStatus tell_callback(FLAC__uint64 *absolute_byte_offset);
		virtual FLAC__SeekableStreamDecoderLengthStatus length_callback(FLAC__uint64 *stream_length);
#else
		virtual FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t *bytes);
		virtual FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 absolute_byte_offset);
		virtual FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64 *absolute_byte_offset);
		virtual FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64 *stream_length);
#endif
		virtual bool eof_callback();
		virtual void error_callback(FLAC__StreamDecoderErrorStatus s){ printf("!!! %d !!!\n", s); };
		virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata);
		virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
		
		QFile		*file;
};


FlacPrivate::FlacPrivate(QString filename)
#ifdef LEGACY_FLAC
			: FLAC::Decoder::SeekableStream()
#else
			: FLAC::Decoder::Stream()
#endif
{
	internalBuffer = new QVector<audio_sample_t>();
	bufferStart = 0;
	open(filename);
	process_until_end_of_metadata();
}


FlacPrivate::~FlacPrivate()
{
	cleanup();
}


bool FlacPrivate::seek(nframes_t start)
{
	return seek_absolute(start);
}


FLAC__StreamDecoderWriteStatus FlacPrivate::write_callback(const FLAC__Frame *frame, const FLAC__int32 * const buffer[]) {
	unsigned i, c, pos = 0;
	unsigned frames = frame->header.blocksize;
	
	if (internalBuffer->size() > 0) {
		// This shouldn't be happening, but if it does, the code can handle it now. :)
		PERROR("internalBuffer is already non-empty");
	}
	
	internalBuffer->resize(internalBuffer->size() + frames * frame->header.channels);
	
	for (i=0; i < frames; i++) {
		// in FLAC channel 0 is left, 1 is right
		for (c=0; c < frame->header.channels; c++) {
			audio_sample_t value = (audio_sample_t)((float)buffer[c][i] / (float)((uint)1<<(frame->header.bits_per_sample-1)));
			internalBuffer->data()[bufferStart + (pos++)] = value;
		}
	}
	
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderReadStatus FlacPrivate::read_callback(FLAC__byte buffer[],                                                                             unsigned *bytes) {
  long retval =  file->read((char *)buffer, (*bytes));
  if(-1 == retval) {
    return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR;
  } else {
    (*bytes) = retval;
    return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK;
  }
}
#else
FLAC__StreamDecoderReadStatus FlacPrivate::read_callback(FLAC__byte buffer[],                                                                             size_t *bytes) {
  long retval =  file->read((char *)buffer, (*bytes));
  if(-1 == retval) {
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
  } else {
    (*bytes) = retval;
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
  }
}
#endif

#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderSeekStatus 
FlacPrivate::seek_callback(FLAC__uint64 absolute_byte_offset) {
  if(!file->seek(absolute_byte_offset))
    return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR;
  else
    return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_OK;
}
#else
FLAC__StreamDecoderSeekStatus 
FlacPrivate::seek_callback(FLAC__uint64 absolute_byte_offset) {
  if(file->seek(absolute_byte_offset) == FALSE)
    return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
  else
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}
#endif

#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderTellStatus 
FlacPrivate::tell_callback(FLAC__uint64 *absolute_byte_offset) {
  (*absolute_byte_offset) = file->pos();
  return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_OK;
}
#else
FLAC__StreamDecoderTellStatus 
FlacPrivate::tell_callback(FLAC__uint64 *absolute_byte_offset) {
  (*absolute_byte_offset) = file->pos();
  return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}
#endif

#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderLengthStatus 
FlacPrivate::length_callback(FLAC__uint64 *stream_length) {
  (*stream_length) = file->size();
  return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK;
}
#else
FLAC__StreamDecoderLengthStatus 
FlacPrivate::length_callback(FLAC__uint64 *stream_length) {
  (*stream_length) = file->size();
  return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}
#endif


void FlacPrivate::metadata_callback(const FLAC__StreamMetadata *metadata) {
  switch (metadata->type) {
  case FLAC__METADATA_TYPE_STREAMINFO:
    m_channels = metadata->data.stream_info.channels;
    m_rate = metadata->data.stream_info.sample_rate;
    m_bitsPerSample = metadata->data.stream_info.bits_per_sample;
    m_samples = metadata->data.stream_info.total_samples;
    break;
  case FLAC__METADATA_TYPE_VORBIS_COMMENT:
    //comments = new FLAC::Metadata::VorbisComment((FLAC__StreamMetadata *)metadata, true);
    break;
  default:
    break;
  }
}


bool FlacPrivate::eof_callback() {
  return file->atEnd();
}




FlacAudioReader::FlacAudioReader(QString filename)
 : AbstractAudioReader(filename)
{
	m_flac = new FlacPrivate(filename);
}


FlacAudioReader::~FlacAudioReader()
{
	if (m_flac) {
		m_flac->finish();
	}
}


bool FlacAudioReader::can_decode(QString filename)
{
	// buffer large enough to read an ID3 tag header
	char buf[10];

	// Note: since file is created on the stack it will be closed automatically
	// by its destructor when this method (i.e. canDecode) returns.
	QFile f(filename);

	if (!f.open(QIODevice::ReadOnly)) {
		PERROR("Could not open file %s", QS_C(filename));
		return false;
	}

	// look for a fLaC magic number or ID3 tag header
	if (10 != f.read(buf, 10)) {
		//PERROR("File too small to be a FLAC file: %s", QS_C(filename));
		return false;
	}

	if (0 == memcmp(buf, "ID3", 3)) {
		// Found ID3 tag, try and seek past it.
		//kdDebug() << "(K3bFLACDecorder) File " << filename << ": found ID3 tag" << endl;

		// See www.id3.org for details of the header, note that the size field
		// unpacks to 7-bit bytes, then the +10 is for the header itself.
		int pos;
		pos = ((buf[6]<<21)|(buf[7]<<14)|(buf[8]<<7)|buf[9]) + 10;

		//kdDebug() << "(K3bFLACDecoder) " << filename << ": seeking to " 
		//	<< pos << endl;
		if (!f.seek(pos)) {
			//PERROR("Couldn't seek to %d in file: %s", pos, QS_C(filename));
			return false;
		}
		else {
			// seek was okay, try and read magic number into buf
			if (4 != f.read(buf, 4)) {
				//PERROR("File has ID3 tag but nothing else: %s", QS_C(filename));
				return false;
			}
		}
	}

	if (memcmp(buf, "fLaC", 4) != 0) {
		//PERROR("Not a flac file: %s", QS_C(filename));
		return false;
	}
	
	f.close();
	
	FlacPrivate flac(filename);
	
	bool valid = flac.is_valid();
	flac.finish();
	
	//PERROR("Return: Is%s a flac file: %s", ((valid) ? "" : " not"), QS_C(filename));
	return valid;
}


int FlacAudioReader::get_num_channels()
{
	if (m_flac) {
		return m_flac->m_channels;
	}
	return 0;
}


nframes_t FlacAudioReader::get_length()
{
	if (m_flac) {
		// Is this returning one frame too long?
		return m_flac->m_samples;
	}
	return 0;
}


int FlacAudioReader::get_rate()
{
	if (m_flac) {
		return m_flac->m_rate;
	}
	return 0;
}


bool FlacAudioReader::seek(nframes_t start)
{
	Q_ASSERT(m_flac);
	
	if (start >= get_length()) {
		PERROR("FlacAudioReader: could not seek to frame %d within %s, it's past the end.", start, QS_C(m_fileName));
		return false;
	}
	
	m_flac->internalBuffer->resize(0);
	m_flac->bufferStart = 0;
	
	m_flac->flush();
	
	if (!m_flac->seek(start)) {
		PERROR("FlacAudioReader: could not seek to frame %d within %s", start, QS_C(m_fileName));
		return false;
	}
	
	return true;
}


int FlacAudioReader::read(audio_sample_t* dst, int sampleCount)
{
	Q_ASSERT(m_flac);
	
	int samplesToCopy;
	int samplesAvailable;
	int samplesCoppied = 0;
	
	while (samplesCoppied < sampleCount) {
		if (m_flac->internalBuffer->size() == 0) {
			// want more data
#ifdef LEGACY_FLAC
			if (m_flac->get_state() == FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM) {
				//printf("flac file finish\n");
				m_flac->reset();
				break;
			}
			else if(m_flac->get_state() == FLAC__SEEKABLE_STREAM_DECODER_OK) {
				//printf("process1\n");
				if (!m_flac->process_single()) {
					PERROR("process_single() error\n");
					m_flac->reset();
					seek(m_readPos);
					return -1;
				}
			}
			else {
				PERROR("flac_state() = %d\n", int(m_flac->get_state()));
				m_flac->reset();
				seek(m_readPos);
				return -1;
			}
#else
			if (m_flac->get_state() == FLAC__STREAM_DECODER_END_OF_STREAM) {
				//printf("flac file finish\n");
				m_flac->reset();
				break;
			}
			else if(m_flac->get_state() < FLAC__STREAM_DECODER_END_OF_STREAM) {
				if (!m_flac->process_single()) {
					PERROR("process_single() error\n");
					m_flac->reset();
					seek(m_readPos);
					return -1;
				}
			}
			else {
				PERROR("flac_state() = %d\n", int(m_flac->get_state()));
				m_flac->reset();
				seek(m_readPos);
				return -1;
			}
#endif
		}
		
		samplesAvailable = m_flac->internalBuffer->size() - m_flac->bufferStart;
		samplesToCopy = (sampleCount - samplesCoppied < samplesAvailable) ? sampleCount - samplesCoppied : samplesAvailable;
		for (int i = 0; i < samplesToCopy; i++) {
			dst[samplesCoppied + i] = m_flac->internalBuffer->at(m_flac->bufferStart + i);
		}
		
		if(samplesToCopy == samplesAvailable) {
			m_flac->internalBuffer->resize(0);
			m_flac->bufferStart = 0;
		}
		else {
			m_flac->bufferStart += samplesToCopy;
		}
		samplesCoppied += samplesToCopy;
		
		//printf("samplesCoppied = %d (%d, %d)\n", samplesCoppied, m_flac->bufferStart, m_flac->internalBuffer->size());
	}
	
	// Pad end of file with 0s if necessary.  (Shouldn't be necessary...)
	int remainingSamplesRequested = sampleCount - samplesCoppied;
	int remainingSamplesInFile = get_length() * get_num_channels() - (m_readPos * get_num_channels() + samplesCoppied);
	if (samplesCoppied == 0 && remainingSamplesInFile > 0) {
		int padLength = (remainingSamplesRequested > remainingSamplesInFile) ? remainingSamplesInFile : remainingSamplesRequested;
		//PERROR("padLength: %d", padLength);
		memset(dst + samplesCoppied, 0, padLength * sizeof(audio_sample_t));
		samplesCoppied += padLength;
	}
	if (samplesCoppied > sampleCount) {
		//PERROR("Truncating");
		samplesCoppied = sampleCount;
	}
	
	//printf("copied %d of %d.  nextFrame: %lu of %lu\n", samplesCoppied, sampleCount, m_readPos, get_length());
	
	return samplesCoppied;
}

