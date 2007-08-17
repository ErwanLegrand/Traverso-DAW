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

#include "FLAC/export.h"

#if !defined FLAC_API_VERSION_CURRENT || FLAC_API_VERSION_CURRENT < 6
#define LEGACY_FLAC
#include "FLAC/seekable_stream_decoder.h"
#else
#undef LEGACY_FLAC
#include "FLAC/stream_decoder.h"
#endif

RELAYTOOL_FLAC


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


class FlacPrivate
{
	public:
		FlacPrivate(QString filename)
		{
			internalBuffer = 0;
			bufferSize = 0;
			bufferUsed = 0;
			bufferStart = 0;
			open(filename);
		}
		
		
		~FlacPrivate()
		{
			cleanup();
		}
		
		
		bool open(QString filename)
		{
			file = new QFile(filename);
			if (!file->open(QIODevice::ReadOnly)) {
				return false;
			}
			
#ifdef LEGACY_FLAC
			flac = FLAC__seekable_stream_decoder_new();
			
			FLAC__seekable_stream_decoder_set_read_callback(flac, FlacPrivate::read_callback);
			FLAC__seekable_stream_decoder_set_seek_callback(flac, FlacPrivate::seek_callback);
			FLAC__seekable_stream_decoder_set_tell_callback(flac, FlacPrivate::tell_callback);
			FLAC__seekable_stream_decoder_set_length_callback(flac, FlacPrivate::length_callback);
			FLAC__seekable_stream_decoder_set_eof_callback(flac, FlacPrivate::eof_callback);
			FLAC__seekable_stream_decoder_set_write_callback(flac, FlacPrivate::write_callback);
			FLAC__seekable_stream_decoder_set_metadata_callback(flac, FlacPrivate::metadata_callback);
			FLAC__seekable_stream_decoder_set_error_callback(flac, FlacPrivate::error_callback);
			FLAC__seekable_stream_decoder_set_client_data(flac, this);
			
			FLAC__seekable_stream_decoder_init(flac);
			FLAC__seekable_stream_decoder_process_until_end_of_metadata(flac);
#else
			flac = FLAC__stream_decoder_new();
			
			FLAC__stream_decoder_init_stream(flac,
				FlacPrivate::read_callback,
				FlacPrivate::seek_callback,
				FlacPrivate::tell_callback,
				FlacPrivate::length_callback,
				FlacPrivate::eof_callback,
				FlacPrivate::write_callback,
				FlacPrivate::metadata_callback,
				FlacPrivate::error_callback,
				this);
			
			FLAC__stream_decoder_process_until_end_of_metadata(flac);
#endif
			return true;
		}
		
		
		bool is_valid() { return (flac != 0); }
#ifdef LEGACY_FLAC
		bool flush() { return FLAC__seekable_stream_decoder_flush(flac); }
		bool finish() { return FLAC__seekable_stream_decoder_finish(flac); }
		bool reset() { return FLAC__seekable_stream_decoder_reset(flac); }
		bool process_single() { return FLAC__seekable_stream_decoder_process_single(flac); }
		FLAC__SeekableStreamDecoderState get_state() { return FLAC__seekable_stream_decoder_get_state(flac); }
#else
		bool flush() { return FLAC__stream_decoder_flush(flac); }
		bool finish() { return FLAC__stream_decoder_finish(flac); }
		bool reset() { return FLAC__stream_decoder_reset(flac); }
		bool process_single() { return FLAC__stream_decoder_process_single(flac); }
		FLAC__StreamDecoderState get_state() { return FLAC__stream_decoder_get_state(flac); }
#endif
		
		
		void cleanup()
		{
			clear_buffers();
			file->close();
			delete file;
			
			finish();
			
#ifdef LEGACY_FLAC
			FLAC__seekable_stream_decoder_delete(flac);
#else
			FLAC__stream_decoder_delete(flac);
#endif
		}
		
		
		void clear_buffers()
		{
			if (internalBuffer) {
				delete [] internalBuffer;
				internalBuffer = 0;
			}
		}
		
