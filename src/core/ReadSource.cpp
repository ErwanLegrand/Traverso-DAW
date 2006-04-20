/*
    Copyright (C) 2006 Remon Sijrier 
 
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
 
    $Id: ReadSource.cpp,v 1.1 2006/04/20 14:51:40 r_sijrier Exp $
*/

#include "ReadSource.h"
#include "AudioSource.h"

#include "Peak.h"
#include "RingBuffer.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


// This constructor is called for existing (recorded/imported) audio sources
ReadSource::ReadSource(const QDomNode node)
                : AudioSource(node)
{
        init();
}

ReadSource::ReadSource(uint chan, QString dir, QString name)
                : AudioSource(chan, dir, name)
{
        init();
}

ReadSource::ReadSource( AudioSource * source )
                : AudioSource(source->get_channel(), source->get_dir(), source->get_name())
{
        m_peak = source->get_peak();
        m_peak->set_audiosource(this);
        source->set_peak(0);
        init();
}

ReadSource::~ReadSource()
{
        PENTERDES;
        if (tmpbuf)
                delete [] tmpbuf;
}


int ReadSource::init( )
{
        PENTER;
        rbFileReadPos = 0;
        rbRelativeFileReadPos = 0;
        rbReady = true;
        needSync = false;
        sourceStartOffset = 0;
        tmpbuf = 0;
        tmpbufsize = 0;
        sf = 0;

        /* although libsndfile says we don't need to set this,
        valgrind and source code shows us that we do.
        Really? Look it up !
        */
        memset (&sfinfo, 0, sizeof(sfinfo));


        if ((sf = sf_open (m_filename.toAscii().data(), SFM_READ, &sfinfo)) == 0) {
                PERROR("Couldn't open soundfile (%s)", m_filename.toAscii().data());
                return -1;
        }

        if (channelNumber >= (uint)sfinfo.channels) {
                PERROR("ReadAudioSource: file only contains %d channels; %d is invalid as a channel number", sfinfo.channels, channelNumber);
                sf_close (sf);
                sf = 0;
                return -1;
        }

        if (sfinfo.channels == 0) {
                PERROR("ReadAudioSource: not a valid channel count: %d", sfinfo.channels);
                sf_close (sf);
                sf = 0;
                return -1;
        }

        m_length = sfinfo.frames;
        PWARN("Length is %d", m_length);
        return 1;
}


int ReadSource::file_read (audio_sample_t* dst, nframes_t start, nframes_t cnt) const
{
        if (start >= m_length) {
                return 0;
        }


        if (sf_seek (sf, (off_t) start, SEEK_SET) < 0) {
                char errbuf[256];
                sf_error_str (0, errbuf, sizeof (errbuf) - 1);
                PERROR("ReadAudioSource: could not seek to frame %d within %s (%s)", start, m_filename.toAscii().data(), errbuf);
                return 0;
        }

        if (sfinfo.channels == 1) {
                nframes_t ret = sf_read_float (sf, dst, cnt);
                m_read_data_count = cnt * sizeof(float);
                return ret;
        }

        int32_t nread;
        float *ptr;
        uint32_t real_cnt = cnt * sfinfo.channels;

        /*	{
        		Do we want to have a Lock here during read?? */

        if (tmpbufsize < real_cnt) {

                if (tmpbuf) {
                        delete [] tmpbuf;
                }
                tmpbufsize = real_cnt;
                tmpbuf = new float[tmpbufsize];
        }

        nread = sf_read_float (sf, tmpbuf, real_cnt);
        ptr = tmpbuf + channelNumber;
        nread /= sfinfo.channels;

        /* stride through the interleaved data */

        for (int32_t n = 0; n < nread; ++n) {
                dst[n] = *ptr;
                ptr += sfinfo.channels;
        }

        // 	}

        // 	m_read_data_count = cnt * sizeof(float);

        return nread;
}


int ReadSource::rb_read(audio_sample_t* dst, nframes_t start, nframes_t cnt)
{

        if ( ! rbReady ) {
                // 		PWARN("RingBuffer is not ready!");
                return 0;
        }


        if (start != rbRelativeFileReadPos) {
                if ( (start > rbRelativeFileReadPos) && (rbRelativeFileReadPos + m_buffer->read_space()) > start) {
                        int advance = start - rbRelativeFileReadPos;
                        int readspace = m_buffer->read_space();
                        PWARN("advance is %d, readspace is %d", advance, readspace);
                        m_buffer->read_advance( advance );
                        rbRelativeFileReadPos += advance;
                } else {
                        start_resync(start);
                        return 0;
                }
        }

        nframes_t readFrames = m_buffer->read(dst, cnt);

        if (readFrames != cnt) {
                // Hmm, not sure what to do in this case....
        }

        rbRelativeFileReadPos += readFrames;

        return readFrames;
}


int ReadSource::rb_file_read( audio_sample_t * dst, nframes_t cnt )
{
        int readFrames = file_read( dst, rbFileReadPos, cnt);
        rbFileReadPos += readFrames;

        return readFrames;
}


void ReadSource::rb_seek_to_file_position( nframes_t position )
{
        int fileposition = position;
        if (fileposition > 0) {
                m_buffer->reset();
                rbFileReadPos = fileposition;
                rbRelativeFileReadPos = fileposition;
        }
}

void ReadSource::set_source_start_offset( nframes_t offset )
{
        sourceStartOffset = offset;
}

void ReadSource::start_resync( nframes_t position )
{
        syncPos = position;
        rbReady = false;
        needSync = true;
        PWARN("Resyncing ringbuffer start");
}

bool ReadSource::need_sync( )
{
        return needSync;
}

void ReadSource::sync( )
{
        rb_seek_to_file_position(syncPos);
        needSync = false;
        PWARN("Resyncing ringbuffer finished");
}

int ReadSource::process_ringbuffer( audio_sample_t * framebuffer )
{
	nframes_t writeSpace = m_buffer->write_space();

	if (writeSpace > 4096) {
		nframes_t toWrite = rb_file_read(framebuffer, writeSpace);

		m_buffer->write(framebuffer, toWrite);
	}
	
	return 0;
}

void ReadSource::set_rb_ready( bool ready )
{
	rbReady = ready;
}

void ReadSource::set_active( )
{
	PENTER;
	active = true;
}

void ReadSource::set_inactive( )
{
	PENTER;
	active = false;
}

//eof

