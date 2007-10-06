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

#ifndef RESTORE_PROJECT_BACKUP_DIALOG_H
#define RESTORE_PROJECT_BACKUP_DIALOG_H

#include "ui_RestoreProjectBackupDialog.h"
#include <QDialog>

class Project;

class RestoreProjectBackupDialog : public QDialog, protected Ui::RestoreProjectBackupDialog
{
	Q_OBJECT

public:
	RestoreProjectBackupDialog(QWidget* parent = 0);
	~RestoreProjectBackupDialog() {};
	
	void set_project_name(const QString& projectname);
	
private:
	QString m_projectname;
	void accept();
	void reject();
	void populate_treeview();
};

#endif

//eof