		bool seek(nframes_t start)
		{
#ifdef LEGACY_FLAC
			return FLAC__seekable_stream_decoder_seek_absolute(flac, start);
#else
			return FLAC__stream_decoder_seek_absolute(flac, start);
#endif
		}
		
		uint m_channels;
		uint m_rate;
		uint m_bitsPerSample;
		uint m_samples;
		
		audio_sample_t	*internalBuffer;
		int		bufferSize;
		int		bufferUsed;
		int		bufferStart;
		
	protected:
#ifdef LEGACY_FLAC
		static FLAC__SeekableStreamDecoderReadStatus read_callback(const FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data);
		static FLAC__SeekableStreamDecoderSeekStatus seek_callback(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data);
		static FLAC__SeekableStreamDecoderTellStatus tell_callback(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
		static FLAC__SeekableStreamDecoderLengthStatus length_callback(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data);
		static FLAC__bool eof_callback(const FLAC__SeekableStreamDecoder *decoder, void *client_data);
		static void error_callback(const FLAC__SeekableStreamDecoder *decoder, FLAC__StreamDecoderErrorStatus s, void *client_data){ Q_UNUSED(decoder); Q_UNUSED(client_data); printf("!!! %d !!!\n", s); };
		static void metadata_callback(const FLAC__SeekableStreamDecoder *decoder, const ::FLAC__StreamMetadata *metadata, void *client_data);
		static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__SeekableStreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
#else
		static FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data);
		static FLAC__StreamDecoderSeekStatus seek_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data);
		static FLAC__StreamDecoderTellStatus tell_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
		static FLAC__StreamDecoderLengthStatus length_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data);
		static FLAC__bool eof_callback(const FLAC__StreamDecoder *decoder, void *client_data);
		static void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus s, void *client_data){ printf("!!! %d !!!\n", s); };
		static void metadata_callback(const FLAC__StreamDecoder *decoder, const ::FLAC__StreamMetadata *metadata, void *client_data);
		static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
#endif
		
		QFile		*file;
#ifdef LEGACY_FLAC
		FLAC__SeekableStreamDecoder	*flac;
#else
		FLAC__StreamDecoder	*flac;
#endif
};


#ifdef LEGACY_FLAC
FLAC__StreamDecoderWriteStatus FlacPrivate::write_callback(const FLAC__SeekableStreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
#else
FLAC__StreamDecoderWriteStatus FlacPrivate::write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
#endif
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	uint		c;
	nframes_t	pos = 0;
	nframes_t	frames = frame->header.blocksize;
	
	if (fp->bufferUsed > 0) {
		// This shouldn't be happening
		PERROR("internalBuffer is already non-empty");
	}
	
	if ((nframes_t)fp->bufferSize < frames * frame->header.channels) {
		if (fp->internalBuffer) {
			delete [] fp->internalBuffer;
		}
		fp->internalBuffer = new audio_sample_t[frames * frame->header.channels];
		fp->bufferSize = frames * frame->header.channels;
	}
	
	for (nframes_t i=0; i < frames; i++) {
		// in FLAC channel 0 is left, 1 is right
		for (c=0; c < frame->header.channels; c++) {
			audio_sample_t value = (audio_sample_t)((float)buffer[c][i] / (float)((uint)1<<(frame->header.bits_per_sample-1)));
			fp->internalBuffer[pos++] = value;
		}
	}
	
	fp->bufferUsed = frames * frame->header.channels;
	
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderReadStatus FlacPrivate::read_callback(const FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data)
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	long retval =  fp->file->read((char *)buffer, (*bytes));
	if(retval == -1) {
		return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR;
	} else {
		(*bytes) = retval;
		return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK;
	}
}
#else
FLAC__StreamDecoderReadStatus FlacPrivate::read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	long retval =  fp->file->read((char *)buffer, (*bytes));
	if(retval == -1) {
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	} else {
		(*bytes) = retval;
		return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	}
}
#endif

