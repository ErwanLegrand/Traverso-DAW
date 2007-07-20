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
	m_file = fopen(filename.toUtf8().data(), "rb");
	if (!m_file) {
		PERROR("Couldn't open file %s.", QS_C(filename));
		return;
	}
	
	if (ov_open(m_file, &m_vf, 0, 0) < 0) {
		PERROR("Input does not appear to be an Ogg bitstream.");
		fclose(m_file);
		return;
	}

	ov_pcm_seek(&m_vf, 0);
	m_vi = ov_info(&m_vf,-1);
}


VorbisAudioReader::~VorbisAudioReader()
{
	if (m_file) {
		ov_clear(&m_vf);
	}
}


bool VorbisAudioReader::can_decode(QString filename)
{
	FILE* file = fopen(filename.toUtf8().data(), "rb");
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
	
	return true;
}


int VorbisAudioReader::get_num_channels()
{
	if (m_file) {
		return m_vi->channels;
	}
	return 0;
}


nframes_t VorbisAudioReader::get_length()
{
	if (m_file) {
		return ov_pcm_total(&m_vf, -1);
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


bool VorbisAudioReader::seek_private(nframes_t start)
{
	Q_ASSERT(m_file);
	
	if (start >= get_length()) {
		return false;
	}
	
	if (int result = ov_pcm_seek(&m_vf, start) < 0) {
		PERROR("VorbisAudioReader: could not seek to frame %d within %s (%d)", start, QS_C(m_fileName), result);
		return false;
	}
	
	return true;
}


nframes_t VorbisAudioReader::read_private(audio_sample_t** buffer, nframes_t frameCount)
{
	Q_ASSERT(m_file);
	
	nframes_t totalFramesRead = 0;
	
	while (totalFramesRead < frameCount) {
		audio_sample_t** tmp;
		int bs;
		nframes_t framesRead = ov_read_float(&m_vf, &tmp, frameCount - totalFramesRead, &bs);
		
		if (framesRead == OV_HOLE) {
			// Hole detected: recursive retry
			PERROR("VorbisAudioReader: OV_HOLE");
			return read(buffer, frameCount);
		}
		else if (framesRead == 0) {
			/* EOF */
			break;
		} else if (framesRead < 0) {
			/* error in the stream. */
			PERROR("VorbisFile decoding error");
			break;
		}
		
		for (int c=0; c < get_num_channels(); c++) {
			memcpy(buffer[c] + totalFramesRead, tmp[c], framesRead * sizeof(audio_sample_t));
		}
		totalFramesRead += framesRead;
	}
	
	return totalFramesRead;
}

