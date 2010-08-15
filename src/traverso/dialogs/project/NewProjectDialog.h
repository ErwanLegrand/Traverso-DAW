/*
    Copyright (C) 2007-2008 Remon Sijrier 
 
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
class ExportFormatOptionsWidget;
class QButtonGroup;
struct ExportSpecification;


class NewProjectDialog : public QDialog, protected Ui::NewProjectDialog
{
	Q_OBJECT

public:
	NewProjectDialog(QWidget* parent = 0);
	~NewProjectDialog();

	AudioFileCopyConvert* get_converter();

private:
	AudioFileCopyConvert* m_converter;
	ExportSpecification* m_exportSpec;
	ExportFormatOptionsWidget* m_formatOptionsWidget;
	QButtonGroup* m_buttonGroup;

	void load_all_files();
	void copy_files();

private slots:
	void accept();
	void use_template_checkbox_state_changed(int state);
	void update_template_combobox();
	void add_files();
	void remove_files();
	void load_file(QString, int, QString);
	void move_up();
	void move_down();
        void on_changeProjectsDirButton_clicked();
        void update_projects_directory_line_edit();

signals:
	void loadFile(QString, int, QString);
        void numberOfFiles(int);
};

#endif

//eof

