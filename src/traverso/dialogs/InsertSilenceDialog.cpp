/*
    Copyright (C) 2007 Ben Levitt 
 
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

#include "InsertSilenceDialog.h"
#include "ProjectManager.h"
#include "Project.h"
#include "Song.h"
#include "Import.h"
#include "Track.h"
#include "AudioClip.h"

#include <QDialogButtonBox>
#include <QPushButton>

InsertSilenceDialog::InsertSilenceDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
	m_track = 0;
	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
}


void InsertSilenceDialog::setTrack(Track* track)
{
	m_track = track;
}

void InsertSilenceDialog::focusInput()
{
	lengthSpinBox->setFocus();
	lengthSpinBox->selectAll();
}

void InsertSilenceDialog::accept()
{
	Song* song = pm().get_project()->get_current_song();
	QList<Track* > tracks = song->get_tracks();

	// Make sure track is still in the song
	if (m_track){
		Track*	foundTrack = 0;

		for (int i=0; i<tracks.size(); i++) {
			if (tracks.at(i) == m_track) {
				foundTrack = tracks.at(i);
			}
		}
		m_track = foundTrack;
	}

	if (song->get_numtracks() > 0) {
		if (!m_track){
			Track*	shortestTrack = tracks.at(0);
	
			for (int i=1; i<tracks.size(); i++) {
				if (tracks.at(i)->get_cliplist().get_last() && tracks.at(i)->get_cliplist().get_last()->get_track_end_frame() > shortestTrack->get_cliplist().get_last()->get_track_end_frame()) {
					shortestTrack = tracks.at(i);
				}
			}
			m_track = shortestTrack;
		}

		nframes_t length = (nframes_t)(lengthSpinBox->value() * pm().get_project()->get_rate());
		Import* cmd = new Import(m_track, true, length);
		Command::process_command(cmd);
	}

	hide();
}

void InsertSilenceDialog::reject()
{
	hide();
}

//eof

