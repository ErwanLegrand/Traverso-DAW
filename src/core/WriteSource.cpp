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

#include "WriteSource.h"

#include "Export.h"
#include <math.h>

#include <AudioDevice.h>
#include "Peak.h"
#include "Utils.h"
#include "DiskIO.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


WriteSource::WriteSource( ExportSpecification * specification )
	: AudioSource(specification->exportdir, specification->name)
	, spec(specification)
{
	diskio = 0;
	m_buffer = 0;
	prepare_export(spec);
}

WriteSource::WriteSource( ExportSpecification * specification, int channelNumber, int superChannelCount )
		: AudioSource(specification->exportdir, specification->name),
		spec(specification),
		m_channelNumber(channelNumber)
{
	diskio = 0;
	m_buffer = 0;
	m_channelCount = superChannelCount;
	prepare_export(spec);
}

WriteSource::~WriteSource()
{
	PENTERDES;
	if (m_peak) {
		delete m_peak;
	}
	
	if (m_buffer) {
		delete m_buffer;
	}
		
	//FIXME spec can be owned by someone else!!!! (ExportWidget for example)
/*	if (spec) {
		delete spec;
		spec = 0;
	}*/
}

int WriteSource::process (nframes_t nframes)
{
	float* float_buffer = 0;
	uint32_t chn;
	uint32_t x;
	uint32_t i;
	sf_count_t written;
	char errbuf[256];
	nframes_t to_write = 0;
	int cnt = 0;

	// nframes MUST be greater then 0, this is a precondition !
	Q_ASSERT(nframes);


	do {

		/* now do sample rate conversion */

		if (sample_rate != (uint)spec->sample_rate) {

			int err;

			src_data.output_frames = out_samples_max / channels;
			src_data.end_of_input = ((spec->pos + nframes) >= spec->end_frame);
			src_data.data_out = dataF2;

			if (leftover_frames > 0) {

				/* input data will be in leftoverF rather than dataF */

				src_data.data_in = leftoverF;

				if (cnt == 0) {

					/* first time, append new data from dataF into the leftoverF buffer */

					memcpy (leftoverF + (leftover_frames * channels), spec->dataF, nframes * channels * sizeof(float));
					src_data.input_frames = nframes + leftover_frames;
				} else {

					/* otherwise, just use whatever is still left in leftoverF; the contents
					were adjusted using memmove() right after the last SRC call (see
					below)
					*/

					src_data.input_frames = leftover_frames;
				}
			} else {

				src_data.data_in = spec->dataF;
				src_data.input_frames = nframes;

			}

			++cnt;

			if ((err = src_process (src_state, &src_data)) != 0) {
				PWARN(("an error occured during sample rate conversion: %s"), src_strerror (err));
				return -1;
			}

			to_write = src_data.output_frames_gen;
			leftover_frames = src_data.input_frames - src_data.input_frames_used;

			if (leftover_frames > 0) {
				if (leftover_frames > max_leftover_frames) {
					PWARN("warning, leftover frames overflowed, glitches might occur in output");
					leftover_frames = max_leftover_frames;
				}
				memmove (leftoverF, (char *) (src_data.data_in + (src_data.input_frames_used * channels)),
					leftover_frames * channels * sizeof(float));
			}

			float_buffer = dataF2;

		} else {

			/* no SRC, keep it simple */

			to_write = nframes;
			leftover_frames = 0;
			float_buffer = spec->dataF;
		}

		if (output_data) {
			memset (output_data, 0, sample_bytes * to_write * channels);
		}

		switch (spec->data_width) {
		case 8:
		case 16:
		case 24:
			for (chn = 0; chn < channels; ++chn) {
				gdither_runf (dither, chn, to_write, float_buffer, output_data);
			}
			break;

		case 32:
			for (chn = 0; chn < channels; ++chn) {

				int *ob = (int *) output_data;
				const double int_max = (float) INT_MAX;
				const double int_min = (float) INT_MIN;

				for (x = 0; x < to_write; ++x) {
					i = chn + (x * channels);

					if (float_buffer[i] > 1.0f) {
						ob[i] = INT_MAX;
					} else if (float_buffer[i] < -1.0f) {
						ob[i] = INT_MIN;
					} else {
						if (float_buffer[i] >= 0.0f) {
							ob[i] = lrintf (int_max * float_buffer[i]);
						} else {
							ob[i] = - lrintf (int_min * float_buffer[i]);
						}
					}
				}
			}
			break;

		default:
			if (processPeaks) {
				m_peak->process( float_buffer, to_write );
			}

			for (x = 0; x < to_write * channels; ++x) {
				if (float_buffer[x] > 1.0f) {
					float_buffer[x] = 1.0f;
				} else if (float_buffer[x] < -1.0f) {
					float_buffer[x] = -1.0f;
				}
			}
			break;
		}

		/* and export to disk */

		switch (spec->data_width) {
		case 8:
			written = sf_write_raw (sf, (void*) output_data, to_write * channels);
			break;

		case 16:
			written = sf_writef_short (sf, (short*) output_data, to_write);
			break;

		case 24:
		case 32:
			written = sf_writef_int (sf, (int*) output_data, to_write);
			break;

		default:
			written = sf_writef_float (sf, float_buffer, to_write);
			break;
		}

		if ((nframes_t) written != to_write) {
			sf_error_str (sf, errbuf, sizeof (errbuf) - 1);
			printf(("Export: could not write data to output file (%s)\n"), errbuf);
			return -1;
		}


	} while (leftover_frames >= nframes);

	return 0;
}

