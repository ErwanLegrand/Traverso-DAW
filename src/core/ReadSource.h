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

#ifndef READSOURCE_H
#define READSOURCE_H

#include "AudioSource.h"

class AudioClip;
class Peak;
class MonoReader;

class ReadSource : public AudioSource
{
	Q_OBJECT
public :
	ReadSource(const QDomNode node);
	ReadSource(const QString& dir, const QString& name);
	ReadSource(const QString& dir, const QString& name, int channelCount);
	ReadSource();  // For creating a 0-channel, silent ReadSource
	~ReadSource();
	
	enum ReadSourceError {
		COULD_NOT_OPEN_FILE = -1,
  		INVALID_CHANNEL_COUNT = -2,
    		ZERO_CHANNELS = -3,
      		CHANNELCOUNT_FILECOUNT_MISMATCH = -4
	};
	
	ReadSource* deep_copy();

	int rb_read(int channel, audio_sample_t* dst, nframes_t start, nframes_t cnt);
	int file_read(int channel, audio_sample_t* dst, nframes_t start, nframes_t cnt, audio_sample_t* readbuffer) const;

	int init();
	int get_error() const {return m_error;}
	int set_file(const QString& filename);
	void set_active(bool active);
	
	void set_audio_clip(AudioClip* clip);
	Peak* get_peak(int channel);
	nframes_t get_nframes() const;
	
	QList<MonoReader*> get_mono_readers() const {return m_sources;}

private:
	QList<MonoReader*> m_sources;
	int	m_refcount;
	int	m_error;
	AudioClip* m_clip;
	bool	m_silent;
	
	int ref() { return m_refcount++;}
	int add_mono_reader(int channel, int channelNumber, const QString& fileName);
	
	friend class MonoReader;
	friend class ResourcesManager;
};

#endif
