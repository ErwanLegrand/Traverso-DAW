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

$Id: ProjectManager.cpp,v 1.1 2006/04/20 14:51:40 r_sijrier Exp $
*/

#include "ProjectManager.h"

#include <sys/stat.h>

#include <QSettings>
#include <QDir>
#include <QApplication>

#include "Project.h"
#include "Song.h"
#include "AudioSourcesList.h"
#include "Information.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


ProjectManager::ProjectManager()
		: ContextItem()
{
	PENTERCONS;
	currentProject = (Project*) 0;

	cpointer().add_contextitem(this);
};

ProjectManager& pm()
{
	static ProjectManager projMan;
	return projMan;
}


void ProjectManager::update()
{
	PENTER;
}


// delete file/dir pName after prepending $HOME/traversoprojects/ to it
//
// if it is a directory, calls itself recursively  on any file/dir in the directory
// before removing the directory
int ProjectManager::remove_recursively(QString pName)
{
	QSettings settings;

	QString name = settings.value("Project/directory").toString();
	name += pName;

	// check if we are removing the currentProject, and delete it before removing its files
	if (project_is_current(pName)) {
		PMESG("removing current project\n");
		delete currentProject;
		currentProject = NULL;
	}

	QFileInfo fileInfo(name);

	if (!fileInfo.exists())
		return -1;

	if (!fileInfo.isWritable()) {
		PERROR("failed to remove %s: you don't have write access to it\n", name.toAscii().data());
		return -1;
	}

	if(fileInfo.isFile()) {
		QFile file(name);
		if (!file.remove()) {
			PERROR("failed to remove file %s\n", name.toAscii().data());
			return -1;
		}
		return 1;
	} else if(fileInfo.isDir()) {
		QDir dir(name);
		PWARN("name is: %s", name.toAscii().data());
		QFileInfoList list = dir.entryInfoList();
		QFileInfo fi;
		for (int i = 0; i < list.size(); ++i) {
			fi = list.at(i);
			if ((fi.fileName() != ".") && (fi.fileName() != "..")) {
				QString nextFileName = pName + "/" + fi.fileName();
				if (remove_recursively(nextFileName) < 0) {
					PERROR("failed to remove directory %s\n", nextFileName.toAscii().data());
					return -1;
				}
			}
		}
		if (!dir.rmdir(name)) {
			PERROR("failed to remove directory %s\n", name.toAscii().data());
			return -1;
		}

		return 1;
	}

	return 1;
}


int ProjectManager::copy_recursively(QString pNameFrom, QString pNameTo)
{
	QSettings settings;
	QString nameFrom = settings.value("Project/directory").toString();
	QString nameTo(nameFrom);

	nameFrom += pNameFrom;
	nameTo += pNameTo;

	QFileInfo fileFromInfo(nameFrom);
	QFileInfo fileToInfo(nameTo);

	if (!fileFromInfo.exists()) {
		PERROR("File or directory %s doesn't exist\n", pNameFrom.toAscii().data());
		return -1;
	}
	if (fileToInfo.exists()) {
		PERROR("File or directory %s already exists", pNameTo.toAscii().data());
		return -1;
	}

	if(fileFromInfo.isFile()) {
		QFile fileFrom(nameFrom);
		if (!fileFrom.open(QIODevice::ReadOnly)) {
			PERROR("failed to open file %s for reading\n", nameFrom.toAscii().data());
			return -1;
		}

		QFile fileTo(nameTo);
		if (!fileTo.open(QIODevice::WriteOnly)) {
			fileFrom.close();
			PERROR("failed to open file for writting%s\n", nameFrom.toAscii().data());
			return -1;
		}

		// the real copy part should perhaps be implemented using QDataStream
		// but .handle() will still be needed to get the optimal block-size
		//
		//! \todo does not keep file mode yet
		int bufferSize = 4096;
		int fileDescFrom = fileFrom.handle();
		int fileDescTo = fileTo.handle();
		struct stat fileStat;
		if (fstat(fileDescFrom, &fileStat) == 0)
			bufferSize = (int)fileStat.st_blksize;

		void *buffer = malloc(sizeof(char) * bufferSize);
		// QMemArray<char> buffer(bufferSize);

		for (;;) {
			int nRead = read(fileDescFrom, buffer, bufferSize);
			if (nRead < 0) {
				fileFrom.close();
				fileTo.close();
				PERROR("Error while reading file %s\n", nameFrom.toAscii().data());
				return -1;
			}
			if (nRead == 0)
				break;
			if (write(fileDescTo, buffer, nRead) < 0) {
				fileFrom.close();
				fileTo.close();
				PERROR("Error while writing file %s\n", nameTo.toAscii().data());
				return -1;
			}
		}
		free(buffer);

		fileFrom.close();
		fileTo.close();

		return 0;
	} else if(fileFromInfo.isDir()) {
		QDir dirFrom(nameFrom);
		QDir dirTo(nameTo);
		if (!dirTo.mkdir(nameTo)) {
			PERROR("failed to create directory %s\n", nameTo.toAscii().data());
			return -1;
		}

		QFileInfoList list = dirFrom.entryInfoList();
		QFileInfo fi;
		QString fileName;
		for (int i = 0; i < list.size(); ++i) {
			fileName = fi.fileName();
			if ((fileName != ".") && (fileName != "..")) {
				copy_recursively(pNameFrom + "/" + fileName, pNameTo + "/" + fileName);
			}
		}
		return 0;
	}
	return -1;
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


bool ProjectManager::project_is_current(QString title)
{
	QSettings settings;
	QString path = settings.value("Project/directory").toString();
	path += title;

	if (currentProject && (currentProject->get_root_dir() == path))
		return true;
	return false;
}


bool ProjectManager::projectExists(QString title)
{
	QSettings settings;
	QString project_dir = settings.value("Project/directory").toString();
	QString project_path = project_dir + title;
	QFileInfo fileInfo(project_path);
	if (fileInfo.exists())
		return true;
	return false;
}


Command* ProjectManager::save_project()
{
	if (currentProject)
		currentProject->save();
	else
		info().information( tr("Open or create a project first!"));
	return (Command*) 0;
}


Command* ProjectManager::render_song()
{
	/*	if (currentProject)
			currentProject->get_current_song()->render();
		else 
			info().information("Open or create a Project first!!");*/
	return (Command*) 0;
}


Command* ProjectManager::toggle_snap()
{
	info().information("Toggle snap on/off");
	if (currentProject) {
		Song* so = currentProject->get_current_song();
		if (so)
			so->toggle_snap();
	}
	return (Command*) 0;
}


Project * ProjectManager::get_project( )
{
	if (currentProject)
		return currentProject;
	return (Project*) 0;
}


void ProjectManager::start( )
{
	QSettings settings;
	int loadProjectAtStartUp = settings.value("Project/loadLastUsed").toInt();
	if (loadProjectAtStartUp == 0) {}
	else {
		QString sCurrentProject = settings.value("Project/current").toString();
		if ((sCurrentProject.isNull()) || (sCurrentProject.isEmpty()))
			sCurrentProject="Untitled";
		if (projectExists(sCurrentProject)) {
			if (load_project(sCurrentProject)<0) {
				PWARN("Cannot load project %s. Continuing anyway...", sCurrentProject.toAscii().data());
				info().warning(tr("Could not load project \"").append(sCurrentProject).append("\""));
			}
		} else {
			if (create_new_project("Untitled", 1)<0) {
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
		currentProject = 0;
	}


	ie().free_memory();

	if (audiodevice().shutdown() < 0) {
	}
	
	QApplication::exit();
	
	return (Command*) 0;
}

//eof
