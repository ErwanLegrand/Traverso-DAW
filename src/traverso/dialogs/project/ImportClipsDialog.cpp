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

#include "ImportClipsDialog.h"
#include "ui_ImportClipsDialog.h"

#include <QComboBox>
#include <QCheckBox>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ImportClipsDialog::ImportClipsDialog( QWidget * parent )
	: QDialog(parent)
{
	setupUi(this);
}

ImportClipsDialog::~ ImportClipsDialog( )
{}

void ImportClipsDialog::set_tracks(QList<AudioTrack*> tracks)
{
	m_tracks = tracks;
	for (int i = 0; i < m_tracks.size(); ++i) {
                AudioTrack* t = m_tracks.at(i);
		comboBoxTrack->addItem(QString("%1: %2").arg(t->get_sort_index()).arg(t->get_name()));
	}
}

AudioTrack* ImportClipsDialog::get_selected_track()
{
	if (m_tracks.isEmpty()) {
                return (AudioTrack*)0;
	}

	return m_tracks.at(comboBoxTrack->currentIndex());
}

bool ImportClipsDialog::get_add_markers()
{
	return checkBoxMarkers->isChecked();
}

int ImportClipsDialog::has_tracks()
{
	return comboBoxTrack->count();
}

//eof
