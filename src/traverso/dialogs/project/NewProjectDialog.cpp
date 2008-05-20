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

#include "NewProjectDialog.h"
#include "ui_NewProjectDialog.h"

#include <QDir>
#include <QStringList>
#include <QMessageBox>
#include <QTextStream>
#include <QDomDocument>
#include <QFileDialog>
#include <QHeaderView>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QList>
#include <QFileInfo>

#include <Config.h>
#include <Information.h>
#include <ProjectManager.h>
#include <Project.h>
#include <Sheet.h>
#include <Track.h>
#include <Utils.h>
#include "Import.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

NewProjectDialog::NewProjectDialog( QWidget * parent )
	: QDialog(parent)
{
	setupUi(this);
	trackCountSpinBox->setValue(config().get_property("Sheet", "trackCreationCount", 4).toInt());
	
	use_template_checkbox_state_changed(Qt::Unchecked);
	update_template_combobox();

	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

	connect(useTemplateCheckBox, SIGNAL(stateChanged (int)), this, SLOT(use_template_checkbox_state_changed(int)));
	connect(pushButtonAddFiles, SIGNAL(clicked()), this, SLOT(add_files()));
	connect(pushButtonRemoveFiles, SIGNAL(clicked()), this, SLOT(remove_files()));
}

NewProjectDialog::~ NewProjectDialog( )
{}


void NewProjectDialog::accept( )
{

        // do we have the name of the project to create ?
	QString title = newProjectName->text();
	
	if (title.length() == 0) {
		info().information(tr("You must supply a name for the project!") );
		return;
	}


	// first test if project exists already
	if (pm().project_exists(title)) {
		switch (QMessageBox::information(this,
			tr("Traverso - Question"),
			   tr("The Project \"%1\" already exists, do you want to remove it and replace it with a new one ?").arg(title),
			      tr("Yes"), tr("No"), QString::null, 1, -1)) 
		{
			case 0:
				pm().remove_project(title);
				break;
			default:
				return;
				break;
		}
	}

	Project* project;
	
	int numSheets = sheetCountSpinBox->value();
	int numTracks = trackCountSpinBox->value();
	
	int index = templateComboBox->currentIndex();
	bool usetemplate = false;

	if (useTemplateCheckBox->isChecked() && index >= 0) {
		usetemplate = true;
	}
	
	// check which method to use. If there are items in the treeWidgetFiles, ignore
	// settings in the "empty project" tab. Else use settings from "empty project" tab.
	int items = treeWidgetFiles->topLevelItemCount();
	bool loadFiles = false;
	if (items > 0)
	{
		//there are items in the treeWidgetFiles
		loadFiles = true;
		numSheets = 1;
		numTracks = items;
		project = pm().create_new_project(numSheets, numTracks, title);
	} else {
		//no items in the treeWidgetFiles
		if (usetemplate) {
			project = pm().create_new_project(QDir::homePath() + "/.traverso/ProjectTemplates/" + 
					templateComboBox->itemText(index) + ".tpt", title);
		
		} else {
			project = pm().create_new_project(numSheets, numTracks, title);
		}
	}

	if (! project) {
		info().warning(tr("Couldn't create project (%1)").arg(title) );
		return;
	}
	
	project->set_description(descriptionTextEdit->toPlainText());
	project->set_engineer(newProjectEngineer->text());
	project->save();
	delete project;
	
	pm().load_project(title);

	if (loadFiles)
	{
		load_files();
	}

	hide();
}

void NewProjectDialog::use_template_checkbox_state_changed(int state)
{
	if (state == Qt::Checked) {
		templateComboBox->setEnabled(true);
		trackCountSpinBox->setEnabled(false);
	} else {
		templateComboBox->setEnabled(false);
		trackCountSpinBox->setEnabled(true);
	}
}

void NewProjectDialog::update_template_combobox()
{
	QDir templatedir(QDir::homePath() + "/.traverso/ProjectTemplates");
	
	foreach (QString filename, templatedir.entryList(QDir::Files | QDir::NoDotAndDotDot)) {
		templateComboBox->insertItem(0, filename.remove(".tpt"));
	}
}

void NewProjectDialog::add_files()
{
	QStringList list = QFileDialog::getOpenFileNames(this, tr("Open Audio Files"), config().get_property("Project", "directory", "/directory/unknown").toString(), tr("Audio files (*.wav *.flac *.ogg *.mp3 *.wv *.w64)"));

	for(int i = 0; i < list.size(); ++i)
	{
		QStringList labels;
		QFileInfo finfo(list.at(i));
		labels << "Unnamed" << finfo.fileName();

		QTreeWidgetItem* item = new QTreeWidgetItem(treeWidgetFiles, labels, 0);
		item->setData(0, Qt::ToolTipRole, finfo.absoluteFilePath());
		treeWidgetFiles->addTopLevelItem(item);
	}
}

void NewProjectDialog::remove_files()
{
	QList<QTreeWidgetItem*> selection = treeWidgetFiles->selectedItems();

	if (selection.isEmpty())
	{
		return;
	}

	while (!selection.isEmpty())
	{
		QTreeWidgetItem *it = selection.takeLast();
		delete it;
	}
}

void NewProjectDialog::load_files()
{
		Sheet* sheet = pm().get_project()->get_current_sheet();

		int i = 0;
		while(treeWidgetFiles->topLevelItemCount()) {
			QTreeWidgetItem* item = treeWidgetFiles->takeTopLevelItem(0);
			QString f = item->data(0, Qt::ToolTipRole).toString();
			delete item;

			if (i < sheet->get_numtracks())
			{
printf(QS_C(QString("****** %1\n").arg(f)));
				Track* track = sheet->get_track_for_index(i);
				Import* cmd = new Import(track, f, (TimeRef)0.0);
				Command::process_command(cmd);
			}
			++i;
		}
}

//eof
