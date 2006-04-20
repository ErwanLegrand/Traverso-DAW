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
 
    $Id: ProjectInfoWidget.h,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#ifndef PROJECTINFOWIDGET_H
#define PROJECTINFOWIDGET_H

#include "ui_ProjectInfoWidget.h"
#include <QWidget>

class Song;
class Project;

class ProjectInfoWidget : public QWidget, protected Ui::ProjectInfoWidget
{
        Q_OBJECT

public:
        ProjectInfoWidget(QWidget* parent = 0);
        ~ProjectInfoWidget();

private:
        Project*		m_project;

public slots:
        void set_project(Project* );
        void update_song_count(Song* );

};

#endif

//eof



