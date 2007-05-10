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
	palette.setColor(QPalette::AlternateBase, themer()->get_color("Track:background"));
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
	songComboBox->clear();
	
	m_project = project;
	
	if (!m_project) {
		songComboBox->setEnabled(false);
		return;
	}
	
	songComboBox->setEnabled(true);
	ResourcesManager* rsmanager = m_project->get_audiosource_manager();
	
	connect(rsmanager, SIGNAL(sourceAdded()), this, SLOT(update_tree_widgets()));
	connect(rsmanager, SIGNAL(clipAdded(AudioClip*)), this, SLOT(clip_added(AudioClip*)));
	connect(rsmanager, SIGNAL(clipRemoved(AudioClip*)), this, SLOT(clip_removed(AudioClip*)));
	connect(rsmanager, SIGNAL(sourceNoLongerUsed(ReadSource*)), this, SLOT(source_nolonger_in_use(ReadSource*)));
	connect(rsmanager, SIGNAL(sourceBackInUse(ReadSource*)), this, SLOT(source_back_in_use(ReadSource*)));
	connect(m_project, SIGNAL(songAdded(Song*)), this, SLOT(song_added(Song*)));
	connect(m_project, SIGNAL(songRemoved(Song*)), this, SLOT(song_removed(Song*)));
}

void ResourcesWidget::update_tree_widgets()
{
	sourcesTreeWidget->clear();
	
	if (!m_project) {
		return;
	}
	
	foreach(ReadSource* rs, resources_manager()->get_all_audio_sources()) {
		QTreeWidgetItem* item = new QTreeWidgetItem(sourcesTreeWidget);
		m_sourceindices.insert(rs->get_id(), item);
		QString duration = frame_to_ms(rs->get_nframes(), 44100);
		item->setText(0, rs->get_short_name());
		item->setText(1, duration);
		item->setText(2, "");
		item->setText(3, "");
		item->setData(0, Qt::UserRole, rs->get_id());
		item->setToolTip(0, rs->get_short_name() + "   " + duration);
		if (!rs->get_ref_count()) {
			item->setForeground(0, QColor(Qt::lightGray));
			item->setForeground(1, QColor(Qt::lightGray));
			item->setForeground(2, QColor(Qt::lightGray));
			item->setForeground(3, QColor(Qt::lightGray));
		}
		
		foreach(AudioClip* clip, resources_manager()->get_clips_for_source(rs)) {
			QTreeWidgetItem* clipitem = new QTreeWidgetItem(item);
			m_clipindices.insert(clip->get_id(), clipitem);
			clipitem->setText(0, clip->get_name());
			QString start = frame_to_ms(clip->get_source_start_frame(), clip->get_rate());
			QString end = frame_to_ms(clip->get_source_end_frame(), clip->get_rate());
			clipitem->setText(1, frame_to_ms(clip->get_length(), clip->get_rate()));
			clipitem->setText(2, start);
			clipitem->setText(3, end);
			clipitem->setData(0, Qt::UserRole, clip->get_id());
			clipitem->setToolTip(0, clip->get_name() + "   " + start + " - " + end);
		
			if (resources_manager()->is_clip_in_use(clip->get_id())) {
				clipitem->setForeground(0, QColor(Qt::lightGray));
				clipitem->setForeground(1, QColor(Qt::lightGray));
				clipitem->setForeground(2, QColor(Qt::lightGray));
				clipitem->setForeground(3, QColor(Qt::lightGray));
			}
		}
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
//	update_tree_widgets();
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

void ResourcesWidget::clip_removed(AudioClip * clip)
{
	QTreeWidgetItem* item = m_clipindices.value(clip->get_id());
	if (!item) return;
	for (int i=0; i<5; ++i) {
		item->setForeground(i, QColor(Qt::lightGray));
	}
}

void ResourcesWidget::clip_added(AudioClip * clip)
{
	printf("clip_added: clip is %lld\n", clip->get_id());
	QTreeWidgetItem* item = m_clipindices.value(clip->get_id());
	if (!item) {
		add_new_clip_entry(clip);
		return;
	}
	for (int i=0; i<5; ++i) {
		item->setForeground(i, QColor(Qt::black));
	}
}

void ResourcesWidget::source_nolonger_in_use(ReadSource * source)
{
	QTreeWidgetItem* item = m_sourceindices.value(source->get_id());
	if (!item) return;
	for (int i=0; i<5; ++i) {
		item->setForeground(i, QColor(Qt::lightGray));
	}
}

void ResourcesWidget::source_back_in_use(ReadSource * source)
{
	QTreeWidgetItem* item = m_sourceindices.value(source->get_id());
	if (!item) return;
	for (int i=0; i<5; ++i) {
		item->setForeground(i, QColor(Qt::black));
	}
}

void ResourcesWidget::add_new_clip_entry(AudioClip * clip)
{
	QTreeWidgetItem* sourceitem = m_sourceindices.value(clip->get_readsource_id());
	
	if (! sourceitem ) return;
	
// 	Q_ASSERT(sourceitem);
	
	QTreeWidgetItem* clipitem = new QTreeWidgetItem(sourceitem);
	m_clipindices.insert(clip->get_id(), clipitem);
	printf("add_new_clip_entry: clip is %lld\n", clip->get_id());
	
	clipitem->setText(0, clip->get_name());
	QString start = frame_to_ms(clip->get_source_start_frame(), clip->get_rate());
	QString end = frame_to_ms(clip->get_source_end_frame(), clip->get_rate());
	clipitem->setText(1, frame_to_ms(clip->get_length(), clip->get_rate()));
	clipitem->setText(2, start);
	clipitem->setText(3, end);
	clipitem->setData(0, Qt::UserRole, clip->get_id());
	clipitem->setToolTip(0, clip->get_name() + "   " + start + " - " + end);
}

