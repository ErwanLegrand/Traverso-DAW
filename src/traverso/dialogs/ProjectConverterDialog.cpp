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

#include "ProjectConverterDialog.h"

#include "ProjectConverter.h"
#include "ProjectManager.h"
#include <QScrollBar>

ProjectConverterDialog::ProjectConverterDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
	m_converter = new ProjectConverter;
	
	stopConversionButton->hide();
	loadProjectButton->hide();
	
	connect(m_converter, SIGNAL(progress(int)), progressBar, SLOT(setValue(int)));
	connect(m_converter, SIGNAL(fileMergeStarted(QString)), this, SLOT(file_merge_started(QString)));
	connect(m_converter, SIGNAL(fileMergeFinished(QString)), this, SLOT(file_merge_finished(QString)));
	connect(m_converter, SIGNAL(message(QString)), this, SLOT(converter_messages(QString)));
	connect(m_converter, SIGNAL(conversionFinished()), this, SLOT(conversion_finished()));
}


void ProjectConverterDialog::accept()
{
	stopConversionButton->show();
	startButton->hide();
	closeButton->hide();
	
	m_converter->start();
}

void ProjectConverterDialog::reject()
{
	hide();
}

void ProjectConverterDialog::set_project(const QString & rootdir, const QString & name)
{
	m_projectname = name;
	projectNameLable->setText(tr("Conversion needed for Project: <b>'%1'</b>").arg(name));
	m_converter->set_project(rootdir, name);
	conversionInfoText->append(m_converter->get_conversion_description());
}

void ProjectConverterDialog::file_merge_started(QString filename)
{
	taskTextBrowswer->append(tr("Converting file %1").arg(filename));
}

void ProjectConverterDialog::file_merge_finished(QString filename)
{
// 	taskTextBrowswer->append(tr("Finished converting %1").arg(filename));
}

void ProjectConverterDialog::converter_messages(QString message)
{
	taskTextBrowswer->append(message);
}

void ProjectConverterDialog::conversion_finished()
{
	loadProjectButton->show();
	closeButton->show();
	startButton->hide();
	stopConversionButton->hide();
}

void ProjectConverterDialog::on_loadProjectButton_clicked()
{
	pm().load_project(m_projectname);
	reject();
}

