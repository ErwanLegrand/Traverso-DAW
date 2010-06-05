/*
Copyright (C) 2007 Remon Sijrier 

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

#ifndef NEW_TRACK_DIALOG_H
#define NEW_TRACK_DIALOG_H

#include "ui_NewTrackDialog.h"

#include <QDialog>

class Project;
class QAbstractButton;

class NewTrackDialog : public QDialog, protected Ui::NewTrackDialog
{
	Q_OBJECT

public:
	NewTrackDialog(QWidget* parent = 0);
        ~NewTrackDialog() {}

protected:
        void showEvent ( QShowEvent * event );


private:
	Project* m_project;

        void create_track();
        void update_driver_info();


private slots:
        void clicked (QAbstractButton * button );
	void set_project(Project* project);
        void update_buses_comboboxes();

};

#endif
