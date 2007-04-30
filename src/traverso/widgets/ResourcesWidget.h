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
class QListView;
class QDirModel;

class ResourcesWidget : public QWidget, protected Ui::ResourcesWidget
{
	Q_OBJECT

public:
	ResourcesWidget(QWidget* parent=0);
	~ResourcesWidget();

	
	
private:
	Project* m_project;
	QListView* m_dirView;
	QDirModel* m_dirModel;
	

private slots:
	void set_project(Project* project);
	void update_tree_widgets();
	void view_combo_box_index_changed(int index);
	void song_combo_box_index_changed(int index);
	void song_added(Song* song);
	void song_removed(Song* song);
	void dirview_item_clicked(const QModelIndex& index );
};

#endif
