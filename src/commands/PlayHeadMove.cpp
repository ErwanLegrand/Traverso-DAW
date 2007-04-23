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

#include "PlayHeadMove.h"

#include <libtraversocore.h>
#include <SongView.h>
#include <Cursors.h>

#include <Debugger.h>

PlayHeadMove::PlayHeadMove(PlayHead* cursor, SongView* sv)
	: Command("Play Cursor Move")
	, m_cursor(cursor)
	, m_song(sv->get_song())
	, m_sv(sv)
{
	m_resync = config().get_property("AudioClip", "SyncDuringDrag", false).toBool();
}

int PlayHeadMove::finish_hold()
{
	int x = cpointer().scene_x();
	if (x < 0) {
		x = 0;
	}
	// if the song is transporting, the seek action will cause 
	// the playcursor to be moved to the correct location.
	// Until then hide it, it will be shown again when the seek is finished!
	if (m_song->is_transporting()) {
		m_cursor->hide();
	}
	m_song->set_transport_pos( (nframes_t) (x * m_sv->scalefactor));
	m_sv->start_shuttle(false);
	return -1;
}

int PlayHeadMove::begin_hold()
{
	m_cursor->show();
	m_cursor->set_active(false);
	m_origXPos = m_song->get_transport_frame() / m_sv->scalefactor;
	m_sv->start_shuttle(true, true);
	return 1;
}

void PlayHeadMove::cancel_action()
{
	m_sv->start_shuttle(false);
	m_cursor->set_active(m_song->is_transporting());
	if (!m_resync) {
		m_cursor->setPos(m_origXPos, 0);
	}
}


void PlayHeadMove::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	cpointer().get_viewport()->set_holdcursor(":/cursorHoldLr");
}

int PlayHeadMove::jog()
{
	int x = cpointer().scene_x();
	if (x < 0) {
		x = 0;
	}
	m_cursor->setPos(x, 0);
	
	nframes_t newpos = (nframes_t) (x * m_sv->scalefactor);
	if (m_resync && m_song->is_transporting()) {
		m_song->set_transport_pos(newpos);
	}
	
	m_sv->update_shuttle_factor();
	cpointer().get_viewport()->set_holdcursor_text(frame_to_smpte(newpos, m_song->get_rate()));
	// Hmm, the alignment of the holdcursor isn't in the center, so we have to 
	// substract half the width of it to make it appear centered... :-(
	cpointer().get_viewport()->set_holdcursor_pos(QPoint(cpointer().scene_x() - 16, cpointer().scene_y() - 16));
	
	return 1;
}

