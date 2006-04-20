/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: Peak.h,v 1.1 2006/04/20 14:51:40 r_sijrier Exp $
*/

#ifndef PEAK_H
#define PEAK_H

#include <QObject>
#include <QThread>
#include <QHash>

#include "defines.h"

class AudioSource;
class Peak;

class PeakBuffer
{
public:
        PeakBuffer();
        ~PeakBuffer();

        nframes_t get_size() const
        {
                return bufferSize;
        }
        unsigned char* get_upper_half_buffer() const
        {
                return upperHalfBuffer;
        }
        unsigned char* get_lower_half_buffer() const
        {
                return lowerHalfBuffer;
        }
        short* get_microview_buffer() const
        {
                return microViewBuffer;
        }
        void set_size(nframes_t size);
        void set_microview_buffer_size(nframes_t size);
        void set_recording_size(nframes_t size);
        void set_upper_value(unsigned char value, nframes_t position)
        {
                upperHalfBuffer[position] = value;
        }
        void set_lower_value(unsigned char value, nframes_t position)
        {
                lowerHalfBuffer[position] = value;
        }
        void set_microview_buffer_value(short value, nframes_t position)
        {
                microViewBuffer[position] = value;
        }



private:

        nframes_t		bufferSize;
        nframes_t		microBufferSize;
        unsigned char*		upperHalfBuffer;
        unsigned char*		lowerHalfBuffer;
        short*			microViewBuffer;
};


class PeakBuildThread : public QThread
{
public:
        PeakBuildThread(Peak* peak);
        Peak* m_peak;
        void run();
};

class Peak : public QObject
{
        Q_OBJECT

public:
        static const int ZOOM_LEVELS = 18;
        static const int MAX_ZOOM_USING_SOURCEFILE;
        static int zoomStep[ZOOM_LEVELS];

        Peak(AudioSource* source);
        ~Peak();

        void process(audio_sample_t* buffer, nframes_t frames);
        void prepare_process_buffer();
        void finish_process_buffer();
        int prepare_buffer_for_zoomlevel(int zoomLevel, nframes_t startPos, nframes_t nframes);

        void free_buffer_memory();

        void set_audiosource(AudioSource* source);

        PeakBuffer* get_prepared_peakbuffer() const
        {
                return preparedBufferPointer;
        }


private:
        QHash<int, PeakBuffer*>		bufferList;
        AudioSource* 			m_source;
        PeakBuffer*			preparedBuffer;
        PeakBuffer*			preparedBufferPointer;
        PeakBuildThread*		peakBuildThread;
        bool 				peaksAvailable;
        PeakBuffer*			processBuffer;
        nframes_t			processedFrames;
        nframes_t			processBufferPos;
        int 				m_progress;
        audio_sample_t		peakUpperValue;
        audio_sample_t		peakLowerValue;

        int load_peaks();
        int load();
        int create_from_scratch();
        int save();
        int create_buffers();

        friend class PeakBuildThread;

signals:
        void finished();
        void progress(int m_progress);
};


#endif

//eof
