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
	
	ReadSource* create_recording_source(const QString& dir,
				const QString& name,
				int channelCount,
				int songId);
	
	ReadSource* create_new_readsource(const QString& dir, const QString& name);
	ReadSource* get_silent_readsource();
	AudioClip* new_audio_clip(const QString& name);
	AudioClip* get_clip(qint64 id);
	
	int remove_clip_from_database(qint64 id);
	int undo_remove_clip_from_database(qint64 id);
	
	void set_clip_removed(AudioClip* clip);
	void set_clip_added(AudioClip* clip);
	void set_source_for_clip(AudioClip* clip, ReadSource* source);
	
	bool is_clip_in_use(qint64) const;

	ReadSource* get_readsource(const QString& fileName);
	ReadSource* get_readsource(qint64 id);
	
	
	int get_total_sources();
	
	QList<ReadSource*> get_all_audio_sources() const;
	QList<AudioClip*> get_all_clips() const;
	QList<AudioClip*> get_clips_for_source(ReadSource* source) const;


private:
	QHash<qint64, ReadSource* >	m_sources;
	QHash<qint64, AudioClip* >	m_clips;
	QHash<qint64, AudioClip* >	m_gettedClips;
	QHash<qint64, AudioClip* >	m_removedClips;
	QHash<qint64, AudioClip* >	m_deprecatedClips;
	ReadSource*			m_silentReadSource;
	
	
signals:
	void sourceAdded();
	void sourceRemoved();
	void stateChanged();
	void clipRemoved(AudioClip* clip);
	void clipAdded(AudioClip* clip);
	void sourceNoLongerUsed(ReadSource* source);
	void sourceBackInUse(ReadSource* source);
	void newSourceCreated(ReadSource* source);
};



#endif

