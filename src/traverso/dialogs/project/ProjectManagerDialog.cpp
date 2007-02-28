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
 
    $Id: ProjectManagerDialog.cpp,v 1.1 2007/02/28 17:26:16 r_sijrier Exp $
*/

#include "ProjectManagerDialog.h"
#include "ui_ProjectManagerDialog.h"

#include "libtraversocore.h"
#include <QSettings>
#include <QDir>
#include <QStringList>
#include <QMessageBox>
#include <QTextStream>
#include <QDomDocument>
#include <QFileDialog>
#include <QHeaderView>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ProjectManagerDialog::ProjectManagerDialog( QWidget * parent )
	: QDialog(parent)
{
	setupUi(this);
	projectListView->setColumnCount(3);
	update_projects_list();
	QStringList stringList;
	stringList << "Project Name" << "Songs" << "Status" ;
	projectListView->setHeaderLabels(stringList);
	
	projectListView->header()->resizeSection(0, 200);

	connect(projectListView, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(projectitem_clicked(QTreeWidgetItem*,int)));
}

ProjectManagerDialog::~ ProjectManagerDialog( )
{}

void ProjectManagerDialog::update_projects_list()
{
	projectListView->clear();
	QSettings settings;
	QString projsDir = settings.value("Project/directory").toString();

	QDir pDir( projsDir );

	QFileInfoList list = pDir.entryInfoList();
	QFileInfo fi;
	QString fileName;
	for( int i = 0; i < list.size(); ++i ) {
		fi = list.at(i);
		fileName = fi.fileName();
		if ( (fileName != ".") && (fileName != "..") ) {
			/************ FROM HERE ****************/
			QDomDocument doc("Project");
			QString fileToOpen = projsDir + fileName + "/project.traverso";
			QFile file(fileToOpen);

			if (!file.open(QIODevice::ReadOnly)) {
				PWARN("Cannot open project properties file (%s)", fileToOpen.toAscii().data());
				return;
			}

			QString errorMsg;
			if (!doc.setContent(&file, &errorMsg)) {
				file.close();
				PWARN("Cannot set content of XML file (%s)", errorMsg.toAscii().data());
				return;
			}

			file.close();

			QDomElement docElem = doc.documentElement();
			QDomNode propertiesNode = docElem.firstChildElement("Properties");
			QDomElement e = propertiesNode.toElement();

			QDomNode songsNode = docElem.firstChildElement("Songs");
			QDomNode songNode = songsNode.firstChild();
			int songCounter = 0;
                        // count to get Songs number....
			while(!songNode.isNull()) {
				songCounter++;
				songNode = songNode.nextSibling();
			}

			QString sNumSongs = QString::number(songCounter);
			QString engineer;
			QString title;

			title = e.attribute( "title", "" );
			engineer = e.attribute( "engineer", "" );

			/*********** TO HERE THIS CODE IS DUPLICATE FROM THAT IN PROJECT.CC :-( Don't know if this is avoidable at all *********/

			QString status;
			if (pm().get_project() && (pm().get_project()->get_title() == title))
				status = pm().get_project()->has_changed()?"Unsaved":"Saved";
			else
				status="Saved";

			QTreeWidgetItem* item = new QTreeWidgetItem(projectListView);
			item->setTextAlignment(0, Qt::AlignLeft);
			item->setTextAlignment(1, Qt::AlignHCenter);
			item->setText(0,title);
			item->setText(1,sNumSongs);
			item->setText(2,status);
		}
	}
        /*	if (pm().get_project())
	selectedProjectName->setText( pm().get_project()->get_title() );*/
}

void ProjectManagerDialog::projectitem_clicked( QTreeWidgetItem* item, int)
{
	if (item)
		selectedProjectName->setText(item->text(0));
}

void ProjectManagerDialog::on_loadProjectButton_clicked( )
{
        // do we have the name of the project to load ?
	QString title;
	if (projectListView->currentItem())
		title = projectListView->currentItem()->text(0);

	if (title.isEmpty()) {
		info().warning(tr("No Project selected!") );
		info().information(tr("Select a project and click the 'Load' button again") );
		return;
	}

        // ask if the current project should first be saved
	if (pm().get_project() && pm().get_project()->has_changed())
		switch (QMessageBox::information(this,
			"Traverso - Question",
   "Should the current project be saved ?",
   tr("Yes"), tr("No"), QString::null, 0, -1)) {
	   case -1:
		   return;
		   break;
	   case 0:
		   pm().get_project()->save();
		   break;
	   default:
		   break;
   }

        // first test if project exists
   if (!pm().project_exists(title)) {
	   info().warning(tr("Project does not exist! (%1)").arg(title));
	   return;
   }
   if (pm().load_project(title)<0) {
	   PERROR("Could not load project %s", title.toAscii().data());
   }
}

