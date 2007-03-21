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
 
*/

#ifndef SONG_MANAGER_DIALOG_H
#define SONG_MANAGER_DIALOG_H

#include "ui_SongManagerDialog.h"
#include <QDialog>

class Project;
class Song;

class SongManagerDialog : public QDialog, protected Ui::SongManagerDialog
{
        Q_OBJECT

public:
        SongManagerDialog(QWidget* parent = 0);
        ~SongManagerDialog();

protected:
	void hideEvent ( QHideEvent * event );
	void showEvent ( QShowEvent * event );

public slots:
        void update_song_list();

private:
	Project* m_project;

private slots:
	void set_project(Project* project);
	void songitem_clicked( QTreeWidgetItem* item, int);
	void on_renameSongButton_clicked();
        void on_deleteSongButton_clicked();
        void on_createSongButton_clicked();
	void song_removed(Song* song);
	void song_added(Song* song);
};

#endif

//eof


