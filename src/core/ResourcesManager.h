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

$Id: ResourcesManager.h,v 1.2 2007/03/29 11:09:38 r_sijrier Exp $
*/

#ifndef RESOURCES_MANAGER_H
#define RESOURCES_MANAGER_H

#include <QString>
#include <QHash>
#include <QList>
#include <QDomDocument>
#include <QObject>


class AudioSource;
class ReadSource;
class AudioClip;

class ResourcesManager : public QObject
{
	Q_OBJECT
	
public:
	ResourcesManager();
	~ResourcesManager();

	int set_state( const QDomNode& node );
	QDomNode get_state(QDomDocument doc);
	
	ReadSource* create_new_readsource(const QString& dir,
				const QString& name,
				int channelCount,
				int fileCount,
				int songId,
				int bitDepth,
				int rate=0,
    				bool wasRecording=false);
	
	ReadSource* create_new_readsource(const QString& dir, const QString& name);
	AudioClip* new_audio_clip(const QString& name);
	AudioClip* get_clip(qint64 id);
	
	int remove_clip_from_database(qint64 id);
	int undo_remove_clip_from_database(qint64 id);
	

	ReadSource* get_readsource(const QString& fileName);
	ReadSource* get_readsource(qint64 id);
	
	
	int get_total_sources();
	
	QList<ReadSource*> get_all_audio_sources() const;
	QList<AudioClip*> get_clips_for_source(ReadSource* source) const;


private:
	QHash<qint64, ReadSource* >	m_sources;
	QHash<qint64, AudioClip* >	m_clips;
	QHash<qint64, AudioClip* >	m_deprecatedClips;
	
	
signals:
	void sourceAdded();
	void sourceRemoved();
};



#endif

