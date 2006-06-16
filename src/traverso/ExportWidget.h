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
 
    $Id: ExportWidget.h,v 1.2 2006/06/16 14:09:26 r_sijrier Exp $
*/

#ifndef EXPORTWIDGET_H
#define EXPORTWIDGET_H

#include "ui_ExportWidget.h"

#include <QWidget>
#include <QDialog>

class Project;
class Song;
struct ExportSpecification;

class ExportWidget : public QDialog, protected Ui::ExportWidget
{
        Q_OBJECT

public:
        ExportWidget(QWidget* parent = 0);
        ~ExportWidget();

private:
        Project*		m_project;
        ExportSpecification* 	spec;

        void show_progress_view();
        void show_settings_view();

private slots:
        void update_song_progress(int progress);
        void update_overall_progress(int progress);
        void render_finished();
        void set_exporting_song(Song* song);

        void on_fileSelectButton_clicked();
        void on_exportStartButton_clicked();
        void on_exportStopButton_clicked();
        void on_selectionSongButton_clicked();
        void on_allSongsButton_clicked();
        void on_currentSongButton_clicked();

};

#endif

//eof


