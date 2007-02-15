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

$Id: Project.h,v 1.7 2007/02/15 21:07:57 r_sijrier Exp $
*/

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QHash>
#include <QList>

#include "ContextItem.h"


class Song;
class AudioSourceManager;
struct ExportSpecification;

class Project : public ContextItem
{
	Q_OBJECT

public :
	Project(const QString& pProjectToLoadTitle);
	~Project();


	// Get functions
	int get_current_song_id() const;
	int get_num_songs() const;
	int get_rate() const;
	int get_bitdepth() const;
	
	AudioSourceManager* get_audiosource_manager() const;
	QString get_title() const;
	QString get_engineer() const;
	QString get_root_dir() const;
	QString get_audiosources_dir() const;
	QStringList get_songs() const;
	QHash<int, Song* > get_song_list() const;
	Song* get_current_song() const ;
	Song* get_song(int id) const;


	// Set functions
	void set_title(const QString& pTitle);
	void set_engineer(const QString& pEngineer);
	void set_song_export_progress(int pogress);

	
	Song* add_song();
	
	bool has_changed();
	
	int create(int pNumSongs);
	int save();
	int load();
	int remove_song(int id);
	int rename();
	int export_project(ExportSpecification* spec);
	int start_export(ExportSpecification* spec);


public slots:
	Command* select();

private:
	QHash<int, Song* >	songList;
	AudioSourceManager* 	asmanager;

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
	void songAdded();
	void songRemoved();
	void songExportProgressChanged(int );
	void overallExportProgressChanged(int );
	void exportFinished();
	void exportStartedForSong(Song* );
};

#endif
