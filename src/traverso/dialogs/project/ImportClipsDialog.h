/*
    Copyright (C) 2008 Nicola Doebelin
 
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

#ifndef IMPORT_CLIPS_DIALOG_H
#define IMPORT_CLIPS_DIALOG_H

#include "ui_ImportClipsDialog.h"
#include "AudioTrack.h"

#include <QDialog>
#include <QList>

class ImportClipsDialog : public QDialog, protected Ui::ImportClipsDialog
{
	Q_OBJECT

public:
	ImportClipsDialog(QWidget* parent = 0);
	~ImportClipsDialog();

        void set_tracks(QList<AudioTrack*>);

        AudioTrack* get_selected_track();
	bool get_add_markers();
	int has_tracks();

private:
        QList<AudioTrack*> m_tracks;

};

#endif

//eof

