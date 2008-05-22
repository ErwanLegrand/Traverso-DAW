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

#ifndef NEW_PROJECT_DIALOG_H
#define NEW_PROJECT_DIALOG_H

#include "ui_NewProjectDialog.h"
#include <QDialog>

class AudioFileCopyConvert;
class QProgressDialog;

class NewProjectDialog : public QDialog, protected Ui::NewProjectDialog
{
	Q_OBJECT

public:
	NewProjectDialog(QWidget* parent = 0);
	~NewProjectDialog();

private:
	AudioFileCopyConvert* m_converter;
	QProgressDialog* m_progressDialog;

	void load_all_files();
	void copy_files();

private slots:
	void accept();
	void use_template_checkbox_state_changed(int state);
	void update_template_combobox();
	void add_files();
	void remove_files();
	void load_file(QString, int);
	void show_progress(QString);

signals:
	void loadFile(QString, int);
};

#endif

//eof

