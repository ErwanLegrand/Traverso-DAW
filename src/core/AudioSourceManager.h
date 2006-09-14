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

$Id: AudioSourceManager.h,v 1.6 2006/09/14 10:49:39 r_sijrier Exp $
*/

#ifndef AUDIOSOURCEMANAGER_H
#define AUDIOSOURCEMANAGER_H

#include <QString>
#include <QHash>
#include <QDomDocument>
#include <QObject>


class AudioSource;
class ReadSource;
class AudioClip;

class AudioSourceManager : public QObject
{
	Q_OBJECT
	
public:
	AudioSourceManager();
	~AudioSourceManager();

	ReadSource* new_readsource(const QString& dir, const QString& name, int channelCount, int fileCount, int songId, int bitDepth, int rate=0 );
	ReadSource* new_readsource(const QString& dir, const QString& name);
	AudioClip* new_audio_clip(const QString& name);
	
	int remove (AudioSource* source);
	int set_state( const QDomNode& node );

	ReadSource* get_readsource(const QString& fileName);
	
	ReadSource* get_readsource(qint64 id);
	AudioClip* get_clip(qint64 id);
	
	QDomNode get_state(QDomDocument doc);
	
	int get_total_sources();


private:
	QHash<qint64, ReadSource* >	m_sources;
	QHash<qint64, AudioClip* >	m_clips;
	
	
signals:
	void sourceAdded();
	void sourceRemoved();
};



#endif

