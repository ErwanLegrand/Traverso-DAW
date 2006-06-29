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
 
    $Id: ProjectInfoWidget.cpp,v 1.3 2006/06/29 22:16:21 r_sijrier Exp $
*/

#include "ProjectInfoWidget.h"
#include "ui_ProjectInfoWidget.h"

#include "libtraversocore.h"
#include "ColorManager.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ProjectInfoWidget::ProjectInfoWidget( QWidget * parent )
                : QWidget(parent)
{
        setupUi(this);

        QPalette palette;
        palette.setColor(QPalette::Background, cm().get("INFO_WIDGET_BACKGROUND"));
        setPalette(palette);
        setAutoFillBackground(true);

        connect(&pm(), SIGNAL(projectLoaded(Project* )), this, SLOT(set_project(Project* )));
}

ProjectInfoWidget::~ ProjectInfoWidget( )
{}

void ProjectInfoWidget::set_project(Project* project)
{
	if (project) {
		m_project = project;
		projectNameLabel->setText(project->get_title());
		bitdepthLabel->setText( QByteArray::number(project->get_bitdepth()) );
		rateLabel->setText( QByteArray::number(project->get_rate()) );
		songCountLabel->setText(QString::number(project->get_num_songs()) );
		
		connect(m_project, SIGNAL(songAdded()), this, SLOT(update_song_count( )));
		connect(m_project, SIGNAL(songRemoved( )), this, SLOT(update_song_count( )));
	} else {
		projectNameLabel->setText("-");
		bitdepthLabel->setText("-");
		rateLabel->setText("-");
		songCountLabel->setText("-");
	}
}

void ProjectInfoWidget::update_song_count( )
{
        songCountLabel->setText(QString::number(m_project->get_num_songs()) );
}

//eof

