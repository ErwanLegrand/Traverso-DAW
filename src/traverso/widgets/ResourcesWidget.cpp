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
		palette.setColor(QPalette::AlternateBase, themer()->get_color("ResourcesBin:alternaterowcolor"));
		
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
		m_box->addItem("", "");
#if defined (Q_WS_WIN)
		m_box->addItem(tr("My Computer"), "");
		m_box->addItem(tr("My Documents"), QDir::homePath() + "\\" + tr("My Documents"));
#else
		m_box->addItem(QDir::rootPath(), QDir::rootPath());
		m_box->addItem(QDir::homePath(), QDir::homePath());
#endif
		QPushButton* button = new QPushButton(this);
		QIcon icon = QApplication::style()->standardIcon(QStyle::SP_FileDialogToParent);
		button->setIcon(icon);
		button->setMaximumHeight(23);
		
		QHBoxLayout* hlay = new QHBoxLayout;
		hlay->addWidget(button);
		hlay->addWidget(m_box, 10);
		
		QVBoxLayout* lay = new QVBoxLayout(this);
		lay->setMargin(0);
		lay->setSpacing(6);
		lay->addLayout(hlay);
		lay->addWidget(m_dirView);
		
		setLayout(lay);
		
		connect(m_dirView, SIGNAL(clicked(const QModelIndex& )), this, SLOT(dirview_item_clicked(const QModelIndex&)));
		connect(button, SIGNAL(clicked()), this, SLOT(dir_up_button_clicked()));
		connect(m_box, SIGNAL(activated(int)), this, SLOT(box_actived(int)));
		
	}
	
	void set_current_path(const QString& path) const;
	
private slots:
	void dirview_item_clicked(const QModelIndex & index);
	void dir_up_button_clicked();
	void box_actived(int i);
	
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
		m_box->setItemData(0, m_dirModel->filePath(index));
		m_box->setCurrentIndex(0);
	}
}

void FileWidget::dir_up_button_clicked()
{
	QDir dir(m_dirModel->filePath(m_dirView->rootIndex()));
	
#if defined (Q_WS_WIN)
	if (m_dirModel->filePath(m_dirView->rootIndex()) == "") {
		return;
	}
	QString oldDir = dir.canonicalPath();
#endif
	
	dir.cdUp();
	QString text = dir.canonicalPath();
	
#if defined (Q_WS_WIN)
	if (oldDir == dir.canonicalPath()) {
		dir.setPath("");
		text = tr("My Computer");
	}
#endif
	
	m_dirView->setRootIndex(m_dirModel->index(dir.canonicalPath()));
	m_box->setItemText(0, text);
	m_box->setItemData(0, dir.canonicalPath());
	m_box->setCurrentIndex(0);
}

void FileWidget::box_actived(int i)
{
	m_dirView->setRootIndex(m_dirModel->index(m_box->itemData(i).toString()));
}

void FileWidget::set_current_path(const QString& path) const
{
	m_dirView->setRootIndex(m_dirModel->index(path));
	m_box->setItemText(0, path);
	m_box->setItemData(0, path);
}


ResourcesWidget::ResourcesWidget(QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);
	
	QPalette palette;
	palette.setColor(QPalette::AlternateBase, themer()->get_color("ResourcesBin:alternaterowcolor"));
	sourcesTreeWidget->setPalette(palette);
	sourcesTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	sourcesTreeWidget->setAlternatingRowColors(true);
	sourcesTreeWidget->setDragEnabled(true);
	sourcesTreeWidget->setDropIndicatorShown(true);
	sourcesTreeWidget->setIndentation(18);
	sourcesTreeWidget->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	sourcesTreeWidget->header()->setResizeMode(1, QHeaderView::ResizeToContents);
	sourcesTreeWidget->header()->setResizeMode(2, QHeaderView::ResizeToContents);
	sourcesTreeWidget->header()->setResizeMode(3, QHeaderView::ResizeToContents);
	sourcesTreeWidget->header()->setStretchLastSection(false);
	
	
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
	sourcesTreeWidget->clear();
	m_sourceindices.clear();
	m_clipindices.clear();
	songComboBox->clear();
	
	m_project = project;
	
	if (!m_project) {
		songComboBox->setEnabled(false);
		return;
	}
	
	songComboBox->setEnabled(true);
	ResourcesManager* rsmanager = m_project->get_audiosource_manager();
	
	connect(rsmanager, SIGNAL(stateRestored()), this, SLOT(populate_tree()));
	connect(rsmanager, SIGNAL(clipAdded(AudioClip*)), this, SLOT(add_clip(AudioClip*)));
	connect(rsmanager, SIGNAL(clipRemoved(AudioClip*)), this, SLOT(remove_clip(AudioClip*)));
	connect(rsmanager, SIGNAL(sourceAdded(ReadSource*)), this, SLOT(add_source(ReadSource*)));
	connect(rsmanager, SIGNAL(sourceRemoved(ReadSource*)), this, SLOT(remove_source(ReadSource*)));
	connect(m_project, SIGNAL(songAdded(Song*)), this, SLOT(song_added(Song*)));
	connect(m_project, SIGNAL(songRemoved(Song*)), this, SLOT(song_removed(Song*)));
}

