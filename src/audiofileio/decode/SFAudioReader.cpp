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

	m_file.setFileName(m_fileName);
	
	if (!m_file.open(QIODevice::ReadOnly)) {
		qWarning("SFAudioReader::Could not open soundfile (%s)", QS_C(m_fileName));
		return;
	}
	
	if ((m_sf = sf_open_fd (m_file.handle(), SFM_READ, &m_sfinfo, false)) == 0) {
		qWarning("SFAudioReader::Could not open soundfile (%s)", QS_C(m_fileName));
		return;
	}
 
	m_channels = m_sfinfo.channels;
	m_nframes = m_sfinfo.frames;
	m_rate = m_sfinfo.samplerate;
	m_length = TimeRef(m_nframes, m_rate);
}


SFAudioReader::~SFAudioReader()
{
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

	QFile file(filename);
	
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning("SFAudioReader::can_decode: Couldn't open soundfile (%s)", QS_C(filename));
		return false;
	}
	
	SNDFILE* sndfile = sf_open_fd(file.handle(), SFM_READ, &infos, false);
	
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
	
	
	if (start >= m_nframes) {
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


nframes_t SFAudioReader::read_private(DecodeBuffer* buffer, nframes_t frameCount)
{
	Q_ASSERT(m_sf);
	
	int framesRead = sf_readf_float(m_sf, buffer->readBuffer, frameCount);
	
	// De-interlace
	switch (m_channels) {
		case 1:
			memcpy(buffer->destination[0], buffer->readBuffer, framesRead * sizeof(audio_sample_t));
			break;	
		case 2:
			for (int f = 0; f < framesRead; f++) {
				int pos = f*2;
				buffer->destination[0][f] = buffer->readBuffer[pos];
				buffer->destination[1][f] = buffer->readBuffer[pos + 1];
			}
			break;	
		default:
			for (int f = 0; f < framesRead; f++) {
				for (int c = 0; c < m_channels; c++) {
					buffer->destination[c][f] = buffer->readBuffer[f * m_channels + c];
				}
			}
	}
	
	return framesRead;
}

