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

	SNDFILE* sndfile = sf_open(QFile::encodeName(filename), SFM_READ, &infos);
	
	//is it supported by libsndfile?
	if (!sndfile) {
		return false;
	}
	
	sf_close(sndfile);
	
	return true;
}


int SFAudioReader::get_num_channels()
{
	if (m_sf) {
		return m_sfinfo.channels;
	}
	return 0;
}


int SFAudioReader::get_length()
{
	if (m_sf) {
		return m_sfinfo.frames;
	}
	return 0;
}


int SFAudioReader::get_rate()
{
	if (m_sf) {
		return m_sfinfo.samplerate;
	}
	return 0;
}


// Should this exist?  Should we just be smarter in MonoReader so we don't need this?
bool SFAudioReader::is_compressed()
{
	return ((m_sfinfo.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_FLAC );
}


bool SFAudioReader::seek(nframes_t start)
{
	Q_ASSERT(m_sf);
	
	if (start >= get_length()) {
		return false;
	}
	
	if (sf_seek (m_sf, (off_t) start, SEEK_SET) < 0) {
		char errbuf[256];
		sf_error_str (0, errbuf, sizeof (errbuf) - 1);
		PERROR("ReadAudioSource: could not seek to frame %d within %s (%s)", start, QS_C(m_fileName), errbuf);
		return false;
	}
	
	m_nextFrame = start;

	return true;
}


int SFAudioReader::read(audio_sample_t* dst, nframes_t cnt)
{
	Q_ASSERT(m_sf);
	
	int samplesRead = sf_read_float (m_sf, dst, cnt);
	
	m_nextFrame += samplesRead / get_num_channels();
	
	return samplesRead;
}

