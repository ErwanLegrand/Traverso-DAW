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

#include "NewProjectDialog.h"
#include "ui_NewProjectDialog.h"

#include <QDir>
#include <QStringList>
#include <QMessageBox>
#include <QTextStream>
#include <QDomDocument>
#include <QFileDialog>
#include <QHeaderView>
#include <QToolButton>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QList>
#include <QFileInfo>
#include <QFile>
#include <QCheckBox>
#include <QRadioButton>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QIcon>

#include <Config.h>
#include "Export.h"
#include "Information.h"
#include "ProjectManager.h"
#include "ResourcesManager.h"
#include <Project.h>
#include <Sheet.h>
#include <Track.h>
#include <Utils.h>
#include <CommandGroup.h>
#include "Import.h"
#include "AudioFileCopyConvert.h"
#include "ReadSource.h"

#include "widgets/ExportFormatOptionsWidget.h"

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

	buttonAdd->setIcon(QIcon(":/add"));
	buttonRemove->setIcon(QIcon(":/remove"));
	buttonUp->setIcon(QIcon(":/up"));
	buttonDown->setIcon(QIcon(":/down"));

	buttonRemove->setEnabled(false);
	buttonUp->setEnabled(false);
	buttonDown->setEnabled(false);

	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

	m_converter = new AudioFileCopyConvert();
	m_exportSpec = new ExportSpecification;
	m_buttonGroup = new QButtonGroup(this);
	m_buttonGroup->addButton(radioButtonImport, 0);
	m_buttonGroup->addButton(radioButtonEmpty, 1);

	connect(useTemplateCheckBox, SIGNAL(stateChanged (int)), this, SLOT(use_template_checkbox_state_changed(int)));
	connect(buttonAdd, SIGNAL(clicked()), this, SLOT(add_files()));
	connect(buttonRemove, SIGNAL(clicked()), this, SLOT(remove_files()));
	connect(buttonUp, SIGNAL(clicked()), this, SLOT(move_up()));
	connect(buttonDown, SIGNAL(clicked()), this, SLOT(move_down()));

	connect(m_converter, SIGNAL(taskFinished(QString, int, QString)), this, SLOT(load_file(QString, int, QString)));
	connect(m_buttonGroup, SIGNAL(buttonClicked(int)), stackedWidget, SLOT(setCurrentIndex(int)));
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

	if (loadFiles) {
		if (checkBoxCopy->isChecked()) {
			copy_files();
		} else {
			load_all_files();
		}
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
	QStringList list = QFileDialog::getOpenFileNames(this, tr("Open Audio Files"),
			config().get_property("Project", "directory", "/directory/unknown").toString(),
			tr("Audio files (*.wav *.flac *.ogg *.mp3 *.wv *.w64)"));

	for(int i = 0; i < list.size(); ++i)
	{
		QStringList labels;
		QFileInfo finfo(list.at(i));
		labels << "Unnamed" << finfo.fileName();

		QTreeWidgetItem* item = new QTreeWidgetItem(treeWidgetFiles, labels, 0);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
		item->setData(0, Qt::ToolTipRole, finfo.absoluteFilePath());
		treeWidgetFiles->addTopLevelItem(item);
	}

	if (treeWidgetFiles->topLevelItemCount()) {
		buttonRemove->setEnabled(true);
		buttonUp->setEnabled(true);
		buttonDown->setEnabled(true);
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

	if (!treeWidgetFiles->topLevelItemCount()) {
		buttonRemove->setEnabled(false);
		buttonUp->setEnabled(false);
		buttonDown->setEnabled(false);
	}
}

void NewProjectDialog::copy_files()
{
	emit number_of_files(treeWidgetFiles->topLevelItemCount());

	QList<QFileInfo> list;
	QStringList trackNameList;
	while(treeWidgetFiles->topLevelItemCount()) {
		QTreeWidgetItem* item = treeWidgetFiles->takeTopLevelItem(0);
		list.append(QFileInfo(item->data(0, Qt::ToolTipRole).toString()));
		trackNameList.append(item->text(0));
		delete item;
	}

	QString destination = pm().get_project()->get_root_dir() + "/audiosources/";
	
	// copy to project dir
	for (int n = 0; n < list.size(); ++n)
	{
		QString fn = destination + list.at(n).fileName();

		// TODO: check for free disk space
		
		// TODO: offer file format conversion while copying: format options widget not there yet.
//		m_formatOptionsWidget->get_format_options(m_exportSpec);

		ReadSource* readsource = resources_manager()->import_source(list.at(n).absolutePath() + "/", list.at(n).fileName());

		if (readsource) {
			m_converter->enqueue_task(readsource, m_exportSpec, destination, list.at(n).fileName(), n, trackNameList.at(n));
		}
	}
}

void NewProjectDialog::load_all_files()
{
	int i = 0;

	while(treeWidgetFiles->topLevelItemCount()) {
		QTreeWidgetItem* item = treeWidgetFiles->takeTopLevelItem(0);
		QString f = item->data(0, Qt::ToolTipRole).toString();
		QString n = item->text(0);
		delete item;

		load_file(f, i, n);
		++i;
	}
}

void NewProjectDialog::load_file(QString name, int i, QString trackname)
{
	Sheet* sheet = pm().get_project()->get_current_sheet();

	if (!sheet) {
		return;
	}

	if (i >= sheet->get_numtracks()) {
		return;
	}

	Track* track = sheet->get_track_for_index(i);

	Import* import = new Import(name);
	track->set_name(trackname);
	import->set_track(track);
	import->set_position((TimeRef)0.0);
	if (import->create_readsource() != -1) {
		Command::process_command(import);
	}
}

void NewProjectDialog::move_up()
{
	QList<QTreeWidgetItem*> selection = treeWidgetFiles->selectedItems();

	if (selection.isEmpty())
	{
		return;
	}

	qSort(selection);
	int firstIndex = treeWidgetFiles->topLevelItemCount();
	QList<int> indexList;

	foreach(QTreeWidgetItem *it, selection) {
	    int idx = treeWidgetFiles->indexOfTopLevelItem(it);
	    firstIndex = qMin(idx, firstIndex);
	}

	firstIndex = firstIndex > 0 ? firstIndex - 1 : firstIndex;

	QList<QTreeWidgetItem*> tempList;
	while (selection.size())
	{
		QTreeWidgetItem *it = treeWidgetFiles->takeTopLevelItem(treeWidgetFiles->indexOfTopLevelItem(selection.takeFirst()));
		treeWidgetFiles->insertTopLevelItem(firstIndex, it);
		it->setSelected(true);
		++firstIndex;
	}
}

void NewProjectDialog::move_down()
{
	QList<QTreeWidgetItem*> selection = treeWidgetFiles->selectedItems();

	if (selection.isEmpty())
	{
		return;
	}

	qSort(selection);
	int firstIndex = 0;
	QList<int> indexList;

	foreach(QTreeWidgetItem *it, selection) {
	    int idx = treeWidgetFiles->indexOfTopLevelItem(it);
	    firstIndex = qMax(idx, firstIndex);
	}

	firstIndex = firstIndex < treeWidgetFiles->topLevelItemCount() - 1 ? firstIndex + 1 : firstIndex;

	while (selection.size()) {
		int idx = treeWidgetFiles->indexOfTopLevelItem(selection.takeFirst());
		QTreeWidgetItem *it = treeWidgetFiles->takeTopLevelItem(idx);
		treeWidgetFiles->insertTopLevelItem(firstIndex, it);
		it->setSelected(true);
	}
}

AudioFileCopyConvert* NewProjectDialog::get_converter()
{
	return m_converter;
}
//eof
