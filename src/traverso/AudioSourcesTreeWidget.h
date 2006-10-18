/*
Copyright (C) 2006 Remon Sijrier 

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

$Id: AudioSourcesTreeWidget.h,v 1.1 2006/10/18 12:09:47 r_sijrier Exp $
*/
 
#ifndef AUDIO_SOURCES_TREE_WIDGET_H
#define AUDIO_SOURCES_TREE_WIDGET_H

#include <QTreeWidget>

#include <QList>
#include <QVariant>

class Project;

class TreeItem
{
public:
	TreeItem(const QList<QVariant> &data, TreeItem *parent = 0);
	~TreeItem();

	void appendChild(TreeItem *child);

	TreeItem *child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	int row() const;
	TreeItem *parent();

private:
	QList<TreeItem*> childItems;
	QList<QVariant> itemData;
	TreeItem *parentItem;
};


class TreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TreeModel(const QString &data, QObject *parent = 0);
	~TreeModel();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
				int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
				const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
	QList<TreeItem*> parents;
	TreeItem*	rootItem;
	Project*	m_project;
	
private slots:
	void setupModelData();
	void set_project(Project* project);
};

#endif
