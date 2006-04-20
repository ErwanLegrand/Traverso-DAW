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
 
    $Id: ManagerWidget.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include "ManagerWidget.h"
#include "ui_ManagerWidget.h"

#include "libtraversocore.h"

#include "ProjectManagerWidget.h"
#include "SongManagerWidget.h"
#include "AudioSourcesManagerWidget.h"
#include "GlobalPropertiesWidget.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ManagerWidget::ManagerWidget( QWidget * parent )
                : QWidget(parent)
{
        setupUi(this);

        pmw = new ProjectManagerWidget(stackedManagerWidget);
        smw = new SongManagerWidget(stackedManagerWidget);
        asmw = new AudioSourcesManagerWidget(stackedManagerWidget);
        gpw = new GlobalPropertiesWidget(stackedManagerWidget);

        stackedManagerWidget->addWidget(pmw);
        stackedManagerWidget->addWidget(smw);
        stackedManagerWidget->addWidget(asmw);
        stackedManagerWidget->addWidget(gpw);

        projectButton->setIcon(QIcon(":/projectmanagement"));
        songButton->setIcon(QIcon(":/songmanagement"));
        audioSourcesButton->setIcon(QIcon(":/audiosourcesmanagement"));
        globalPropertiesButton->setIcon(QIcon(":/globalproperties"));
        stackedManagerWidget->setCurrentWidget(pmw);
}

ManagerWidget::~ ManagerWidget( )
{}


void ManagerWidget::on_projectButton_clicked( )
{
        pmw->update_projects_list();
        stackedManagerWidget->setCurrentWidget(pmw);
}

void ManagerWidget::on_songButton_clicked( )
{
        smw->update_song_list();
        stackedManagerWidget->setCurrentWidget(smw);
}

void ManagerWidget::on_audioSourcesButton_clicked()
{
        asmw->update_audio_sources_list();
        stackedManagerWidget->setCurrentWidget(asmw);
}

void ManagerWidget::on_globalPropertiesButton_clicked( )
{
        stackedManagerWidget->setCurrentWidget(gpw);
}



//eof
