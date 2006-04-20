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
 
    $Id: ProjectManager.h,v 1.1 2006/04/20 14:51:40 r_sijrier Exp $
*/

#ifndef ProjectManager_H
#define ProjectManager_H

#include <libtraverso.h>
#include <libtraversocore.h>

#include "ContextItem.h"


class Project;


class ProjectManager : public ContextItem
{
        Q_OBJECT
public:
        int create_new_project(QString projectName, int numSong);
        int load_project(QString projectName);

        bool project_is_current(QString title);
        bool projectExists(QString title);

        int delete_source(QString sourceName);
        int save_song(QString songName);
        int save_song_as(QString songName, QString title, QString artists);

        Project* get_project();

        void set_current_project(Project* pProject);
        int remove_recursively(QString pName);
        int copy_recursively(QString pNameFrom, QString pNameTo);


public slots:
        void update();
        void start();
        Command* toggle_snap();
        Command* save_project();
        Command* render_song();
        Command* exit();


private:
        ProjectManager();
        ProjectManager(const ProjectManager&);


        Project* currentProject;

        // allow this function to create one instance
        friend ProjectManager& pm();

signals:
        void projectLoaded(Project* );
        void currentProjectChanged(Project* );
};


// use this function to access the settings
ProjectManager& pm();

#endif
