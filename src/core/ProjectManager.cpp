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

$Id: ProjectManager.cpp,v 1.8 2006/08/08 19:37:03 r_sijrier Exp $
*/

#include "ProjectManager.h"

#include <QSettings>
#include <QApplication>
#include <QFileInfo>

#include "Project.h"
#include "Song.h"
#include "AudioSourceManager.h"
#include "Information.h"
#include "FileHelpers.h"
#include <AudioDevice.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


ProjectManager::ProjectManager()
		: ContextItem()
{
	PENTERCONS;
	currentProject = (Project*) 0;

	cpointer().add_contextitem(this);
}

ProjectManager& pm()
{
	static ProjectManager projMan;
	return projMan;
}

int ProjectManager::save_song(QString songName)
{
	if (currentProject) {
		currentProject->save();
	}

	return 0;
}

void ProjectManager::set_current_project(Project* pProject)
{
	PENTER;
	
	if (currentProject) {
		currentProject->save();
		delete currentProject;
	}

	currentProject = pProject;

	emit projectLoaded(currentProject);

	QSettings settings;
	
	QString title = "";
	
	if (currentProject) {
		title = currentProject->get_title();
		settings.setValue("Project/current", title);
	}
	
}

int ProjectManager::create_new_project(QString projectName, int numSongs)
{
	PENTER;

	if (project_exists(projectName)) {
		PERROR("project %s already exists\n", projectName.toAscii().data());
		return -1;
	}
	
	Project *newProject = new Project(projectName);
	
	if (newProject->create(numSongs) < 0) {
		delete newProject;
		PERROR("couldn't create new project %s", projectName.toAscii().data());
		return -1;
	}

	return 0;
}

int ProjectManager::load_project(QString projectName)
{
	PENTER;
	
	if( ! project_exists(projectName) ) {
		PERROR("project %s doesn't exist\n", projectName.toAscii().data());
		return -1;
	}

	Project *newProject = new Project(projectName);
	
	if (!newProject)
		return -1;

	set_current_project(newProject);

	if (currentProject->load() < 0) {
		set_current_project( (Project*) 0 );
		PERROR("couldn't load project %s", projectName.toAscii().data());
		return -1;
	}

	return 0;
}

int ProjectManager::remove_project( QString name )
{
	// check if we are removing the currentProject, and delete it before removing its files
	if (project_is_current(name)) {
		PMESG("removing current project\n");
		set_current_project( 0 );
	}
	
	return FileHelper::remove_recursively( name );
}

bool ProjectManager::project_is_current(QString title)
{
	QSettings settings;
	QString path = settings.value("Project/directory").toString();
	path += title;

	if (currentProject && (currentProject->get_root_dir() == path)) {
		return true;
	}
	
	return false;
}

bool ProjectManager::project_exists(QString title)
{
	QSettings settings;
	QString project_dir = settings.value("Project/directory").toString();
	QString project_path = project_dir + title;
	QFileInfo fileInfo(project_path);
	
	if (fileInfo.exists()) {
		return true;
	}
	
	return false;
}

Command* ProjectManager::save_project()
{
	if (currentProject) {
		currentProject->save();
	} else {
		info().information( tr("Open or create a project first!"));
	}
	
	return (Command*) 0;
}

Project * ProjectManager::get_project( )
{
	return currentProject;
}

void ProjectManager::start( )
{
	QSettings settings;
	int loadProjectAtStartUp = settings.value("Project/loadLastUsed").toInt();
	
	if (loadProjectAtStartUp != 0) {
		QString projectToLoad = settings.value("Project/current").toString();
		
		if ( projectToLoad.isNull() || projectToLoad.isEmpty() )
			projectToLoad="Untitled";
		
		if (project_exists(projectToLoad)) {
			if ( load_project(projectToLoad) < 0 ) {
				PWARN("Cannot load project %s. Continuing anyway...", projectToLoad.toAscii().data());
				info().warning( tr("Could not load project %1").arg(projectToLoad) );
			}
		} else {
			if (create_new_project("Untitled", 1) < 0) {
				PWARN("Cannot create project Untitled. Continuing anyway...");
			} else {
				load_project("Untitled");
			}
			
		}
	}
}

Command* ProjectManager::exit()
{
	set_current_project( (Project*) 0 );

	ie().free_memory();
	
	// FIXME This really sucks!
	// Give the audiodevice some time to handle the disconnections and 
	// deletion of the Songs. Afterwards, we force the events to be processed
	// which will do the actuall deletion of the Songs. After this point, it's fine to
	// shutdown the audiodevice and exit the application.
	usleep(200000);
	QCoreApplication::processEvents();
	
	audiodevice().shutdown();
	
	QApplication::exit();
	
	return (Command*) 0;
}

//eof
