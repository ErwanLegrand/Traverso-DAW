/*
Copyright (C) 2007 Remon Sijrier 

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

#include "ResourcesWidget.h"

#include <ProjectManager.h>
#include <Project.h>
#include <ResourcesManager.h>
#include <AudioSource.h>
#include <ReadSource.h>
#include <AudioClip.h>
#include <Utils.h>


ResourcesWidget::ResourcesWidget(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
/*	audioFileTreeWidget->hide();
	clipTreeWidget->show();*/
	
	set_project(pm().get_project());
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
}

ResourcesWidget::~ ResourcesWidget()
{
}

void ResourcesWidget::set_project(Project * project)
{
/*	audioFileTreeWidget->clear();
	clipTreeWidget->clear();*/
	
	m_project = project;
	
	if (!m_project) {
		return;
	}
	
	connect(m_project->get_audiosource_manager(), SIGNAL(sourceAdded()), this, SLOT(update_tree_widgets()));
	update_tree_widgets();
}

void ResourcesWidget::update_tree_widgets()
{
	ResourcesManager* manager = resources_manager();
	if (! manager) {
		return;
	}
	
	foreach(ReadSource* rs, manager->get_all_audio_sources()) {
		QTreeWidgetItem* item = new QTreeWidgetItem(audioFileTreeWidget);
		item->setText(0, rs->get_name());
		item->setData(0, Qt::UserRole, rs->get_id());
	}
	
	
	foreach(AudioClip* clip, resources_manager()->get_all_clips()) {
		QTreeWidgetItem* item = new QTreeWidgetItem(clipTreeWidget);
		item->setText(0, clip->get_name());
		item->setData(0, Qt::UserRole, clip->get_id());
	}

	clipTreeWidget->sortItems(0, Qt::AscendingOrder);
	audioFileTreeWidget->sortItems(0, Qt::AscendingOrder);
}

