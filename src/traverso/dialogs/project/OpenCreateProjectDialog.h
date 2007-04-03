/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: OpenCreateProjectDialog.h,v 1.1 2007/04/03 19:05:38 r_sijrier Exp $
*/

#ifndef PROJECT_MANAGER_DIALOG_H
#define PROJECT_MANAGER_DIALOG_H

#include "ui_OpenCreateProjectDialog.h"
#include <QDialog>

class OpenCreateProjectDialog : public QDialog, protected Ui::OpenCreateProjectDialog
{
Q_OBJECT

public:
	OpenCreateProjectDialog(QWidget* parent = 0);
	~OpenCreateProjectDialog();

private slots:
	void update_projects_list();
	void on_loadProjectButton_clicked();
	void on_createProjectButton_clicked();
	void on_deleteProjectbutton_clicked();
	void on_projectDirSelectButton_clicked();
	void projectitem_clicked( QTreeWidgetItem* , int  );
};

#endif

//eof

