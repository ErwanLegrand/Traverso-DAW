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

#ifndef PROJECT_CONVERTER_DIALOG_H
#define PROJECT_CONVERTER_DIALOG_H

#include "ui_ProjectConverterDialog.h"
#include <QDialog>

class Project;
class ProjectConverter;

class ProjectConverterDialog : public QDialog, protected Ui::ProjectConverterDialog
{
	Q_OBJECT

public:
	ProjectConverterDialog(QWidget* parent = 0);
	~ProjectConverterDialog() {};
	
	void set_project(const QString& rootdir, const QString& name);
	
private:
	ProjectConverter* m_converter;
	QString m_projectname;
	
	void accept();
	void reject();
	
private slots:
	void file_merge_started(QString);
	void file_merge_finished(QString);
	void converter_messages(QString);
	void conversion_finished();
	void on_loadProjectButton_clicked();
};

#endif

//eof
