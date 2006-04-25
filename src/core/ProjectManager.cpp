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

$Id: ProjectManager.cpp,v 1.2 2006/04/25 16:48:32 r_sijrier Exp $
*/

#include "ProjectManager.h"

#include <QSettings>
#include <QApplication>
#include <QFileInfo>

#include "Project.h"
#include "Song.h"
#include "AudioSourcesList.h"
#include "Information.h"
#include "FileHelpers.h"

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
	Song* s = (Song*) 0;
	if (songName == "") {
		s = currentProject->get_current_song();
		songName.setNum(s->get_id());
		songName.prepend("Song ");
	} else {
		//assuming that song name is always: "Song nr title" so nr is always on position 5-7
		QString t = songName.mid(5,2);
		bool b;
		int nr = t.toInt(&b, 10);
		s = currentProject->get_song( (nr - 1) );
	}

	// Hmmm, this no longer works :-(
	// Use currentProject->save instead
	/*	if(s && (s->save()))
			return 1;*/
	currentProject->save();

	return 0;
}

int ProjectManager::save_song_as(QString songName, QString title, QString artists)
{
	Song* s;
	QString t = songName.mid(5,2);
	bool b;
	//assuming that song name is always: "Song nr title" so nr is always on position 5-7
	int nr = t.toInt(&b, 10);
	s = currentProject->get_song( (nr - 1) );
	s = currentProject->get_song( (nr - 1) );
	if (title.length() != 0) {
		s->set_title(title);
	}
	if (artists.length() != 0) {
		s->set_artists(artists);
	}
	// Hmmm, this no longer works :-(
	// Use currentProject->save instead
	/*	if(s->save())
			return 1;*/
	currentProject->save();

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

	emit currentProjectChanged(currentProject);

	QSettings settings;
	settings.setValue("Project/current", currentProject->get_title());

}

int ProjectManager::create_new_project(QString projectName, int numSongs)
{
	PENTER;

	if (projectExists(projectName)) {
		PERROR("project %s already exists\n", projectName.toAscii().data());
		return -1;
	}
	Project *newProject = new Project(projectName);
	if (newProject->create(numSongs) < 0) {
		delete newProject;
		PERROR("couldn't create new project %s", projectName.toAscii().data());
		return -1;
	}

	set_current_project(newProject);
	return 0;
}

int ProjectManager::delete_source(QString )
{
	int r = 0;
	/*	QString s = sourceName;
		s.prepend(currentProject->get_root_dir());
		Song* cs = currentProject->get_current_song();
		AudioSource* a = cs->get_audiosources_list()->get_audio_for_source(s);
		cs->remove_all_clips_for_audio(a);
		cs->get_audiosources_list()->remove(a);
		delete a;
		QFile file(s);
		if (!file.remove())
			{
			PERROR("failed to remove file %s\n", s.toAscii().data());
			r = -1;
			}
		QString peakFile = s + ".peak" ;
		file.setFileName(peakFile);
		if (!file.remove())
			{
			PERROR("failed to remove file %s\n", peakFile.toAscii().data());
			r = -1;
			}*/
	return r;
}

int ProjectManager::load_project(QString projectName)
{
	PENTER;
	if(!projectExists(projectName)) {
		PERROR("project %s doesn't exist\n", projectName.toAscii().data());
		return -1;
	}

	Project *newProject = new Project(projectName);
	if (!newProject)
		return -1;

	emit projectLoaded(newProject);

	if (newProject->load() > 0) {
		set_current_project(newProject);
	} else {
		delete newProject;
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
		delete currentProject;
		currentProject = NULL;
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

bool ProjectManager::projectExists(QString title)
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
	if (currentProject) {
		return currentProject;
	}
	
	return (Project*) 0;
}

void ProjectManager::start( )
{
	QSettings settings;
	int loadProjectAtStartUp = settings.value("Project/loadLastUsed").toInt();
	
	if (loadProjectAtStartUp != 0) {
		QString sCurrentProject = settings.value("Project/current").toString();
		
		if ((sCurrentProject.isNull()) || (sCurrentProject.isEmpty()))
			sCurrentProject="Untitled";
		
		if (projectExists(sCurrentProject)) {
			if (load_project(sCurrentProject) < 0) {
				PWARN("Cannot load project %s. Continuing anyway...", sCurrentProject.toAscii().data());
				info().warning(tr("Could not load project \"").append(sCurrentProject).append("\""));
			}
		} else {
			if (create_new_project("Untitled", 1) < 0) {
				PWARN("Cannot load project Untitled. Continuing anyway...");
			}
		}
	}
}

Command* ProjectManager::exit()
{

	if (currentProject) {
		currentProject->save();
		delete currentProject;
	}

	ie().free_memory();
	
	// Give the audiodevice some time to handle the disconnections and 
	// deletion of the Songs. Afterwards, we force the events to be processed
	// which will do the actuall deletion of the Songs. After this point, it's fine to
	// shutdown the audiodevice.
	usleep(50000);
	QCoreApplication::processEvents();

	audiodevice().shutdown();
	
	QApplication::exit();
	
	return (Command*) 0;
}

//eof
