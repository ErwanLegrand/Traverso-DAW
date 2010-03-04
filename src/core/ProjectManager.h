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

#ifndef ProjectManager_H
#define ProjectManager_H

#include "ContextItem.h"
#include <QUndoGroup>
#include <QList>
#include <QTimer>
#include <QStringList>


class Project;
class Sheet;
class Command;
class ResourcesManager;
class QFileSystemWatcher;


class ProjectManager : public ContextItem
{
	Q_OBJECT
	Q_CLASSINFO("save_project", tr("Save Project"))
	Q_CLASSINFO("exit", tr("Exit application"))
	
public:
	Project* create_new_project(int numSheet, int numTracks, const QString& projectName);
	Project* create_new_project(const QString& templatefile, const QString& projectName);
	
	int load_project(const QString& projectName);
	int load_renamed_project(const QString& name);

	bool project_exists(const QString& title);

	int create_projectfilebackup_dir(const QString& rootDir);
	int remove_project(const QString& title);
	
	void scheduled_for_deletion(Sheet* sheet);
	void delete_sheet(Sheet* sheet);
	void set_current_project_dir(const QString& path);
	void add_valid_project_path(const QString& path);
	void remove_wrong_project_path(const QString& path);
	
	int rename_project_dir(const QString& olddir, const QString& newdir);
	int restore_project_from_backup(const QString& projectdir, uint restoretime);

	QList<uint> get_backup_date_times(const QString& projectdir);
        QStringList get_projects_list();
        QString get_projects_directory();
	void start_incremental_backup(const QString& projectname);

	Project* get_project();
	QUndoGroup* get_undogroup() const;

	void start(const QString& basepath, const QString& projectname);

public slots:
	Command* save_project();
        Command* close_current_project();
        Command* exit();
	Command* undo();
	Command* redo();


private:
	ProjectManager();
	ProjectManager(const ProjectManager&);

	Project* currentProject;
	QList<Sheet*>	m_deletionSheetList;
	bool		m_exitInProgress;
	QStringList	m_projectDirs;
	QFileSystemWatcher*	m_watcher;

	bool clientRequestInProgress;
	static QUndoGroup	undogroup;
	
	void set_current_project(Project* project);
	void cleanup_backupfiles_for_project(const QString& projectname);
	bool project_is_current(const QString& title);
	
	// allow this function to create one instance
	friend ProjectManager& pm();

signals:
	void projectLoaded(Project* );
	void currentProjectDirChanged();
        void projectsListChanged();
	void unsupportedProjectDirChangeDetected();
	void projectDirChangeDetected();
	void projectLoadFailed(QString,QString);
	void projectFileVersionMismatch(QString,QString);
	
private slots:
	void project_dir_rename_detected(const QString& dirname);
};


// use this function to access the ProjectManager
ProjectManager& pm();

// Use this function to get the loaded Project's Resources Manager
ResourcesManager* resources_manager();

#endif
