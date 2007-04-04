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

*/

#include "ProjectManager.h"

#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

#include "Project.h"
#include "Song.h"
#include "ContextPointer.h"
#include "ResourcesManager.h"
#include "Information.h"
#include "Config.h"
#include "FileHelpers.h"
#include <AudioDevice.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/**	\class ProjectManager
	\brief ProjectManager is a singleton used for loading, creating and deleting Projects.
 
 
 */

QUndoGroup ProjectManager::undogroup;

ProjectManager::ProjectManager()
	: ContextItem()
{
	PENTERCONS;
	currentProject = (Project*) 0;
	m_exitInProgress = false;

	cpointer().add_contextitem(this);
}

/**
 * 	Used to get a reference to the ProjectManager
 * @return A reference to the ProjectManager singleton 
 */
ProjectManager& pm()
{
	static ProjectManager projMan;
	return projMan;
}

/**
 * 	The Resources Manager for the currently loaded Project

 * @return A pointer to the Resources Manager of the loaded Project, 0 if no Project is loaded
 */
ResourcesManager* resources_manager()
{
	Project* proj = pm().get_project();
	if (proj) {
		return proj->get_audiosource_manager();
	}
	return 0;
}

void ProjectManager::set_current_project(Project* project)
{
	PENTER;

	emit projectLoaded(project);
	
	if (currentProject) {
		currentProject->save();
		delete currentProject;
	}

	currentProject = project;


	QString title = "";

	if (currentProject) {
		title = currentProject->get_title();
		config().set_property("Project", "current", title);
	}

}

Project* ProjectManager::create_new_project(int numSongs, int numTracks, const QString& projectName)
{
	PENTER;

	if (project_exists(projectName)) {
		info().critical(tr("Project %1 already exists!").arg(projectName));
		return 0;
	}

	Project* newProject = new Project(projectName);

	if (newProject->create(numSongs, numTracks) < 0) {
		delete newProject;
		info().critical(tr("Unable to create new Project %1").arg(projectName));
		return 0;
	}
	
	return newProject;
}

Project* ProjectManager::create_new_project(const QString& templatefile, const QString& projectName)
{
	if (project_exists(projectName)) {
		info().critical(tr("Project %1 already exists!").arg(projectName));
		return 0;
	}

	
	Project* newProject = new Project(projectName);
	
	if (newProject->create(0, 0) < 0) {
		delete newProject;
		info().critical(tr("Unable to create new Project %1").arg(projectName));
		return 0;
	}
	
	if (newProject->load(templatefile) < 0) {
		return 0;
	}
	
	// title gets overwritten in newProject->load()
	newProject->set_title(projectName);
	
	return newProject;
}

int ProjectManager::load_project(const QString& projectName)
{
	PENTER;

	if( ! project_exists(projectName) ) {
		PERROR("project %s doesn't exist!", projectName.toAscii().data());
		return -1;
	}

	Project* newProject = new Project(projectName);

	if (!newProject)
		return -1;

	set_current_project(newProject);

	if (currentProject->load() < 0) {
		delete currentProject;
		currentProject = 0;
		set_current_project(0);
		info().critical(tr("Unable to load Project %1").arg(projectName));
		return -1;
	}

	return 1;
}

int ProjectManager::remove_project( const QString& name )
{
	// check if we are removing the currentProject, and delete it before removing its files
	if (project_is_current(name)) {
		PMESG("removing current project\n");
		set_current_project(0);
	}

	return FileHelper::remove_recursively( name );
}

bool ProjectManager::project_is_current(const QString& title)
{
	QString path = config().get_property("Project", "directory", "/directory/unknown").toString();
	path += title;

	if (currentProject && (currentProject->get_root_dir() == path)) {
		return true;
	}

	return false;
}

bool ProjectManager::project_exists(const QString& title)
{
	QString project_dir = config().get_property("Project", "directory", "/directory/unknown").toString();
	QString project_path = project_dir + "/" + title;
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
		info().information( tr("No Project to save, open or create a Project first!"));
	}

	return (Command*) 0;
}

Project * ProjectManager::get_project( )
{
	return currentProject;
}

void ProjectManager::start( )
{
	QString defaultpath = config().get_property("Project", "DefaultDirectory", "").toString();
	QString projects_path = config().get_property("Project", "directory", defaultpath).toString();

	QDir dir;
	if ( (projects_path.isEmpty()) || (!dir.exists(projects_path)) ) {
		if (projects_path.isEmpty())
			projects_path = QDir::homePath();

		QString newPath = QFileDialog::getExistingDirectory(0,
				tr("Choose an existing or create a new Project Directory"),
				   projects_path );
		if (dir.exists(newPath)) {
			QMessageBox::information( 0, 
					tr("Traverso - Information"), 
					tr("Using existing Project directory: %1\n").arg(newPath), 
					"OK", 0 );
		} else if (!dir.mkpath(newPath)) {
			QMessageBox::warning( 0, tr("Traverso - Warning"), 
					tr("Unable to create Project directory! \n") +
					tr("Please check permission for this directory: %1").arg(newPath) );
			return;
		} else {
			QMessageBox::information( 0, 
					tr("Traverso - Information"), 
					tr("Created new Project directory for you here: %1\n").arg(newPath), 
					"OK", 0 );
		}
		config().set_property("Project", "directory", newPath);
	}
	
	bool loadProjectAtStartUp = config().get_property("Project", "loadLastUsed", 1).toBool();

	if (loadProjectAtStartUp) {
		QString projectToLoad = config().get_property("Project", "current", "").toString();

		if ( projectToLoad.isNull() || projectToLoad.isEmpty() )
			projectToLoad="Untitled";

		if (project_exists(projectToLoad)) {
			load_project(projectToLoad);
		} else {
			Project* project;
			if ( (project = create_new_project(1, 4, "Untitled")) ) {
				project->set_description(tr("Default Project created by Traverso"));
				project->save();
				delete project;
				load_project("Untitled");
			} else {
				PWARN("Cannot create project Untitled. Continuing anyway...");
			}

		}
	}
}


QUndoGroup* ProjectManager::get_undogroup() const
{
	return &undogroup;
}


Command* ProjectManager::exit()
{
	PENTER;
	m_exitInProgress = true;
	
	if (currentProject) {
		set_current_project(0);
	} else {
		QApplication::exit();
	}


	return (Command*) 0;
}

void ProjectManager::scheduled_for_deletion( Song * song )
{
	PENTER;
	m_deletionSongList.append(song);
}

void ProjectManager::delete_song( Song * song )
{
	PENTER;
	m_deletionSongList.removeAll(song);
	emit aboutToDelete(song);
	delete song;
	
	if (m_deletionSongList.isEmpty() && m_exitInProgress) {
		QApplication::exit();
	}
		
}

Command* ProjectManager::undo()
{
	undogroup.undo();
	return 0;
}

Command* ProjectManager::redo()
{
	undogroup.redo();
	return 0;
}


//eof
