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
 
    $Id: SongInfoWidget.h,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#ifndef SONGINFOWIDGET_H
#define SONGINFOWIDGET_H

#include "ui_SongInfoWidget.h"
#include <QWidget>
#include <QTimer>


class Song;
class Project;
class QTimer;

class SongInfoWidget : public QWidget, protected Ui::SongInfoWidget
{
        Q_OBJECT

public:
        SongInfoWidget(QWidget* parent = 0);
        ~SongInfoWidget();

private:
        Song*		m_song;
        Project*		m_project;

        QTimer		smpteTimer;


public slots:
        void set_song(Song* );
        void update_smpte();
        void update_zoom();
        void update_snapstatus();
        void set_project(Project* );
        void start_smpte_update_timer();
        void stop_smpte_update_timer();
};

#endif

//eof




