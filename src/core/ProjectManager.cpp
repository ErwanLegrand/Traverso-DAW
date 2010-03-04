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
#include <QFileSystemWatcher>
#include <QTextStream>


#include "Project.h"
#include "Sheet.h"
#include "ContextPointer.h"
#include "ResourcesManager.h"
#include "Information.h"
#include "Config.h"
#include "FileHelpers.h"
#include <AudioDevice.h>
#include <Utils.h>

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
	
	m_watcher = new QFileSystemWatcher(0);

	QString path = config().get_property("Project", "directory", "").toString();
	set_current_project_dir(path);
	
	cpointer().add_contextitem(this);
	
	connect(m_watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(project_dir_rename_detected(const QString&)));
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

	QString oldprojectname = "";
	
	if (currentProject) {
//                printf("exit in progress");
                QString oncloseaction = config().get_property("Project", "onclose", "save").toString();
                if (oncloseaction == "save") {
                        currentProject->save();
                } else if (oncloseaction == "ask") {
                        QMessageBox::StandardButton button =
                                QMessageBox::question(0,
                                tr("Save Project"),
                                tr("Should Project '%1' be safed before closing it?").arg(currentProject->get_title()),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::Yes);
                        if (button  == QMessageBox::Yes) {
                                currentProject->save();
                        }
                }
		
		oldprojectname = currentProject->get_title();
		delete currentProject;
	}

	currentProject = project;

        emit projectLoaded(currentProject);

	QString title = "";

	if (currentProject) {
		title = currentProject->get_title();
		config().set_property("Project", "current", title);
	}
	
	if ( ! oldprojectname.isEmpty() ) {
		cleanup_backupfiles_for_project(oldprojectname);
	}

}

Project* ProjectManager::create_new_project(int numSheets, int numTracks, const QString& projectName)
{
	PENTER;

	if (project_exists(projectName)) {
		info().critical(tr("Project %1 already exists!").arg(projectName));
		return 0;
	}

	QString newrootdir = config().get_property("Project", "directory", "/directory/unknown/").toString() + "/" + projectName;
	m_projectDirs.append(newrootdir);
	
	Project* newProject = new Project(projectName);

	if (newProject->create(numSheets, numTracks) < 0) {
		delete newProject;
		info().critical(tr("Unable to create new Project %1").arg(projectName));
		return 0;
	}

        emit projectsListChanged();
	
	return newProject;
}

Project* ProjectManager::create_new_project(const QString& templatefile, const QString& projectName)
{
	if (project_exists(projectName)) {
		info().critical(tr("Project %1 already exists!").arg(projectName));
		return 0;
	}

	QString newrootdir = config().get_property("Project", "directory", "/directory/unknown/").toString() + "/" + projectName;
	m_projectDirs.append(newrootdir);
	
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

        emit projectsListChanged();
	
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

	int err;
	if ((err = currentProject->load()) < 0) {
		switch (err) {
			case Project::PROJECT_FILE_VERSION_MISMATCH: {
				emit projectFileVersionMismatch(currentProject->get_root_dir(), currentProject->get_title());
				break;
			}
			default: {
				emit projectLoadFailed(currentProject->get_title(), currentProject->get_error_string());
			}
		}
		delete currentProject;
		currentProject = 0;
		set_current_project(0);
		info().critical(tr("Unable to load Project %1").arg(projectName));
		return -1;
	}
	
	return 1;
}

int ProjectManager::load_renamed_project(const QString & name)
{
	Q_ASSERT(currentProject);
	
	delete currentProject;
	currentProject= 0;
	
	return load_project(name);
}


int ProjectManager::remove_project( const QString& name )
{
	// check if we are removing the currentProject, and delete it before removing its files
	if (project_is_current(name)) {
		PMESG("removing current project\n");
		set_current_project(0);
	}
	
	QString oldrootdir = config().get_property("Project", "directory", "/directory/unknown/").toString() + "/" + name;
	m_projectDirs.removeAll(oldrootdir);

        int r = FileHelper::remove_recursively( name );
        if (r == 1) {
                emit projectsListChanged();

        }

        return r;
}

