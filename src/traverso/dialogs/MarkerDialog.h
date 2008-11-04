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

#ifndef MARKER_DIALOG_H
#define MARKER_DIALOG_H

#include "ui_MarkerDialog.h"
#include <QDialog>

class Project;
class Marker;
class Sheet;

class MarkerDialog : public QDialog, protected Ui::MarkerDialog
{
	Q_OBJECT

public:
	MarkerDialog(QWidget* parent = 0);
	~MarkerDialog() {};
	
private:
	Project* m_project;
	Marker* m_marker;
	Sheet* m_sheet;

	Marker* get_marker(qint64);
	void next_item(QLineEdit *);

private slots:
	void update_marker_treeview();
	void item_changed(QTreeWidgetItem *, QTreeWidgetItem *);
	void description_changed(const QString &);
	void position_changed(const QString &);
	void remove_marker();
	void export_toc();
	void apply();
	void cancel();

	void title_enter();
	void position_enter();
	void performer_enter();
	void composer_enter();
	void arranger_enter();
	void sheetwriter_enter();
	void message_enter();
	void isrc_enter();

	void title_all();
	void performer_all();
	void composer_all();
	void arranger_all();
	void sheetwriter_all();
	void message_all();
	void copy_all();
	void pemph_all();
};

#endif
