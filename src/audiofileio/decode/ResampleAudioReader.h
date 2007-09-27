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

#ifndef RESAMPLEAUDIOREADER_H
#define RESAMPLEAUDIOREADER_H

#include <AbstractAudioReader.h>
#include <QVector>
#include <samplerate.h>


class ResampleAudioReader : public AbstractAudioReader
{

public:
	ResampleAudioReader(QString filename, const QString& decoder);
	~ResampleAudioReader();
	
	nframes_t read_from(DecodeBuffer* buffer, TimeRef start, nframes_t count) {
		return AbstractAudioReader::read_from(buffer, start.to_frame(m_outputRate), count);
	}
	QString decoder_type() const {return (m_reader) ? m_reader->decoder_type() : "";}
	void clear_buffers();
	
	int get_output_rate();
	int get_file_rate();
	void set_output_rate(int rate);
	void set_converter_type(int converter_type);
	
protected:
	void reset();
	
	bool seek_private(nframes_t start);
	nframes_t read_private(DecodeBuffer* buffer, nframes_t frameCount);
	
	nframes_t resampled_to_file_frame(nframes_t frame);
	nframes_t file_to_resampled_frame(nframes_t frame);
	
	AbstractAudioReader*	m_reader;
	QVector<SRC_STATE*>	m_srcStates;
	SRC_DATA		m_srcData;
	audio_sample_t**	m_overflowBuffers;
	long			m_overflowUsed;
	int			m_outputRate;
	bool			m_isResampleAvailable;
	nframes_t		m_readExtraFrames;
	
private:
	void create_overflow_buffers();
};

#endif
