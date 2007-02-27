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

$Id: ManagerWidget.cpp,v 1.5 2007/02/27 19:49:22 r_sijrier Exp $
*/

#include "ManagerWidget.h"
#include "ui_ManagerWidget.h"

//#include "libtraversocore.h"

#include "ProjectManagerWidget.h"
#include "SongManagerWidget.h"
#include "AudioSourcesManagerWidget.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ManagerWidget::ManagerWidget( QWidget * parent )
		: QWidget(parent)
{
	setupUi(this);
	
	smw = 0;
	asmw = 0;

	m_scrollArea = new QScrollArea();
	pmw = new ProjectManagerWidget();

	projectButton->setIcon(QIcon(":/projectmanagement"));
	songButton->setIcon(QIcon(":/songmanagement"));
	audioSourcesButton->setIcon(QIcon(":/audiosourcesmanagement"));
	globalPropertiesButton->setIcon(QIcon(":/globalproperties"));
	
	layout()->addWidget(m_scrollArea);
	on_projectButton_clicked();
}

ManagerWidget::~ ManagerWidget( )
{}


void ManagerWidget::on_projectButton_clicked( )
{
	pmw->update_projects_list();
	m_scrollArea->takeWidget();
	m_scrollArea->setWidget(pmw);
}

void ManagerWidget::on_songButton_clicked( )
{
	if (!smw) {
		smw = new SongManagerWidget(m_stackedWidget);
	}
	
	smw->update_song_list();
	m_scrollArea->takeWidget();
	m_scrollArea->setWidget(smw);
}

void ManagerWidget::on_audioSourcesButton_clicked()
{
	if (!asmw) {
		asmw = new AudioSourcesManagerWidget(m_stackedWidget);
		m_stackedWidget->addWidget(asmw);
	}
	
	asmw->update_audio_sources_list();
	m_scrollArea->takeWidget();
	m_scrollArea->setWidget(asmw);
}


//eof
