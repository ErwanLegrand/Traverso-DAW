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
#include <Song.h>
#include <ResourcesManager.h>
#include <AudioSource.h>
#include <ReadSource.h>
#include <AudioClip.h>
#include <Utils.h>
#include <Themer.h>

#include <QHeaderView>


ResourcesWidget::ResourcesWidget(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
/*	audioFileTreeWidget->hide();
	clipTreeWidget->show();*/
	QPalette palette;
	palette.setColor(QPalette::AlternateBase, themer()->get_color("Track:background"));
	clipTreeWidget->setPalette(palette);
	clipTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	clipTreeWidget->setAlternatingRowColors(true);
	clipTreeWidget->setDragEnabled(true);
	clipTreeWidget->setDropIndicatorShown(true);
	clipTreeWidget->setIndentation(12);
	clipTreeWidget->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	clipTreeWidget->header()->setResizeMode(1, QHeaderView::ResizeToContents);
	clipTreeWidget->header()->setResizeMode(2, QHeaderView::ResizeToContents);
	clipTreeWidget->header()->setResizeMode(3, QHeaderView::ResizeToContents);
	clipTreeWidget->hide();
	
	audioFileTreeWidget->setPalette(palette);
	audioFileTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	audioFileTreeWidget->setAlternatingRowColors(true);
	audioFileTreeWidget->setDragEnabled(true);
	audioFileTreeWidget->setDropIndicatorShown(true);
	audioFileTreeWidget->setIndentation(12);
	audioFileTreeWidget->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	audioFileTreeWidget->header()->setResizeMode(1, QHeaderView::ResizeToContents);
	
	viewComboBox->setFocusPolicy(Qt::NoFocus);
	songComboBox->setFocusPolicy(Qt::NoFocus);
	clipTreeWidget->setFocusPolicy(Qt::NoFocus);
	audioFileTreeWidget->setFocusPolicy(Qt::NoFocus);

	
	connect(viewComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(view_combo_box_index_changed(int)));
	connect(songComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(song_combo_box_index_changed(int)));
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
}

ResourcesWidget::~ ResourcesWidget()
{
}

void ResourcesWidget::set_project(Project * project)
{
	audioFileTreeWidget->clear();
	clipTreeWidget->clear();
	songComboBox->clear();
	
	m_project = project;
	
	if (!m_project) {
		songComboBox->setEnabled(false);
		return;
	}
	
	songComboBox->setEnabled(true);
	
	connect(m_project->get_audiosource_manager(), SIGNAL(sourceAdded()), this, SLOT(update_tree_widgets()));
	connect(m_project->get_audiosource_manager(), SIGNAL(stateChanged()), this, SLOT(update_tree_widgets()));
	connect(m_project, SIGNAL(songAdded(Song*)), this, SLOT(song_added(Song*)));
	connect(m_project, SIGNAL(songRemoved(Song*)), this, SLOT(song_removed(Song*)));
	
	update_tree_widgets();
}

void ResourcesWidget::update_tree_widgets()
{
	audioFileTreeWidget->clear();
	clipTreeWidget->clear();
	
	foreach(ReadSource* rs, m_project->get_audiosource_manager()->get_all_audio_sources()) {
		QTreeWidgetItem* item = new QTreeWidgetItem(audioFileTreeWidget);
		QString duration = frame_to_ms(rs->get_nframes(), 44100);
		item->setText(0, rs->get_name());
		item->setText(1, duration);
		item->setData(0, Qt::UserRole, rs->get_id());
		item->setToolTip(0, rs->get_name() + "   " + duration);
		if (!rs->get_ref_count()) {
			item->setForeground(0, QColor(Qt::lightGray));
			item->setForeground(1, QColor(Qt::lightGray));
		}
	}
	
	
	foreach(AudioClip* clip, m_project->get_audiosource_manager()->get_all_clips()) {
		QTreeWidgetItem* item = new QTreeWidgetItem(clipTreeWidget);
		item->setText(0, clip->get_name());
		QString start = frame_to_ms(clip->get_source_start_frame(), clip->get_rate());
		QString end = frame_to_ms(clip->get_source_end_frame(), clip->get_rate());
		item->setText(1, start);
		item->setText(2, end);
		item->setText(3, frame_to_ms(clip->get_length(), clip->get_rate()));
		item->setData(0, Qt::UserRole, clip->get_id());
		item->setToolTip(0, clip->get_name() + "   " + start + " - " + end);
		
		if (!clip->get_ref_count()) {
			item->setForeground(0, QColor(Qt::lightGray));
			item->setForeground(1, QColor(Qt::lightGray));
			item->setForeground(2, QColor(Qt::lightGray));
			item->setForeground(3, QColor(Qt::lightGray));
		}
	}

	clipTreeWidget->sortItems(0, Qt::AscendingOrder);
	audioFileTreeWidget->sortItems(0, Qt::AscendingOrder);
}

void ResourcesWidget::view_combo_box_index_changed(int index)
{
	if (index == 0) {
		audioFileTreeWidget->show();
		clipTreeWidget->hide();
	} else if (index == 1) {
		audioFileTreeWidget->hide();
		clipTreeWidget->show();
	} else {
		audioFileTreeWidget->show();
		clipTreeWidget->show();
	}
		
}

void ResourcesWidget::song_combo_box_index_changed(int index)
{
	update_tree_widgets();
}

void ResourcesWidget::song_added(Song * song)
{
	songComboBox->addItem("Song " + QString::number(m_project->get_song_index(song->get_id())), song->get_id());
	update_tree_widgets();
}

void ResourcesWidget::song_removed(Song * song)
{
	int index = songComboBox->findData(song->get_id());
	songComboBox->removeItem(index);
	update_tree_widgets();
}