int WriteSource::prepare_export (ExportSpecification* spec)
{
	PENTER;
	
	Q_ASSERT(spec->is_valid() == 1);
	
	char errbuf[256];
	GDitherSize dither_size;

	sample_rate = audiodevice().get_sample_rate();
	channels = spec->channels;
	processPeaks = false;
	m_peak = 0;
	diskio = 0;
	dataF2 = leftoverF = 0;
	dither = 0;
	output_data = 0;
	src_state = 0;
	

	switch (spec->data_width) {
	case 8:
		dither_size = GDither8bit;
		break;

	case 16:
		dither_size = GDither16bit;
		break;

	case 24:
		dither_size = GDither32bit;
		break;

	default:
		dither_size = GDitherFloat;
		break;
	}

	memset (&sfinfo, 0, sizeof(sfinfo));

	sfinfo.format = spec->format;
	sfinfo.samplerate = spec->sample_rate;
	sfinfo.frames = spec->end_frame - spec->start_frame + 1;
	sfinfo.channels = spec->channels;

	if (sf_format_check(&sfinfo) == false) {
		PWARN("sf_format_check returned false");
	}


	/* XXX make sure we have enough disk space for the output */

	QString name = m_fileName;
	if (spec->isRecording) {
		name.append("-ch" + QByteArray::number(m_channelNumber) + ".wav");
	}
	
	if ((sf = sf_open(name.toUtf8().data(), SFM_WRITE, &sfinfo)) == 0) {
		sf_error_str (0, errbuf, sizeof (errbuf) - 1);
		PWARN("Export: cannot open output file \"%s\" (%s)", QS_C(m_fileName), errbuf);
		return -1;
	}


	if ((uint)spec->sample_rate != sample_rate) {
		qDebug("Doing samplerate conversion");
		int err;

		if ((src_state = src_new (spec->src_quality, channels, &err)) == 0) {
			PWARN("cannot initialize sample rate conversion: %s", src_strerror (err));
			return -1;
		}

		src_data.src_ratio = spec->sample_rate / (double) sample_rate;
		out_samples_max = (nframes_t) ceil (spec->blocksize * src_data.src_ratio * channels);
		dataF2 = new audio_sample_t[out_samples_max];

		max_leftover_frames = 4 * spec->blocksize;
		leftoverF = new audio_sample_t[max_leftover_frames * channels];
		leftover_frames = 0;
	} else {
		out_samples_max = spec->blocksize * channels;
	}

	dither = gdither_new (spec->dither_type, channels, dither_size, spec->data_width);

	/* allocate buffers where dithering and output will occur */

	switch (spec->data_width) {
	case 8:
		sample_bytes = 1;
		break;

	case 16:
		sample_bytes = 2;
		break;

	case 24:
	case 32:
		sample_bytes = 4;
		break;

	default:
		sample_bytes = 0; // float format
		break;
	}

	if (sample_bytes) {
		output_data = (void*) malloc (sample_bytes * out_samples_max);
	}

	return 0;
}


int WriteSource::finish_export( )
{
	PENTER;

	if (sf_close (sf)) {
		qWarning("sf_close returned an error!");
	}
	sf = 0;

	if (dataF2)
		delete dataF2;
	if (leftoverF)
		delete leftoverF;

	if (dither) {
		gdither_free (dither);
		dither = 0;
	}

	if (output_data) {
		free (output_data);
		output_data = 0;
	}

	if (src_state) {
		src_delete (src_state);
		src_state = 0;
	}

/*	if (processPeaks) {
		m_peak->finish_processing();
	}*/
		
	if (diskio) {
		diskio->unregister_write_source(this);
	}


	printf("WriteSource :: thread id is: %ld\n", QThread::currentThreadId ());
	PWARN("WriteSource :: emiting exportFinished");
	emit exportFinished( this );

	return 1;
}

int WriteSource::rb_write(audio_sample_t* src, nframes_t cnt )
{
	int written = m_buffer->write(src, cnt);
	return written;
}

void WriteSource::set_process_peaks( bool process )
{
	processPeaks = process;
	if (!processPeaks) {
		return;
	}
	
	Q_ASSERT(!m_peak);
	
	m_peak = new Peak(this, m_channelNumber);
	
	if (m_peak->prepare_processing() < 0) {
		PERROR("Cannot process peaks realtime");
		delete m_peak;
		m_peak = 0;
		processPeaks = false;
	}
}

int WriteSource::rb_file_write( nframes_t cnt )
{
	int read = m_buffer->read(spec->dataF, cnt);

	if (read > 0) {
		process(read);
	}

	return read;
}

void WriteSource::set_recording( int rec )
{
	m_isRecording = rec;
}

void WriteSource::process_ringbuffer( audio_sample_t* framebuffer, bool seek)
{
	Q_UNUSED(seek);

	spec->dataF = framebuffer;
	int readSpace = m_buffer->read_space();

	if (! m_isRecording ) {
		PWARN("Writing remaining  (%d) samples to ringbuffer", readSpace);
		rb_file_write(readSpace);
		PWARN("WriteSource :: calling source->finish_export()");
		finish_export();
		return;
	}

	rb_file_write(readSpace);
}

void WriteSource::prepare_buffer( )
{
	m_buffersize = sample_rate * DiskIO::writebuffertime;
	m_chunksize = m_buffersize / DiskIO::bufferdividefactor;
	m_buffer = new RingBufferNPT<audio_sample_t>(m_buffersize);
}

void WriteSource::set_diskio( DiskIO * io )
{
	diskio = io;
}

//eof

