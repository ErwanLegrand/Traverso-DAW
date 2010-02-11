/**
    Copyright (C) 2005-2007 Remon Sijrier 
 
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

#include "ExportDialog.h"
#include "ui_ExportDialog.h"

#include <QFileDialog>
#include <QCloseEvent>

#include "Export.h"
#include "Information.h"
#include "Project.h"
#include "ProjectManager.h"
#include "Sheet.h"

#include "widgets/ExportFormatOptionsWidget.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



ExportDialog::ExportDialog( QWidget * parent )
	: QDialog(parent)
	, m_exportSpec(0)
{
        setupUi(this);
        
        QBoxLayout* lay = qobject_cast<QBoxLayout*>(layout());
        if (lay) {
		m_formatOptionsWidget = new ExportFormatOptionsWidget(lay->widget());
		lay->insertWidget(1, m_formatOptionsWidget);
	}

	abortButton->hide();
	QIcon icon = QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon);
	fileSelectButton->setIcon(icon);
	
	set_project(pm().get_project());
	
	setMaximumSize(400, 450);
}


ExportDialog::~ ExportDialog( )
{
}


bool ExportDialog::is_safe_to_export()
{
	PENTER;
	if (m_project->is_recording()) {
		info().warning(tr("Export during recording is not supported!"));
		return false;
	}
	
	return true;
}


void ExportDialog::on_startButton_clicked( )
{
	if (!is_safe_to_export()) {
		return;
	}
	
	if (exportDirName->text().isEmpty()) {
		info().warning(tr("No Export Direcoty was given, please supply one first!"));
		return;
	}
	
	connect(m_project, SIGNAL(sheetExportProgressChanged(int)), this, SLOT(update_sheet_progress(int)));
	connect(m_project, SIGNAL(overallExportProgressChanged(int)), this, SLOT(update_overall_progress(int)));
	connect(m_project, SIGNAL(exportFinished()), this, SLOT(render_finished()));
	connect(m_project, SIGNAL(exportStartedForSheet(Sheet*)), this, SLOT (set_exporting_sheet(Sheet*)));
        connect(m_project, SIGNAL(exportMessage(QString)), this, SLOT(set_export_message(QString)));

	// clear extraformats, it might be different now from previous runs!
	m_exportSpec->extraFormat.clear();
	
	m_formatOptionsWidget->get_format_options(m_exportSpec);
	
	if (allSheetsButton->isChecked()) {
                m_exportSpec->allSheets = true;
	} else {
                m_exportSpec->allSheets = false;
	}
	
	m_exportSpec->exportdir = exportDirName->text();
	if (m_exportSpec->exportdir.size() > 1 && (m_exportSpec->exportdir.at(m_exportSpec->exportdir.size()-1).decomposition() != "/")) {
		m_exportSpec->exportdir = m_exportSpec->exportdir.append("/");
	}
	QString name = m_exportSpec->exportdir;
	QFileInfo fi(m_exportSpec->name);
	name += fi.completeBaseName() + ".toc";
	m_exportSpec->tocFileName = name;

	m_exportSpec->isRecording = false;
	
	if (m_project->export_project(m_exportSpec) == -1) {
		return;
	}
	
	startButton->hide();
	closeButton->hide();
	abortButton->show();
}


void ExportDialog::on_closeButton_clicked()
{
	hide();
}


void ExportDialog::on_abortButton_clicked( )
{
	m_exportSpec->stop = true;
	m_exportSpec->breakout = true;
}


void ExportDialog::on_fileSelectButton_clicked( )
{
	if (!m_project) {
		info().information(tr("No project loaded, to export a project, load it first!"));
		return;
	}
	
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Choose/create an export directory"), m_exportSpec->exportdir);
	
	if (!dirName.isEmpty()) {
		exportDirName->setText(dirName);
	}
}


void ExportDialog::update_sheet_progress( int progress )
{
}

void ExportDialog::update_overall_progress( int progress )
{
	progressBar->setValue(progress);
}

void ExportDialog::set_export_message(QString message)
{
        currentProcessingSheetName->setText(message);
}

void ExportDialog::render_finished( )
{
	disconnect(m_project, SIGNAL(sheetExportProgressChanged(int)), this, SLOT(update_sheet_progress(int)));
	disconnect(m_project, SIGNAL(overallExportProgressChanged(int)), this, SLOT(update_overall_progress(int)));
	disconnect(m_project, SIGNAL(exportFinished()), this, SLOT(render_finished()));
	disconnect(m_project, SIGNAL(exportStartedForSheet(Sheet*)), this, SLOT (set_exporting_sheet(Sheet*)));
	
	startButton->show();
	closeButton->show();
	abortButton->hide();
	progressBar->setValue(0);
}

void ExportDialog::set_exporting_sheet( Sheet * sheet )
{
	QString name = tr("Progress of Sheet ") + 
		QString::number(m_project->get_sheet_index(sheet->get_id())) + ": " +
                sheet->get_name();
	
	currentProcessingSheetName->setText(name);
}

void ExportDialog::set_project(Project * project)
{
	m_project = project;
	if (! m_project) {
		info().information(tr("No project loaded, to export a project, load it first!"));
		setEnabled(false);
		if (m_exportSpec) {
			delete m_exportSpec;
			m_exportSpec = 0;
		}
	} else {
		setEnabled(true);
		if (m_exportSpec) {
			delete m_exportSpec;
			m_exportSpec = 0;
		}
		m_exportSpec = new ExportSpecification;
		m_exportSpec->exportdir = m_project->get_root_dir() + "/Export/";
		m_exportSpec->renderfinished = false;
		exportDirName->setText(m_exportSpec->exportdir);
	}
}



void ExportDialog::closeEvent(QCloseEvent * event)
{
	if (closeButton->isHidden()) {
		event->setAccepted(false);
		return;
	}
	QDialog::closeEvent(event);
}

void ExportDialog::reject()
{
	if (closeButton->isHidden()) {
		return;
	}
	hide();
}

