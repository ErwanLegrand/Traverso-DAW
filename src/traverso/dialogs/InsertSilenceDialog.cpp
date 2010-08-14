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
#include "Sheet.h"
#include "Import.h"
#include "AudioTrack.h"
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


void InsertSilenceDialog::setTrack(AudioTrack* track)
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
        Sheet* sheet = qobject_cast<Sheet*>(pm().get_project()->get_current_session());
        if (!sheet) {
                return;
        }

        QList<AudioTrack*> tracks = sheet->get_audio_tracks();

	// Make sure track is still in the sheet
	if (m_track){
		AudioTrack*	foundTrack = 0;

		foreach(AudioTrack* track, tracks) {
			if (track == m_track) {
				foundTrack = track;
			}
		}
		m_track = foundTrack;
	}

	if (sheet->get_numtracks() > 0) {
		if (!m_track){
			AudioTrack*	shortestTrack = (AudioTrack*)tracks.first();
	
			foreach(AudioTrack* track, tracks) {
				if (!track->get_cliplist().isEmpty() && (track->get_cliplist().last())->get_track_end_location() > (shortestTrack->get_cliplist().last())->get_track_end_location()) {
					shortestTrack = track;
				}
			}
			m_track = shortestTrack;
		}

		TimeRef length = TimeRef(lengthSpinBox->value() * UNIVERSAL_SAMPLE_RATE);
		Import* cmd = new Import(m_track, length, true);
		TCommand::process_command(cmd);
	}

	hide();
}

void InsertSilenceDialog::reject()
{
	hide();
}


//eof