#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderSeekStatus FlacPrivate::seek_callback(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	if(!fp->file->seek(absolute_byte_offset))
		return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR;
	else
		return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_OK;
}
#else
FLAC__StreamDecoderSeekStatus FlacPrivate::seek_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	if(!fp->file->seek(absolute_byte_offset))
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	else
		return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}
#endif

#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderTellStatus FlacPrivate::tell_callback(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	(*absolute_byte_offset) = fp->file->pos();
	return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_OK;
}
#else
FLAC__StreamDecoderTellStatus FlacPrivate::tell_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	(*absolute_byte_offset) = fp->file->pos();
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}
#endif

#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderLengthStatus FlacPrivate::length_callback(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	(*stream_length) = fp->file->size();
	return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK;
}
#else
FLAC__StreamDecoderLengthStatus FlacPrivate::length_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	(*stream_length) = fp->file->size();
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}
#endif


#ifdef LEGACY_FLAC
void FlacPrivate::metadata_callback(const FLAC__SeekableStreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
#else
void FlacPrivate::metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
#endif
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	switch (metadata->type)
	{
		case FLAC__METADATA_TYPE_STREAMINFO:
			fp->m_channels = metadata->data.stream_info.channels;
			fp->m_rate = metadata->data.stream_info.sample_rate;
			fp->m_bitsPerSample = metadata->data.stream_info.bits_per_sample;
			fp->m_samples = metadata->data.stream_info.total_samples;
			break;
		case FLAC__METADATA_TYPE_VORBIS_COMMENT:
			//comments = new FLAC::Metadata::VorbisComment((FLAC__StreamMetadata *)metadata, true);
			break;
		default:
			break;
	}
}


#ifdef LEGACY_FLAC
FLAC__bool FlacPrivate::eof_callback(const FLAC__SeekableStreamDecoder *decoder, void *client_data)
#else
FLAC__bool FlacPrivate::eof_callback(const FLAC__StreamDecoder *decoder, void *client_data)
#endif
{
	Q_UNUSED(decoder);
	
	FlacPrivate *fp = (FlacPrivate*)client_data;
	
	return fp->file->atEnd();
}




FlacAudioReader::FlacAudioReader(QString filename)
 : AbstractAudioReader(filename)
{
	m_flac = new FlacPrivate(filename);
	
	if (m_flac->is_valid()) {
		m_channels = m_flac->m_channels;
		m_length = m_flac->m_samples;
		m_rate = m_flac->m_rate;
	}
}


FlacAudioReader::~FlacAudioReader()
{
	if (m_flac) {
		delete m_flac;
	}
}


void FlacAudioReader::clear_buffers()
{
	if (m_flac) {
		m_flac->clear_buffers();
	}
}


