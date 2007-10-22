/*
Copyright (C) 2007 Ben Levitt 
 * Based on the ogg encoder module for K3b.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>

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

#include "FlacAudioWriter.h"

#include <stdio.h>
#include "FLAC/export.h"

#if !defined FLAC_API_VERSION_CURRENT || FLAC_API_VERSION_CURRENT < 6
#define LEGACY_FLAC
#include "FLAC/file_encoder.h"
#else
#undef LEGACY_FLAC
#include "FLAC/stream_encoder.h"
#endif

RELAYTOOL_FLAC;

#include <QString>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


class FlacAudioWriter::Private
{
public:
	Private()
		: encoder(0),
		quality(5),
		buffer(0),
		bufferSize(0) {
	}
	~Private() {
		if (buffer) {
			delete [] buffer;
		}
	}
	
#ifdef LEGACY_FLAC
	FLAC__FileEncoder *encoder;
#else
	FLAC__StreamEncoder *encoder;
#endif
	int quality;
	
	int32_t	*buffer;
	long	bufferSize;
};


FlacAudioWriter::FlacAudioWriter()
 : AbstractAudioWriter()
{
	d = new Private();
}


FlacAudioWriter::~FlacAudioWriter()
{
	cleanup();
	delete d;
}


const char* FlacAudioWriter::get_extension()
{
	return ".flac";
}


bool FlacAudioWriter::set_format_attribute(const QString& key, const QString& value)
{
	return false;
}


bool FlacAudioWriter::open_private()
{
	cleanup();
	
	/* allocate the encoder */
#ifdef LEGACY_FLAC
	if ((d->encoder = FLAC__file_encoder_new()) == NULL) {
#else
	if ((d->encoder = FLAC__stream_encoder_new()) == NULL) {
#endif
		PERROR("ERROR: allocating encoder");
		return false;
	}
	
	bool ok = true;
	
#ifdef LEGACY_FLAC
	//ok &= FLAC__file_encoder_set_compression_level(d->encoder, d->quality);
	ok &= FLAC__file_encoder_set_channels(d->encoder, m_channels);
	ok &= FLAC__file_encoder_set_bits_per_sample(d->encoder, m_sampleWidth);
	ok &= FLAC__file_encoder_set_sample_rate(d->encoder, m_rate);
	ok &= FLAC__file_encoder_set_total_samples_estimate(d->encoder, 0);  // Set when done
	ok &= FLAC__file_encoder_set_filename(d->encoder, m_fileName.toUtf8().data());
#else
	ok &= FLAC__stream_encoder_set_compression_level(d->encoder, d->quality);
	ok &= FLAC__stream_encoder_set_channels(d->encoder, m_channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(d->encoder, m_sampleWidth);
	ok &= FLAC__stream_encoder_set_sample_rate(d->encoder, m_rate);
	ok &= FLAC__stream_encoder_set_total_samples_estimate(d->encoder, 0);  // Set when done
#endif
	
	/* initialize encoder */
	if (ok) {
#ifdef LEGACY_FLAC
		FLAC__FileEncoderState init_status;
		init_status = FLAC__file_encoder_init(d->encoder);
		if (init_status != FLAC__FILE_ENCODER_OK) {
			PERROR("ERROR: initializing encoder: %s", FLAC__FileEncoderStateString[init_status]);
			ok = false;
		}
#else
		FLAC__StreamEncoderInitStatus init_status;
		init_status = FLAC__stream_encoder_init_file(d->encoder, m_fileName.toUtf8().data(), NULL, /*client_data=*/NULL);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			PERROR("ERROR: initializing encoder: %s", FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
#endif
	}
	
	return ok;
}


nframes_t FlacAudioWriter::write_private(void* buffer, nframes_t frameCount)
{
	FLAC__byte *data = (FLAC__byte *)buffer;
	
	if (d->bufferSize < (long)(frameCount * m_channels)) {
		if (d->buffer) {
			delete [] d->buffer;
		}
		d->bufferSize = frameCount * m_channels;
		d->buffer = new int32_t[d->bufferSize];
	}
	
	if (m_sampleWidth == 8) {
		for (nframes_t i = 0; i < frameCount * m_channels; i++) {
			d->buffer[i] = data[i];
		}
	}
	else if (m_sampleWidth == 16) {
		for (nframes_t i = 0; i < frameCount * m_channels; i++) {
			d->buffer[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)data[2*i+1] << 8) | (FLAC__int16)data[2*i]);
		}
	}
	else if (m_sampleWidth == 24 || m_sampleWidth == 32) {
		for (nframes_t i = 0; i < frameCount * m_channels; i++) {
			d->buffer[i] = (FLAC__int64)(((FLAC__int32)(((FLAC__int16)(FLAC__int8)data[4*i+1] << 8) | (FLAC__int16)data[4*i]) << 16) |
			(FLAC__int64)(((FLAC__int16)(FLAC__int8)data[4*i+3] << 8) | (FLAC__int16)data[4*i+2]));
		}
	}
	
#ifdef LEGACY_FLAC
	bool ok = FLAC__file_encoder_process_interleaved(d->encoder, d->buffer, frameCount);
#else
	bool ok = FLAC__stream_encoder_process_interleaved(d->encoder, d->buffer, frameCount);
#endif
	
	return ((ok) ? frameCount : 0);
}


void FlacAudioWriter::cleanup()
{
	if (d->encoder) {
#ifdef LEGACY_FLAC
		FLAC__file_encoder_delete(d->encoder);
#else
		FLAC__stream_encoder_delete(d->encoder);
#endif
		d->encoder = 0;
	}
}


bool FlacAudioWriter::close_private()
{
#ifdef LEGACY_FLAC
	FLAC__file_encoder_finish(d->encoder);
	bool success = (FLAC__file_encoder_get_state(d->encoder) == FLAC__FILE_ENCODER_UNINITIALIZED);
#else
	bool success = FLAC__stream_encoder_finish(d->encoder);
#endif
	
	// FIXME: Rewrite the header to store the real length (num samples)
	
	cleanup();
	
	return success;
}

