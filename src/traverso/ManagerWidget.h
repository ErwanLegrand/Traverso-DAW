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
 
    $Id: ManagerWidget.h,v 1.3 2007/02/27 19:49:22 r_sijrier Exp $
*/

#ifndef MANAGERWIDGET_H
#define MANAGERWIDGET_H

#include "ui_ManagerWidget.h"
#include <QWidget>
#include <QScrollArea>
#include <QStackedWidget>

class ProjectManagerWidget;
class SongManagerWidget;
class AudioSourcesManagerWidget;


class ManagerWidget : public QWidget, private Ui::ManagerWidget
{
        Q_OBJECT

public:
        ManagerWidget(QWidget* parent = 0);
        ~ManagerWidget();

private:
        ProjectManagerWidget* pmw;
        SongManagerWidget* smw;
        AudioSourcesManagerWidget* asmw;
	QScrollArea*	m_scrollArea;
	QStackedWidget*	m_stackedWidget;

private slots:
        void on_projectButton_clicked();
        void on_songButton_clicked();
        void on_audioSourcesButton_clicked();


};

#endif

//eof


