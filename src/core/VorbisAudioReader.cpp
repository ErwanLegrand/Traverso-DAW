/*
Copyright (C) 2007 Ben Levitt 

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

#include "VorbisAudioReader.h"
#include <QFile>
#include <QString>
#include "Utils.h"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


VorbisAudioReader::VorbisAudioReader(QString filename)
 : AbstractAudioReader(filename)
{
	m_file = fopen(QFile::encodeName(filename), "r");
	if (!m_file) {
		PERROR("Couldn't open file %s.", QS_C(filename));
		return;
	}
	
	if (ov_open(m_file, &m_vf, NULL, 0) < 0) {
		PERROR("Input does not appear to be an Ogg bitstream.");
		return;
	}

	m_vi = ov_info(&m_vf,-1);
}


VorbisAudioReader::~VorbisAudioReader()
{
	if (m_file) {
		ov_clear(&m_vf);
		fclose(m_file);
	}
}


bool VorbisAudioReader::can_decode(QString filename)
{
	FILE* file = fopen(QFile::encodeName(filename), "r");
	if (!file) {
		PERROR("Could not open file: %s.", QS_C(filename));
		return false;
	}
	
	OggVorbis_File of;
	
	if (ov_test(file, &of, 0, 0)) {
		fclose(file);
		return false;
	}
	
	ov_clear(&of);
	fclose(file);
	
	return true;
}


int VorbisAudioReader::get_num_channels()
{
	if (m_file) {
		return m_vi->channels;
	}
	return 0;
}


int VorbisAudioReader::get_length()
{
	if (m_file) {
		return ov_pcm_total(&m_vf, -1) / get_num_channels();
	}
	return 0;
}


int VorbisAudioReader::get_rate()
{
	if (m_file) {
		return m_vi->rate;
	}
	return 0;
}


// Should this exist?  Should we just be smarter in MonoReader so we don't need this?
bool VorbisAudioReader::is_compressed()
{
	return false;
}


bool VorbisAudioReader::seek(nframes_t start)
{
	Q_ASSERT(m_file);
	
	if (start >= get_length()) {
		return false;
	}
	//printf("seek to %lu\n", start);
	if (ov_pcm_seek(&m_vf, start) < 0) {
		PERROR("VorbisAudioReader: could not seek to frame %d within %s", start, QS_C(m_fileName));
		return false;
	}
	
	m_nextFrame = start;

	return true;
}


int VorbisAudioReader::read(audio_sample_t* dst, nframes_t cnt)
{
	Q_ASSERT(m_file);
	
	audio_sample_t** tmp;
	int bs;
	
	int samplesRead = ov_read_float (&m_vf, &tmp, cnt, &bs);
	
	if (samplesRead == OV_HOLE) {
		PERROR("VorbisAudioReader: OV_HOLE");
		// recursive new try
		return read(dst, cnt);
	}
	else if (samplesRead == 0) {
		/* EOF */
		return 0;
	} else if (samplesRead < 0) {
		/* error in the stream. */
		return 0;
	}
	
	int frames = samplesRead/get_num_channels();
	for (int f=0; f < frames; f++) {
		for (int c=0; c < get_num_channels(); c++) {
			dst[f * get_num_channels() + c] = tmp[c][f];
		}
	}
	
	//printf("SFAudioReader: cnt = %lu, samplesRead = %d, length = %lu\n", cnt, samplesRead, get_length());
	
	// m_nextFrame currently exists just for debugging
	m_nextFrame += frames;
	
	return samplesRead;
}

