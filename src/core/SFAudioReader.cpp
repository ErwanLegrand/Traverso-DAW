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

#include "SFAudioReader.h"
#include <QFile>
#include <QString>
#include "Utils.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


SFAudioReader::SFAudioReader(QString filename)
 : AbstractAudioReader(filename)
{
	/* although libsndfile says we don't need to set this,
	valgrind and source code shows us that we do.
	Really? Look it up !
	*/
	memset (&m_sfinfo, 0, sizeof(m_sfinfo));

	if ((m_sf = sf_open ((m_fileName.toUtf8().data()), SFM_READ, &m_sfinfo)) == 0) {
		PERROR("Couldn't open soundfile (%s)", QS_C(m_fileName));
	}
	
	m_channels = m_sfinfo.channels;
	m_length = m_sfinfo.frames;
	m_rate = m_sfinfo.samplerate;
	
	m_tmpBuffer = 0;
	m_tmpBufferSize = 0;
}


SFAudioReader::~SFAudioReader()
{
	if (m_tmpBuffer) {
		delete m_tmpBuffer;
	}
	
	if (m_sf) {
		if (sf_close(m_sf)) {
			qWarning("sf_close returned an error!");
		}
	}
}


bool SFAudioReader::can_decode(QString filename)
{
	SF_INFO infos;

	/* although libsndfile says we don't need to set this,
	valgrind and source code shows us that we do.
	Really? Look it up !
	*/
	memset (&infos, 0, sizeof(infos));

	SNDFILE* sndfile = sf_open(QFile::encodeName(filename), SFM_READ, &infos);
	
	//is it supported by libsndfile?
	if (!sndfile) {
		return false;
	}
	
	sf_close(sndfile);
	
	return true;
}


bool SFAudioReader::seek_private(nframes_t start)
{
	Q_ASSERT(m_sf);
	
	
	if (start >= m_length) {
		return false;
	}
	
	if (sf_seek (m_sf, (off_t) start, SEEK_SET) < 0) {
		char errbuf[256];
		sf_error_str (0, errbuf, sizeof (errbuf) - 1);
		PERROR("ReadAudioSource: could not seek to frame %d within %s (%s)", start, QS_C(m_fileName), errbuf);
		return false;
	}
	
	return true;
}


nframes_t SFAudioReader::read_private(audio_sample_t** buffer, nframes_t frameCount)
{
	Q_ASSERT(m_sf);
	
	// Make sure the temp buffer is big enough for this read
	if (m_tmpBufferSize < frameCount) {
		if (m_tmpBuffer) {
			delete m_tmpBuffer;
		}
		m_tmpBuffer = new audio_sample_t[frameCount * m_channels];
	}
	nframes_t framesRead = sf_readf_float(m_sf, m_tmpBuffer, frameCount);
	
	// De-interlace
	switch (m_channels) {
		case 1:
			memcpy(buffer[0], m_tmpBuffer, framesRead * sizeof(audio_sample_t));
			break;	
		case 2:
			for (int f = 0; f < framesRead; f++) {
				buffer[0][f] = m_tmpBuffer[f * 2];
				buffer[1][f] = m_tmpBuffer[f * 2 + 1];
			}
			break;	
		default:
			for (int f = 0; f < framesRead; f++) {
				for (int c = 0; c < m_channels; c++) {
					buffer[c][f] = m_tmpBuffer[f * m_channels + c];
				}
			}
	}
	
	return framesRead;
}

