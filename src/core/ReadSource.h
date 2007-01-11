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

$Id: ReadSource.h,v 1.13 2007/01/11 14:57:37 r_sijrier Exp $
*/

#ifndef READSOURCE_H
#define READSOURCE_H

#include "AudioSource.h"

class AudioClip;
class Peak;
class PrivateReadSource;

class ReadSource : public AudioSource
{
public :
	ReadSource(const QDomNode node);
	ReadSource(const QString& dir, const QString& name);
	ReadSource(const QString& dir, const QString& name, int channelCount, int fileCount);
	~ReadSource();
	
	ReadSource* deep_copy();

	int rb_read(int channel, audio_sample_t* dst, nframes_t start, nframes_t cnt);
	int file_read(int channel, audio_sample_t* dst, nframes_t start, nframes_t cnt) const;

	int init();
	int ref();
	void set_active(bool active);
	
	void set_audio_clip(AudioClip* clip);
	Peak* get_peak(int channel);
	nframes_t get_nframes() const;
	
	QList<PrivateReadSource*> get_private_sources() const {return m_sources;}

private:
	QList<PrivateReadSource*> m_sources;
	int		refcount;
	
	int add_private_source(int channel, int channelNumber, const QString& fileName);
	
	friend class PrivateReadSource;
};

#endif
