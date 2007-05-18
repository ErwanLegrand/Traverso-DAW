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
#include "ui_ResourcesWidget.h"

class Project;
class Song;
class FileWidget;
class AudioClip;
class ReadSource;
class QTreeWidgetItem;

class ClipTreeItem : public QObject, public QTreeWidgetItem
{
	Q_OBJECT
	
public:
	ClipTreeItem(QTreeWidgetItem* parent, AudioClip* clip);


public slots:
	void clip_state_changed();	

private:
	AudioClip* m_clip;
};


class ResourcesWidget : public QWidget, protected Ui::ResourcesWidget
{
	Q_OBJECT

public:
	ResourcesWidget(QWidget* parent=0);
	~ResourcesWidget();

	
	
private:
	Project* m_project;
	FileWidget* m_filewidget;
	QHash<qint64, ClipTreeItem*> m_clipindices;
	QHash<qint64, QTreeWidgetItem*> m_sourceindices;
	
	void update_clip_state(AudioClip* clip);
	void update_source_state(qint64 id);
	
private slots:
	void set_project(Project* project);
	void populate_tree();
	
	void view_combo_box_index_changed(int index);
	void song_combo_box_index_changed(int index);
	
	void song_added(Song* song);
	void song_removed(Song* song);
	
	void add_clip(AudioClip* clip);
	void remove_clip(AudioClip* clip);
	void add_source(ReadSource* source);
	void remove_source(ReadSource* source);
};


#endif