void ProjectManagerDialog::on_createProjectButton_clicked( )
{

        // do we have the name of the project to create ?
	QString title;
	title = newProjectName->text();
	if (title.length() == 0) {
		info().information(tr("You must supply a name for the project!") );
		return;
	}

        // ask if the current project should first be saved (only when status is "UnSaved")
	if (pm().get_project() && pm().get_project()->has_changed())
		switch (QMessageBox::information(this,
			tr("Traverso - Question"),
			   tr("Should the current project be saved ?"),
			      tr("Yes"), tr("No"), QString::null, 0, -1)) {
				      case -1:
					      return;
					      break;
				      case 0:
					      pm().get_project()->save();
					      break;
				      default:
					      break;
			      }


        // first test if project exists already
			      if (pm().project_exists(title)) {
				      switch (QMessageBox::information(this,
					      tr("Traverso - Question"),
							      tr("The Project \"%1\" already exists, do you want to remove it and replace it with a new one ?").arg(title),
									      tr("Yes"), tr("No"), QString::null, 1, -1)) {
										case 0:
										pm().remove_project(title);
										break;
										default:
										return;
										break;
									      }
			      }
			      QString sNumSongs = numberOfSongs->text();
			      bool ok;
			      int numSongs = sNumSongs.toInt(&ok, 10);
        //When input is not a decimal number, set it to a sane default value
			      if (!ok)
				      numSongs = 2;

			      if( pm().create_new_project(title, numSongs) < 0)
				      info().warning(tr("Couldn't create project (%1)").arg(title) );
			      else {
				      update_projects_list();
			      }
}

void ProjectManagerDialog::on_deleteProjectbutton_clicked( )
{
        // do we have the name of the project to delete ?
	QString title = selectedProjectName->text();

	if (title.isEmpty()) {
		info().information(tr("You must supply a name for the project!") );
		return;
	}

        // first test if project exists
	if (!pm().project_exists(title)) {
		info().warning(tr("Project does not exist! (%1)").arg(title));
		return;
	}

	switch (QMessageBox::information(this,
		tr("Traverso - Question"),
		   tr("Are you sure that you want to remove the project %1 ? It's not possible to undo it !").arg(title).toAscii().data(),
		      "Yes", "No", QString::null, 1, -1)) {
			      case 0:
				      pm().remove_project(title);
				      update_projects_list();
				      break;
			      default:
				      return;
				      break;
		      }
		      return;
}

void ProjectManagerDialog::on_saveAsProjectButton_clicked( )
{
	if (pm().get_project()) {
		pm().get_project()->set_title(selectedProjectName->text());
		pm().get_project()->save();
	}
	selectedProjectName->setText("");
	update_projects_list();
}

void ProjectManagerDialog::on_projectDirSelectButton_clicked( )
{
	QSettings settings;
	
	QString projects_path = QDir::homePath();
	
	QString newPath = QFileDialog::getExistingDirectory(0,
			tr("Choose an existing or create a new Project Directory"), projects_path);
			
	if (newPath.isEmpty() || newPath.isNull()) {
		return;
	}
	
	QDir dir;
	
	if (dir.exists(newPath)) {
// 		QMessageBox::information( interface, tr("Traverso - Information"), tr("Using existing Project directory: %1\n").arg(newPath), "OK", 0 );
	} else if (!dir.mkpath(newPath)) {
		QMessageBox::warning( this, tr("Traverso - Warning"), tr("Unable to create Project directory! \n") +
				tr("Please check permission for this directory: %1").arg(newPath) );
		return;
	} else {
		QMessageBox::information( this, tr("Traverso - Information"), tr("Created new Project directory for you here: %1\n").arg(newPath), "OK", 0 );
	}
	
	settings.setValue("Project/directory", newPath);
	
	update_projects_list();
}


//eof
