/*
Copyright (C) 2005-2007 Remon Sijrier 

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

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QList>
#include <QDomNode>
#include "ContextItem.h"


class Song;
class ResourcesManager;
struct ExportSpecification;

class Project : public ContextItem
{
	Q_OBJECT

public :
	~Project();


	// Get functions
	int get_current_song_id() const;
	int get_num_songs() const;
	int get_rate() const;
	int get_bitdepth() const;
	
	ResourcesManager* get_audiosource_manager() const;
	QString get_title() const;
	QString get_engineer() const;
	QString get_description() const;
	QString get_discid() const;
	QString get_performer() const;
	QString get_arranger() const;
	QString get_songwriter() const;
	QString get_message() const;
	QString get_upc_ean() const;
	int get_genre();
	QString get_root_dir() const;
	QString get_audiosources_dir() const;
	QString get_import_dir() const;
	QList<Song* > get_songs() const;
	Song* get_current_song() const ;
	Song* get_song(qint64 id) const;
	int get_song_index(qint64 id) const;
	QDomNode get_state(QDomDocument doc, bool istemplate=false);


	// Set functions
	void set_title(const QString& pTitle);
	void set_engineer(const QString& pEngineer);
	void set_description(const QString& des);
	void set_discid(const QString& pId);
	void set_performer(const QString& pPerformer);
	void set_arranger(const QString& pArranger);
	void set_songwriter(const QString& pSongwriter);
	void set_message(const QString& pMessage);
	void set_upc_ean(const QString& pUPC);
	void set_genre(int pGenre);
	void set_song_export_progress(int pogress);
	void set_current_song(qint64 id);
	void set_import_dir(const QString& dir);

	
	Command* add_song(Song* song, bool historable=true);
	Command* remove_song(Song* song, bool historable=true);
	
	bool has_changed();
	bool is_save_to_close() const;
	bool is_recording() const;
	
	int save();
	int load(QString projectfile = "");
	int export_project(ExportSpecification* spec);
	int start_export(ExportSpecification* spec);


public slots:
	Command* select();

private:
	Project(const QString& title);
	
	QList<Song* >	m_songs;
	ResourcesManager* 	m_asmanager;

	QString 	title;
	QString 	rootDir;
	QString 	sourcesDir;
	QString 	engineer;
	QString		m_description;
	QString		m_importDir;
	QString		m_discid;
	int		m_genre;
	QString		m_upcEan;
	QString		m_performer;
	QString		m_arranger;
	QString		m_songwriter;
	QString		m_message;

	int		m_rate;
	int		m_bitDepth;

	int		overallExportProgress;
	int 		renderedSongs;
	QList<Song* > 	songsToRender;

	qint64 		m_currentSongId;
	
	int create(int songcount, int numtracks);
	QString		get_cdrdao_header(ExportSpecification* spec);
	
	friend class ProjectManager;

private slots:
	void private_add_song(Song* song);
	void private_remove_song(Song* song);

signals:
	void currentSongChanged(Song* );
	void songAdded(Song*);
	void songRemoved(Song*);
	void songExportProgressChanged(int );
	void overallExportProgressChanged(int );
	void exportFinished();
	void exportStartedForSong(Song* );
};

#endif
