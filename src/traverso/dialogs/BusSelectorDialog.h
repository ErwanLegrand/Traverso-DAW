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

#ifndef BUS_SELECTOR_DIALOG_H
#define BUS_SELECTOR_DIALOG_H

#include "ui_BusSelectorDialog.h"

#include <QDialog>

class Track;

class BusSelectorDialog : public QDialog, protected Ui::BusSelectorDialog
{
	Q_OBJECT

public:
	BusSelectorDialog(QWidget* parent = 0);
	~BusSelectorDialog() {};

        void set_current_track(Track* track);

private:
        Track* m_currentTrack;
	void accept();
	void reject();
	
private slots:
	void current_track_changed(int);
	void update_buses_list_widget();
};

#endif
