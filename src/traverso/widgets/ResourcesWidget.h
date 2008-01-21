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
 
#ifndef RESOURCESWIDGET_H
#define RESOURCESWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "ui_ResourcesWidget.h"

class Project;
class Sheet;
class AudioClip;
class ReadSource;
class SourceTreeItem;
class QShowEvent;
class QListView;
class QDirModel;
class QComboBox;

class FileWidget : public QWidget
{
	Q_OBJECT
public:

	FileWidget(QWidget* parent=0) : QWidget(parent) {
		m_dirModel = 0;
	}

	void showEvent ( QShowEvent * event );
	void set_current_path(const QString& path) const;

private slots:
	void dirview_item_clicked(const QModelIndex & index);
	void dir_up_button_clicked();
	void refresh_button_clicked();
	void box_actived(int i);

private:
	QListView* m_dirView;
	QDirModel* m_dirModel;
	QComboBox* m_box;
};


class ClipTreeItem : public QObject, public QTreeWidgetItem
{
	Q_OBJECT
	
public:
	ClipTreeItem(SourceTreeItem* parent, AudioClip* clip);
	
	void apply_filter(Sheet* sheet);


public slots:
	void clip_state_changed();	

private:
	AudioClip* m_clip;
};

class SourceTreeItem : public QObject, public QTreeWidgetItem
{
	Q_OBJECT
	
public:
	SourceTreeItem(QTreeWidget* parent, ReadSource* source);

	void apply_filter(Sheet* sheet);

private:
	ReadSource* m_source;
	
public slots:
	void source_state_changed();
};

class ResourcesWidget : public QWidget, protected Ui::ResourcesWidget
{
	Q_OBJECT

public:
	ResourcesWidget(QWidget* parent=0);
	~ResourcesWidget();

protected:
	void showEvent( QShowEvent * event );
	void resizeEvent( QResizeEvent * e );

private:
	Project* m_project;
	Sheet* m_currentSheet;
	FileWidget* m_filewidget;
	QHash<qint64, ClipTreeItem*> m_clipindices;
	QHash<qint64, SourceTreeItem*> m_sourceindices;
	
	void update_clip_state(AudioClip* clip);
	void update_source_state(qint64 id);
	
	void filter_on_current_sheet();
	
private slots:
	void set_project(Project* project);
	void project_load_finished();
	
	void view_combo_box_index_changed(int index);
	void sheet_combo_box_index_changed(int index);
	
	void sheet_added(Sheet* sheet);
	void sheet_removed(Sheet* sheet);
	void set_current_sheet(Sheet* sheet);
	
	void add_clip(AudioClip* clip);
	void remove_clip(AudioClip* clip);
	void add_source(ReadSource* source);
	void remove_source(ReadSource* source);
};


#endif
