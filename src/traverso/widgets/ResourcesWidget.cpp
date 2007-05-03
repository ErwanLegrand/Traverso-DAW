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
#include <QDirModel>
#include <QListView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>

class FileWidget : public QWidget
{
	Q_OBJECT
public:
	
	FileWidget(QWidget* parent=0)
	: QWidget(parent)
	{
		QPalette palette;
		palette.setColor(QPalette::AlternateBase, themer()->get_color("Track:background"));
		
		m_dirModel = new QDirModel;
		m_dirModel->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
		m_dirView = new QListView;
		m_dirView->setModel(m_dirModel);
		m_dirView->setDragEnabled(true);
		m_dirView->setDropIndicatorShown(true);
		m_dirView->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_dirView->setAlternatingRowColors(true);
		m_dirView->setPalette(palette);
		m_dirModel->setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
		
		m_box = new QComboBox(this);
		QFileInfoList list =  QDir::drives();
		foreach(QFileInfo info, list) {
			m_box->addItem(info.dir().canonicalPath());
		}
		m_box->addItem(QDir::homePath());
		m_box->addItem(QDir::rootPath());
		QPushButton* button = new QPushButton(this);
		QIcon icon = QApplication::style()->standardIcon(QStyle::SP_FileDialogToParent);
		button->setIcon(icon);
		
		QHBoxLayout* hlay = new QHBoxLayout;
		hlay->addWidget(m_box, 10);
		hlay->addWidget(button);
		hlay->addSpacing(2);
		
		QVBoxLayout* lay = new QVBoxLayout(this);
		lay->setMargin(0);
		lay->addLayout(hlay);
		lay->addWidget(m_dirView);
		
		setLayout(lay);
		
		connect(m_dirView, SIGNAL(clicked(const QModelIndex& )), this, SLOT(dirview_item_clicked(const QModelIndex&)));
		connect(button, SIGNAL(clicked()), this, SLOT(dir_up_button_clicked()));
		connect(m_box, SIGNAL(activated(const QString&)), this, SLOT(box_actived(const QString&)));
		
	}
	
	void set_current_path(const QString& path) const;
	
private slots:
	void dirview_item_clicked(const QModelIndex & index);
	void dir_up_button_clicked();
	void box_actived(const QString& path);
	
private:
	QListView* m_dirView;
	QDirModel* m_dirModel;
	QComboBox* m_box;
};

#include "ResourcesWidget.moc"
			 
void FileWidget::dirview_item_clicked(const QModelIndex & index)
{
	if (m_dirModel->isDir(index)) {
		m_dirView->setRootIndex(index);
		pm().get_project()->set_import_dir(m_dirModel->filePath(index));
		m_box->setItemText(0, m_dirModel->filePath(index));
	}
}

void FileWidget::dir_up_button_clicked()
{
	QDir dir(m_dirModel->filePath(m_dirView->rootIndex()));
	dir.cdUp();
	m_dirView->setRootIndex(m_dirModel->index(dir.canonicalPath()));
	m_box->setItemText(0, dir.canonicalPath());
}

void FileWidget::box_actived(const QString& path)
{
	m_dirView->setRootIndex(m_dirModel->index(path));
}

void FileWidget::set_current_path(const QString& path) const
{
	m_dirView->setRootIndex(m_dirModel->index(path));
	m_box->setItemText(0, path);
}


ResourcesWidget::ResourcesWidget(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
	
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
	
	m_filewidget = new FileWidget(this);
	layout()->addWidget(m_filewidget);
	m_filewidget->hide();
	
	
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
// 	connect(m_project->get_audiosource_manager(), SIGNAL(stateChanged()), this, SLOT(update_tree_widgets()));
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
		item->setText(0, rs->get_short_name());
		item->setText(1, duration);
		item->setData(0, Qt::UserRole, rs->get_id());
		item->setToolTip(0, rs->get_short_name() + "   " + duration);
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
		m_filewidget->hide();
	} else if (index == 1) {
		audioFileTreeWidget->hide();
		clipTreeWidget->show();
		m_filewidget->hide();
	} else if (index == 2) {
		audioFileTreeWidget->show();
		clipTreeWidget->show();
		m_filewidget->hide();
	} else {
		audioFileTreeWidget->hide();
		clipTreeWidget->hide();
		m_filewidget->show();
		m_filewidget->set_current_path(m_project->get_import_dir());
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