bool ProjectManager::project_is_current(const QString& title)
{
	QString path = config().get_property("Project", "directory", "/directory/unknown").toString();
	path += "/" + title;

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


void ProjectManager::start(const QString & basepath, const QString & projectname)
{
	config().set_property("Project", "directory", basepath);
	
	if (project_exists(projectname)) {
		load_project(projectname);
	}
}

QUndoGroup* ProjectManager::get_undogroup() const
{
	return &undogroup;
}


Command* ProjectManager::exit()
{
	PENTER;
	
	if (currentProject) {
		if (currentProject->get_sheets().size() == 0) {
			// No sheets to unregister from the audiodevice,
			// just save and quit:
			set_current_project(0);
			QApplication::exit();
			return 0;
		} else if (currentProject->is_save_to_close()) {
			m_exitInProgress = true;
			set_current_project(0);
		} else {
			return 0;
		}
	} else {
		QApplication::exit();
	}


	return (Command*) 0;
}

void ProjectManager::scheduled_for_deletion( Sheet * sheet )
{
	PENTER;
	m_deletionSheetList.append(sheet);
}

void ProjectManager::delete_sheet( Sheet * sheet )
{
	PENTER;
	m_deletionSheetList.removeAll(sheet);
	delete sheet;
	
	if (m_deletionSheetList.isEmpty() && m_exitInProgress) {
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


int ProjectManager::rename_project_dir(const QString & olddir, const QString & newdir)
{
	QDir dir(olddir);
	
	m_projectDirs.removeAll(olddir);
	m_projectDirs.append(newdir);
	
	if ( ! dir.rename(olddir, newdir)) {
		info().critical(tr("Could not rename Project directory to %1").arg(newdir));
		return - 1;
	}
	
	return 1;
}

void ProjectManager::set_current_project_dir(const QString & path)
{
	if (path.isEmpty()) {
		return;
	}
	
	QDir newdir(path);
	
	config().set_property("Project", "directory", newdir.canonicalPath());
	
	QStringList list = newdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	m_projectDirs.clear();
	
	foreach(const QString &string, list) {
		m_projectDirs += path + "/" + string;
	}
	
	m_watcher->addPath(path);
	
	emit projectDirChangeDetected();
}

void ProjectManager::project_dir_rename_detected(const QString & dirname)
{
	Q_UNUSED(dirname);
	
	emit projectDirChangeDetected();
	
	QString path = config().get_property("Project", "directory", "").toString();
	QDir dir(path);
	
	QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	
	bool startwhining = false;
	foreach(const QString &string, list) {
		if (!m_projectDirs.contains(path + "/" + string)) {
			startwhining = true;
			break;
		}
	}

	
	if (!startwhining) {
		return;
	}
	
	emit unsupportedProjectDirChangeDetected();
}

void ProjectManager::add_valid_project_path(const QString & path)
{
	m_projectDirs.append(path);
}

void ProjectManager::remove_wrong_project_path(const QString & path)
{
	m_projectDirs.removeAll(path);
}


void ProjectManager::start_incremental_backup(const QString& projectname)
{
	if (! project_exists(projectname)) {
		return;
	}
	
	QString project_dir = config().get_property("Project", "directory", "/directory/unknown").toString();
	QString project_path = project_dir + "/" + projectname;
	QString fileName = project_path + "/project.tpf";
	QString backupdir = project_path + "/projectfilebackup";
	
	// Check if the projectfilebackup directory still exist
	QDir dir(backupdir);
	if (!dir.exists(backupdir)) {
		create_projectfilebackup_dir(project_path);
	}
	
	QFile reader(fileName);
	if (!reader.open(QIODevice::ReadOnly)) {
		info().warning(tr("Projectfile backup: The project file %1 could not be opened for reading (Reason: %2)").arg(fileName).arg(reader.errorString()));
		return;
	}
	
	QDateTime time = QDateTime::currentDateTime();
	QString writelocation = backupdir + "/" + time.toString() + "__" + QString::number(time.toTime_t());
	QFile compressedWriter(writelocation);
	
	if (!compressedWriter.open( QIODevice::WriteOnly ) ) {
		compressedWriter.close();
		return;
	}
	
	
	QByteArray array = reader.readAll();
	QByteArray compressed = qCompress(array, 9);
	QDataStream stream(&compressedWriter);
	stream << compressed;
	
	compressedWriter.close();
}


void ProjectManager::cleanup_backupfiles_for_project(const QString & projectname)
{
	if (! project_exists(projectname)) {
		return;
	}
	
	QString project_dir = config().get_property("Project", "directory", "/directory/unknown").toString();
	QString project_path = project_dir + "/" + projectname;
	QString backupdir = project_path + "/projectfilebackup";
	
	// Check if the projectfilebackup directory still exist
	QDir dir(backupdir);
	// A map to insert files based on their time value,
	// so it's sorted on date automatically
	QMap<int, QString> map;
	QStringList entrylist = dir.entryList(QDir::Files);
	
	// If there are more then 1000 saves, remove the last 200!
	if (entrylist.size() > 1000) {
		printf("more then thousand backup files, deleting oldest 200\n");
		
		int key;
		foreach (QString file, dir.entryList(QDir::Files)) {
			key = file.right(10).toUInt();
			map.insert(key, file);
		}
		
		QList<QString> tobedeleted = map.values();
		
		if (tobedeleted.size() < 201) {
			return;
		}

		for(int i=0; i<200; ++i) {
			QFile file(backupdir + "/" + tobedeleted.at(i));
			if ( ! file.remove() ) {
				printf("Could not remove file %s (Reason: %s)\n", QS_C(tobedeleted.at(i)), QS_C(FileHelper::fileerror_to_string(file.error())));
			}
		}
	}
}


int ProjectManager::restore_project_from_backup(const QString& projectname, uint restoretime)
{
	if (! project_exists(projectname)) {
		return -1;
	}
	QString project_dir = config().get_property("Project", "directory", "/directory/unknown").toString();
	QString project_path = project_dir + "/" + projectname;
	QString backupDir = project_path + "/projectfilebackup";
	
	if (currentProject) {
		currentProject->save();
		set_current_project(0);
		delete currentProject;
		currentProject = 0;
	}

	QString fileName = project_path + "/project.tpf";
	
	QDir dir(backupDir);
	QString backupfile;
	
	foreach (QString backup, dir.entryList(QDir::Files)) {
		if (backup.right(10).toUInt() == restoretime) {
			backupfile = backupDir + "/" + backup;
			printf("backupfile %s\n", QS_C(backupfile));
			break;
		}
	}
	
	QFile reader(backupfile);
	if (!reader.open(QIODevice::ReadOnly)) {
		//
		reader.close();
		return -1;
	}
	
	
	QFile writer(fileName);
	if (!writer.open( QIODevice::WriteOnly | QIODevice::Text) ) {
		PERROR("Could not open %s for writing!", QS_C(fileName));
		writer.close();
		return -1;
	}
	
	QDataStream dataIn(&reader);
	QByteArray compByteArray;
	dataIn >> compByteArray;
	
	QByteArray a = qUncompress(compByteArray);
	QTextStream stream(&writer);
	stream << a;
	
	writer.close();
	
	return 1;
}

QList< uint > ProjectManager::get_backup_date_times(const QString& projectname)
{
	if (! project_exists(projectname)) {
		return QList<uint>();
	}
	QString project_dir = config().get_property("Project", "directory", "/directory/unknown").toString();
	QString backupDir = project_dir + "/" + projectname + "/projectfilebackup";
	
	QList<uint> dateList;
	QDir dir(backupDir);
	
	foreach (QString filename, dir.entryList(QDir::Files)) {
		bool ok;
		uint date = filename.right(10).toUInt(&ok);
		if (ok) {
			dateList.append(date);
		} else {
			printf("filename: %s is not backupfile made by Traverso, removing it!\n", QS_C(filename));
			QFile::remove(backupDir + "/" + filename);
		}
	}

	return dateList;
}

int ProjectManager::create_projectfilebackup_dir(const QString& rootDir)
{
	QDir dir;
	QString path = rootDir + "/projectfilebackup/";

	if (dir.mkdir(path) < 0) {
		info().critical(tr("Cannot create dir %1").arg(path));
		return -1;
	}
	
	return 1;
}

Command* ProjectManager::close_current_project()
{
        set_current_project(0);
        return 0;
}

QStringList ProjectManager::get_projects_list()
{
        QString path = config().get_property("Project", "directory", "none").toString();
        QDir dir(path);
        QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        QStringList projects;

        foreach(const QString &dirname, list) {
                QString fileToOpen = path + "/" + dirname + "/project.tpf";

                QFile file(fileToOpen);

                if (!file.open(QIODevice::ReadOnly)) {
                        PWARN("ProjectManager:: Cannot open project properties file (%s)", fileToOpen.toUtf8().data());
                        continue;
                }

                file.close();

                projects.append(dirname);
        }

        return projects;
}


QString ProjectManager::get_projects_directory()
{
        QString path = config().get_property("Project", "directory", "").toString();

        printf("path is %s\n", path.toAscii().data());
        if (path.isEmpty() || path.isNull()) {
                path = QDir::homePath();
        }

        return path;
}

