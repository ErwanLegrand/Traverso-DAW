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

#ifndef TGLOBALCONTEXT_H
#define TGLOBALCONTEXT_H

#include <QObject>

class Project;
class TSession;

class TGlobalContext : public QObject
{
    Q_OBJECT

public:
	explicit TGlobalContext(QObject *parent = 0);

protected:
	TSession*	m_session;
	Project*	m_project;

signals:

public slots:

private slots:
	void set_project(Project* project);
	void set_session(TSession* session);

};

#endif // TGLOBALCONTEXT_H
