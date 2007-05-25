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

#include <Config.h>
#include <Information.h>
#include <ProjectManager.h>
#include <Project.h>
#include <Utils.h>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

NewProjectDialog::NewProjectDialog( QWidget * parent )
	: QDialog(parent)
{
	setupUi(this);
	trackCountSpinBox->setValue(config().get_property("Song", "trackCreationCount", 4).toInt());
	
	use_template_checkbox_state_changed(Qt::Unchecked);
	update_template_combobox();

	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

	connect(useTemplateCheckBox, SIGNAL(stateChanged (int)), this, SLOT(use_template_checkbox_state_changed(int)));
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
	
	int numSongs = songCountSpinBox->value();
	int numTracks = trackCountSpinBox->value();
	
	int index = templateComboBox->currentIndex();
	bool usetemplate = false;
	if (useTemplateCheckBox->isChecked() && index >= 0) {
		usetemplate = true;
	}
	
	Project* project;
	
	if (usetemplate) {
		project = pm().create_new_project(QDir::homePath() + "/.traverso/ProjectTemplates/" + 
				templateComboBox->itemText(index) + ".tpt", title);
		
	} else {
		project = pm().create_new_project(numSongs, numTracks, title);
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


//eof
