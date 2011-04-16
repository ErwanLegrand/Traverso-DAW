/*
Copyright (C) 2011 Remon Sijrier

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

#include "TGlobalContext.h"

#include "ProjectManager.h"
#include "Project.h"
#include "Sheet.h"

TGlobalContext::TGlobalContext(QObject *parent) :
    QObject(parent)
{
	m_session = 0;
	m_project = 0;

	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
}

void TGlobalContext::set_project(Project *project)
{
	m_project = project;
	if (m_project) {
		connect(m_project, SIGNAL(currentSessionChanged(TSession*)), this, SLOT(set_session(TSession*)));
	} else {
		set_session(0);
	}
}

void TGlobalContext::set_session(TSession *session)
{
	m_session = session;
}
