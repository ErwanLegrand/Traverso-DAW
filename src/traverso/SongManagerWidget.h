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
 
    $Id: SongManagerWidget.h,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#ifndef SONGMANAGERWIDGET_H
#define SONGMANAGERWIDGET_H

#include "ui_SongManagerWidget.h"
#include <QWidget>

class SongManagerWidget : public QWidget, protected Ui::SongManagerWidget
{
        Q_OBJECT

public:
        SongManagerWidget(QWidget* parent = 0);
        ~SongManagerWidget();

public slots:
        void update_song_list();

private slots:
        void songitem_clicked( QTreeWidgetItem* item, int);
        void on_saveSongButton_clicked();
        void on_deleteSongButton_clicked();
        void on_createSongButton_clicked();
        void on_importFileButton_clicked();
        void on_chooseFileButton_clicked();
};

#endif

//eof