bool FlacAudioReader::can_decode(QString filename)
{
	if (!libFLAC_is_present) {
		return false;
	}
	
	// buffer large enough to read an ID3 tag header
	char buf[10];

	// Note: since file is created on the stack it will be closed automatically
	// by its destructor when this method (i.e. canDecode) returns.
	QFile f(filename);

	if (!f.open(QIODevice::ReadOnly)) {
		PERROR("Could not open file %s", filename.toUtf8().data());
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


bool FlacAudioReader::seek_private(nframes_t start)
{
	Q_ASSERT(m_flac);
	
	if (start >= get_length()) {
		PERROR("FlacAudioReader: could not seek to frame %d within %s, it's past the end.", start, m_fileName.toUtf8().data());
		return false;
	}
	
	m_flac->bufferUsed = 0;
	m_flac->bufferStart = 0;
	
	m_flac->flush();
	
	if (!m_flac->seek(start)) {
		PERROR("FlacAudioReader: could not seek to frame %d within %s", start, m_fileName.toUtf8().data());
		return false;
	}
	
	return true;
}


nframes_t FlacAudioReader::read_private(DecodeBuffer* buffer, nframes_t frameCount)
{
	Q_ASSERT(m_flac);
	
	nframes_t framesToCopy;
	nframes_t framesAvailable;
	nframes_t framesCoppied = 0;
	
	while (framesCoppied < frameCount) {
		if (m_flac->bufferUsed == 0) {
			// want more data
#ifdef LEGACY_FLAC
			if (m_flac->get_state() == FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM) {
				//printf("flac file finish\n");
				m_flac->flush();
				m_flac->reset();
				break;
			}
			else if(m_flac->get_state() == FLAC__SEEKABLE_STREAM_DECODER_OK) {
				//printf("process\n");
				if (!m_flac->process_single()) {
					PERROR("process_single() error\n");
					m_flac->reset();
					seek(m_readPos);
					return 0;
				}
			}
			else {
				PERROR("flac_state() = %d\n", int(m_flac->get_state()));
				m_flac->reset();
				seek(m_readPos);
				return 0;
			}
#else
			if (m_flac->get_state() == FLAC__STREAM_DECODER_END_OF_STREAM) {
				//printf("flac file finish\n");
				m_flac->flush();
				m_flac->reset();
				break;
			}
			else if(m_flac->get_state() < FLAC__STREAM_DECODER_END_OF_STREAM) {
				//printf("process\n");
				if (!m_flac->process_single()) {
					PERROR("process_single() error\n");
					m_flac->reset();
					seek(m_readPos);
					return 0;
				}
			}
			else {
				PERROR("flac_state() = %d\n", int(m_flac->get_state()));
				m_flac->reset();
				seek(m_readPos);
				return 0;
			}
#endif
		}
		
		framesAvailable = (m_flac->bufferUsed - m_flac->bufferStart) / get_num_channels() ;
		framesToCopy = (frameCount - framesCoppied < framesAvailable) ? frameCount - framesCoppied : framesAvailable;
		switch (get_num_channels()) {
			case 1:
				memcpy(buffer->destination[0] + framesCoppied, m_flac->internalBuffer + m_flac->bufferStart, framesToCopy);
				break;
			case 2:
				for (nframes_t i = 0; i < framesToCopy; i++) {
					buffer->destination[0][framesCoppied + i] = m_flac->internalBuffer[m_flac->bufferStart + i * 2];
					buffer->destination[1][framesCoppied + i] = m_flac->internalBuffer[m_flac->bufferStart + i * 2 + 1];
				}
				break;
			default:
				for (nframes_t i = 0; i < framesToCopy; i++) {
					for (int c = 0; c < get_num_channels(); c++) {
						buffer->destination[c][framesCoppied + i] = m_flac->internalBuffer[m_flac->bufferStart + i * get_num_channels() + c];
					}
				}
				break;
		}
		
		if(framesToCopy == framesAvailable) {
			m_flac->bufferUsed = 0;
			m_flac->bufferStart = 0;
		}
		else {
			m_flac->bufferStart += framesToCopy * get_num_channels();
		}
		framesCoppied += framesToCopy;
		
		//printf("samplesCoppied = %d (%d, %d)\n", samplesCoppied, m_flac->bufferStart, m_flac->buferSize);
	}
	
	// Pad end of file with 0s if necessary.  (Shouldn't be necessary...)
	/*int remainingFramesRequested = frameCount - framesCoppied;
	int remainingFramesInFile = get_length() - (m_readPos + framesCoppied);
	if (framesCoppied == 0 && remainingFramesInFile > 0) {
		int padLength = (remainingFramesRequested > remainingFramesInFile) ? remainingFramesInFile : remainingFramesRequested;
		PERROR("padLength: %d", padLength);
		for (int c = 0; c < get_num_channels(); c++) {
			memset(buffer[c] + framesCoppied, 0, padLength * sizeof(audio_sample_t));
		}
		framesCoppied += padLength;
	}
	if (framesCoppied > frameCount) {
		PERROR("Truncating");
		framesCoppied = frameCount;
	}*/
	
	//printf("copied %d of %d.  nextFrame: %lu of %lu\n", framesCoppied, frameCount, m_readPos, m_length); fflush(stdout);
	
	return framesCoppied;
}

