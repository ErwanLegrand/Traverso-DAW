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

$Id: Project.h,v 1.2 2006/04/25 16:50:29 r_sijrier Exp $
*/

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QHash>
#include <QList>

#include <libtraversocore.h>
#include "ContextItem.h"


class Song;
class AudioSourcesList;
struct ExportSpecification;

class Project : public ContextItem
{
	Q_OBJECT

public :
	Project(QString pProjectToLoadTitle);
	~Project();


	// Get functions
	int get_current_song_id() const
	{
		return currentSongId;
	}
	int get_num_songs() const
	{
		return songList.size();
	}
	int get_rate();
	int get_bitdepth();
	QString get_title() const
	{
		return title;
	}
	QString get_engineer() const
	{
		return engineer;
	}
	QString get_root_dir() const
	{
		return rootDir;
	}
	QStringList get_songs();
	QHash<int, Song* > get_song_list();

	Song* get_current_song();
	Song* get_song(int id);

	// Set functions
	void set_title(QString pTitle);
	void set_engineer(QString pEngineer);
	void set_song_export_progress(int pogress);

	Song* add_song();
	AudioSource* new_audio_source(int songId, QString name, uint channel);
	AudioSource* get_source(qint64 sourceId);
	AudioSource* get_source(QString fileName, int channel);
	bool has_changed();
	int create(int pNumSongs);
	int save();
	int load();
	int remove_song(int id);
	int rename();
	int export_project(ExportSpecification* spec);
	int start_export(ExportSpecification* spec);
	int add_audio_source(AudioSource* source, int channel);




public slots:
	Command* select();

private:
	QHash<int, Song* >	songList;
	AudioSourcesList* 	audioSourcesList;

	QString 		title;
	QString 		rootDir;
	QString 		sourcesDir;
	QString 		engineer;

	int			m_rate;
	int			m_bitDepth;

	int			overallExportProgress;
	int 			renderedSongs;
	QList<Song* > songsToRender;

	int currentSongId;
	
	void set_current_song(int id);

	friend class Song;

signals:
	void currentSongChanged(Song* );
	void newSongCreated(Song* );
	void songAdded(Song* );
	void songRemoved(Song* );
	void songExportProgressChanged(int );
	void overallExportProgressChanged(int );
	void exportFinished();
	void exportStartedForSong(Song* );
};

#endif
