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
#include <AbstractAudioWriter.h>
#include <SFAudioWriter.h>
#include "Peak.h"
#include "Utils.h"
#include "DiskIO.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


WriteSource::WriteSource( ExportSpecification* specification )
	: AudioSource(specification->exportdir, specification->name)
	, m_spec(specification)
{
	m_diskio = 0;
	m_writer = 0;
	m_peak = 0;
}

WriteSource::~WriteSource()
{
	PENTERDES;
	if (m_peak) {
		delete m_peak;
	}
	
	for(int i=0; i<m_buffers.size(); ++i) {
		delete m_buffers.at(i);
	}
	
	if (m_spec->isRecording) {
		delete m_spec;
	}
	if (m_writer) {
		delete m_writer;
	}
}

int WriteSource::process (nframes_t nframes)
{
	float* float_buffer = 0;
	int chn;
	uint32_t x;
	uint32_t i;
	nframes_t written;
	nframes_t to_write = 0;
	int cnt = 0;

	// nframes MUST be greater then 0, this is a precondition !
	Q_ASSERT(nframes);


	do {

		/* now do sample rate conversion */

		if (m_sampleRate != (uint)m_spec->sample_rate) {

			int err;

			m_src_data.output_frames = m_out_samples_max / m_channelCount;
			int rate = audiodevice().get_sample_rate();
			m_src_data.end_of_input = (m_spec->pos + TimeRef(nframes, rate)) >= m_spec->endLocation;
			m_src_data.data_out = m_dataF2;

			if (m_leftover_frames > 0) {

				/* input data will be in m_leftoverF rather than dataF */

				m_src_data.data_in = m_leftoverF;

				if (cnt == 0) {

					/* first time, append new data from dataF into the m_leftoverF buffer */

					memcpy (m_leftoverF + (m_leftover_frames * m_channelCount), m_spec->dataF, nframes * m_channelCount * sizeof(float));
					m_src_data.input_frames = nframes + m_leftover_frames;
				} else {

					/* otherwise, just use whatever is still left in m_leftoverF; the contents
					were adjusted using memmove() right after the last SRC call (see
					below)
					*/

					m_src_data.input_frames = m_leftover_frames;
				}
			} else {

				m_src_data.data_in = m_spec->dataF;
				m_src_data.input_frames = nframes;

			}

			++cnt;

			if ((err = src_process (m_src_state, &m_src_data)) != 0) {
				PWARN(("an error occured during sample rate conversion: %s"), src_strerror (err));
				return -1;
			}

			to_write = m_src_data.output_frames_gen;
			m_leftover_frames = m_src_data.input_frames - m_src_data.input_frames_used;

			if (m_leftover_frames > 0) {
				if (m_leftover_frames > m_max_leftover_frames) {
					PWARN("warning, leftover frames overflowed, glitches might occur in output");
					m_leftover_frames = m_max_leftover_frames;
				}
				memmove (m_leftoverF, (char *) (m_src_data.data_in + (m_src_data.input_frames_used * m_channelCount)),
					 m_leftover_frames * m_channelCount * sizeof(float));
			}

			float_buffer = m_dataF2;

		} else {

			/* no SRC, keep it simple */

			to_write = nframes;
			m_leftover_frames = 0;
			float_buffer = m_spec->dataF;
		}

		if (m_output_data) {
			memset (m_output_data, 0, m_sample_bytes * to_write * m_channelCount);
		}

		switch (m_spec->data_width) {
		case 8:
		case 16:
		case 24:
			for (chn = 0; chn < m_channelCount; ++chn) {
				gdither_runf (m_dither, chn, to_write, float_buffer, m_output_data);
			}
			/* and export to disk */
			written = m_writer->write(m_output_data, to_write);
			break;

		case 32:
			for (chn = 0; chn < m_channelCount; ++chn) {

				int *ob = (int *) m_output_data;
				const double int_max = (float) INT_MAX;
				const double int_min = (float) INT_MIN;

				for (x = 0; x < to_write; ++x) {
					i = chn + (x * m_channelCount);

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
			/* and export to disk */
			written = m_writer->write(m_output_data, to_write);
			break;

		default:
			for (x = 0; x < to_write * m_channelCount; ++x) {
				if (float_buffer[x] > 1.0f) {
					float_buffer[x] = 1.0f;
				} else if (float_buffer[x] < -1.0f) {
					float_buffer[x] = -1.0f;
				}
			}
			/* and export to disk */
			written = m_writer->write(float_buffer, to_write);
			break;
		}

	} while (m_leftover_frames >= nframes);

	return 0;
}

int WriteSource::prepare_export()
{
	PENTER;
	
	Q_ASSERT(m_spec->is_valid() == 1);
	
	GDitherSize dither_size;

	m_sampleRate = audiodevice().get_sample_rate();
	m_channelCount = m_spec->channels;
	m_processPeaks = false;
	m_diskio = 0;
	m_dataF2 = m_leftoverF = 0;
	m_dither = 0;
	m_output_data = 0;
	m_src_state = 0;
	

	switch (m_spec->data_width) {
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

	if (m_writer) {
		delete m_writer;
	}
	
	m_writer = AbstractAudioWriter::create_audio_writer(m_spec->writerType);
	m_writer->set_rate(m_spec->sample_rate);
	m_writer->set_bits_per_sample(m_spec->data_width);
	m_writer->set_num_channels(m_channelCount);
	
	QString key;
	foreach (key, m_spec->extraFormat.keys()) {
		if (m_writer->set_format_attribute(key, m_spec->extraFormat[key]) == false) {
			printf("Invalid Extra Format Info: %s=%s\n", QS_C(key), QS_C(m_spec->extraFormat[key]));
		}
	}
	
	/* XXX make sure we have enough disk space for the output */
	
	m_fileName.append(m_writer->get_extension());
	
	if (m_writer->open(m_fileName) == false) {
		return -1;
	}
	
	if ((uint)m_spec->sample_rate != m_sampleRate) {
		qDebug("Doing samplerate conversion");
		int err;

		if ((m_src_state = src_new (m_spec->src_quality, m_channelCount, &err)) == 0) {
			PWARN("cannot initialize sample rate conversion: %s", src_strerror (err));
			return -1;
		}

		m_src_data.src_ratio = m_spec->sample_rate / (double) m_sampleRate;
		m_out_samples_max = (nframes_t) ceil (m_spec->blocksize * m_src_data.src_ratio * m_channelCount);
		m_dataF2 = new audio_sample_t[m_out_samples_max];

		m_max_leftover_frames = 4 * m_spec->blocksize;
		m_leftoverF = new audio_sample_t[m_max_leftover_frames * m_channelCount];
		m_leftover_frames = 0;
	} else {
		m_out_samples_max = m_spec->blocksize * m_channelCount;
	}

	m_dither = gdither_new (m_spec->dither_type, m_channelCount, dither_size, m_spec->data_width);

	/* allocate buffers where dithering and output will occur */

	switch (m_spec->data_width) {
	case 8:
		m_sample_bytes = 1;
		break;

	case 16:
		m_sample_bytes = 2;
		break;

	case 24:
	case 32:
		m_sample_bytes = 4;
		break;

	default:
		m_sample_bytes = 0; // float format
		break;
	}

	if (m_sample_bytes) {
		m_output_data = (void*) malloc (m_sample_bytes * m_out_samples_max);
	}

	return 0;
}


int WriteSource::finish_export( )
{
	PENTER;

	if (m_writer) {
		m_writer->close();
		delete m_writer;
		m_writer = 0;
	}
	
	if (m_dataF2)
		delete m_dataF2;
	if (m_leftoverF)
		delete m_leftoverF;

	if (m_dither) {
		gdither_free (m_dither);
		m_dither = 0;
	}

	if (m_output_data) {
		free (m_output_data);
		m_output_data = 0;
	}

	if (m_src_state) {
		src_delete (m_src_state);
		m_src_state = 0;
	}

	if (m_peak && m_peak->finish_processing() < 0) {
		PERROR("WriteSource::finish_export : peak->finish_processing() failed!");
	}
		
	if (m_diskio) {
		m_diskio->unregister_write_source(this);
	}


	printf("WriteSource :: thread id is: %ld\n", QThread::currentThreadId ());
	PWARN("WriteSource :: emiting exportFinished");
	emit exportFinished();

	return 1;
}

int WriteSource::rb_write(audio_sample_t** src, nframes_t cnt)
{
	int written = 0;
	
	for (int chan=m_channelCount-1; chan>=0; --chan) {
		written = m_buffers.at(chan)->write(src[chan], cnt);
	}
	
	return written;
}

void WriteSource::set_process_peaks( bool process )
{
	m_processPeaks = process;
	
	if (!m_processPeaks) {
		return;
	}
	
	Q_ASSERT(!m_peak);
	
	m_peak = new Peak(this);

	if (m_peak->prepare_processing(audiodevice().get_sample_rate()) < 0) {
		PERROR("Cannot process peaks realtime");
		m_processPeaks = false;
		delete m_peak;
		m_peak = 0;
		
		return;
	}
}

int WriteSource::rb_file_write(nframes_t cnt)
{
	uint read = 0;
	int chan;
	
	audio_sample_t* readbuffer[m_channelCount];
	
	for (chan=0; chan<m_channelCount; ++chan) {
		
		readbuffer[chan] = new audio_sample_t[cnt * m_channelCount];

		read = m_buffers.at(chan)->read(readbuffer[chan], cnt);
		
		if (read != cnt) {
			printf("WriteSource::rb_file_write() : could only process %d frames, %d were requested!\n", read, cnt);
		}
		
		m_peak->process(chan, readbuffer[chan], read);
	}

	if (read > 0) {
		if (m_channelCount == 1) {
			m_spec->dataF = readbuffer[0];
		} else {
			// Interlace data into dataF buffer!
			for (uint f=0; f<read; f++) {
				for (chan = 0; chan < m_channelCount; chan++) {
					m_spec->dataF[f * m_channelCount + chan] = readbuffer[chan][f];
				}
			}
		}
		
		process(read);
	}
	
	for (chan=0; chan<m_channelCount; ++chan) {
		delete [] readbuffer[chan];
	}
	
	return read;
}

void WriteSource::set_recording( int rec )
{
	m_isRecording = rec;
}

void WriteSource::process_ringbuffer(audio_sample_t* buffer)
{
	m_spec->dataF = buffer;
	int readSpace = m_buffers.at(0)->read_space();

	if (! m_isRecording ) {
		PMESG("Writing remaining  (%d) samples to ringbuffer", readSpace);
		rb_file_write(readSpace);
		finish_export();
		return;
	}

	rb_file_write(readSpace);
}

void WriteSource::prepare_rt_buffers( )
{
	m_bufferSize = m_sampleRate * DiskIO::writebuffertime;
	m_chunkSize = m_bufferSize / DiskIO::bufferdividefactor;
	for (int i=0; i<m_channelCount; ++i) {
		m_buffers.append(new RingBufferNPT<audio_sample_t>(m_bufferSize));
	}
}

void WriteSource::set_diskio( DiskIO * io )
{
	m_diskio = io;
	prepare_rt_buffers();
}

//eof