void ResourcesWidget::populate_tree()
{
	if (!m_project) {
		return;
	}
	
	foreach(ReadSource* rs, resources_manager()->get_all_audio_sources()) {
		add_source(rs);
	}
	
	foreach(AudioClip* clip, resources_manager()->get_all_clips()) {
		add_clip(clip);
	}

	sourcesTreeWidget->sortItems(0, Qt::AscendingOrder);
}

void ResourcesWidget::view_combo_box_index_changed(int index)
{
	if (index == 0) {
		sourcesTreeWidget->show();
		m_filewidget->hide();
	} else if (index == 1) {
		sourcesTreeWidget->hide();
		m_filewidget->show();
		m_filewidget->set_current_path(m_project->get_import_dir());
	}
}

void ResourcesWidget::song_combo_box_index_changed(int index)
{
}

void ResourcesWidget::song_added(Song * song)
{
	songComboBox->addItem("Song " + QString::number(m_project->get_song_index(song->get_id())), song->get_id());
}

void ResourcesWidget::song_removed(Song * song)
{
	int index = songComboBox->findData(song->get_id());
	songComboBox->removeItem(index);
}

void ResourcesWidget::add_clip(AudioClip * clip)
{
	QTreeWidgetItem* item = m_clipindices.value(clip->get_id());
	
	if (!item) {
		QTreeWidgetItem* sourceitem = m_sourceindices.value(clip->get_readsource_id());
	
		if (! sourceitem ) return;
	
		ClipTreeItem* clipitem = new ClipTreeItem(sourceitem, clip);
		m_clipindices.insert(clip->get_id(), clipitem);
		
		connect(clip, SIGNAL(positionChanged(Snappable*)), clipitem, SLOT(clip_state_changed()));
	}
	
	update_clip_state(clip);
}

void ResourcesWidget::remove_clip(AudioClip * clip)
{
	QTreeWidgetItem* item = m_clipindices.value(clip->get_id());
	
	if (!item) {
		 return;
	}
	
	update_clip_state(clip);
}

void ResourcesWidget::add_source(ReadSource * source)
{
	QTreeWidgetItem* item = m_sourceindices.value(source->get_id());
	
	if (! item) {
		QTreeWidgetItem* item = new QTreeWidgetItem(sourcesTreeWidget);
		m_sourceindices.insert(source->get_id(), item);
		QString duration = frame_to_ms(source->get_nframes(), 44100);
		item->setText(0, source->get_short_name());
		item->setText(1, duration);
		item->setText(2, "");
		item->setText(3, "");
		item->setData(0, Qt::UserRole, source->get_id());
		item->setToolTip(0, source->get_short_name() + "   " + duration);
	}
	
	update_source_state(source->get_id());
}

void ResourcesWidget::remove_source(ReadSource * source)
{
}

void ResourcesWidget::update_clip_state(AudioClip* clip)
{
	ClipTreeItem* item = m_clipindices.value(clip->get_id());
	Q_ASSERT(item);
	
	item->clip_state_changed();
	
	update_source_state(clip->get_readsource_id());
}

void ResourcesWidget::update_source_state(qint64 id)
{
	QTreeWidgetItem* item = m_sourceindices.value(id);
	Q_ASSERT(item);
	
	if (resources_manager()->is_source_in_use(id)) {
		for (int i=0; i<5; ++i) {
			item->setForeground(i, QColor(Qt::black));
		}
	} else {
		for (int i=0; i<5; ++i) {
			item->setForeground(i, QColor(Qt::lightGray));
		}
	}
}

ClipTreeItem::ClipTreeItem(QTreeWidgetItem * parent, AudioClip * clip)
	: QTreeWidgetItem(parent)
	, m_clip(clip)
{
	setData(0, Qt::UserRole, clip->get_id());
}

void ClipTreeItem::clip_state_changed()
{
	if (resources_manager()->is_clip_in_use(m_clip->get_id())) {
		for (int i=0; i<5; ++i) {
			setForeground(i, QColor(Qt::black));
		}
	} else {
		for (int i=0; i<5; ++i) {
			setForeground(i, QColor(Qt::lightGray));
		}
	}
	
	QString start = frame_to_ms(m_clip->get_source_start_frame(), m_clip->get_rate());
	QString end = frame_to_ms(m_clip->get_source_end_frame(), m_clip->get_rate());
		
	setText(0, m_clip->get_name());
	setText(1, frame_to_ms(m_clip->get_length(), m_clip->get_rate()));
	setText(2, start);
	setText(3, end);
	setToolTip(0, m_clip->get_name() + "   " + start + " - " + end);
}

