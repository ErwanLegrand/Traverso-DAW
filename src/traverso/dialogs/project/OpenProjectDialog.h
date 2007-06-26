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

#ifndef OPEN_PROJECT_DIALOG_H
#define OPEN_PROJECT_DIALOG_H

#include "ui_OpenProjectDialog.h"
#include <QDialog>

class QFileSystemWatcher;

class OpenProjectDialog : public QDialog, protected Ui::OpenProjectDialog
{
Q_OBJECT

public:
	OpenProjectDialog(QWidget* parent = 0);
	~OpenProjectDialog();

	
private:
	QFileSystemWatcher*	m_watcher;
	bool	m_dirchangeDetected;

private slots:
	void update_projects_list();
	void on_loadProjectButton_clicked();
	void on_deleteProjectbutton_clicked();
	void on_projectDirSelectButton_clicked();
	void projectitem_clicked( QTreeWidgetItem* , int  );
	void project_dir_rename_detected(const QString& dirname);
};

#endif

//eof

