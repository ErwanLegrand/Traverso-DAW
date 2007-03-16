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

class Project;
class Song;
class Command;
class ResourcesManager;

class ProjectManager : public ContextItem
{
	Q_OBJECT
public:
	Project* create_new_project(int numSong, const QString& projectName);
	
	int load_project(const QString& projectName);

	bool project_exists(const QString& title);

	int remove_project(const QString& title);
	
	void scheduled_for_deletion(Song* song);
	void delete_song(Song* song);

	Project* get_project();
	QUndoGroup* get_undogroup() const;


public slots:
	void start();
	
	Command* save_project();
	Command* exit();
	Command* undo();
	Command* redo();


private:
	ProjectManager();
	ProjectManager(const ProjectManager&);

	Project* currentProject;
	QList<Song*>	m_deletionSongList;
	bool		m_exitInProgress;

	bool clientRequestInProgress;
	static QUndoGroup	undogroup;
	
	void set_current_project(Project* pProject);
	bool project_is_current(const QString& title);
	
	// allow this function to create one instance
	friend ProjectManager& pm();

signals:
	void projectLoaded(Project* );
	void aboutToDelete(Song* );
};


// use this function to access the ProjectManager
ProjectManager& pm();

// Use this function to get the loaded Project's Resources Manager
ResourcesManager* resources_manager();

#endif
