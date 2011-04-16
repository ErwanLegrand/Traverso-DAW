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
#include "SheetView.h"
#include "ClipsViewPort.h"
#include "Cursors.h"

#include <Debugger.h>

PlayHeadMove::PlayHeadMove(SheetView* sv)
        : MoveCommand("Play Cursor Move")
        , m_session(sv->get_sheet())
{
        m_sv = sv;
        m_playhead = m_sv->get_play_cursor();

	m_resync = config().get_property("AudioClip", "SyncDuringDrag", false).toBool();
        m_newTransportLocation = m_session->get_transport_location();
}

int PlayHeadMove::finish_hold()
{
	// When SyncDuringDrag is true, don't seek in finish_hold()
	// since that causes another audio glitch.
        if (!(m_resync && m_session->is_transport_rolling())) {
		// if the sheet is transporting, the seek action will cause 
		// the playcursor to be moved to the correct location.
		// Until then hide it, it will be shown again when the seek is finished!
                if (m_session->is_transport_rolling()) {
                        m_playhead->hide();
		}

                m_session->set_transport_pos(m_newTransportLocation);
	}
	m_sv->start_shuttle(false);
	return -1;
}

int PlayHeadMove::begin_hold()
{
        m_playhead->set_active(false);
        m_origXPos = m_newXPos = int(m_session->get_transport_location() / m_sv->timeref_scalefactor);
	m_sv->start_shuttle(true, true);
        m_holdCursorSceneY = cpointer().scene_y();

        ClipsViewPort* port = m_sv->get_clips_viewport();
        port->set_holdcursor_pos(QPointF(m_playhead->scenePos().x(), -20));
        int x = port->mapFromScene(m_playhead->scenePos()).x();

        if (x < 0 || x > port->width()) {
                m_sv->center_in_view(m_playhead, Qt::AlignHCenter);
        }

//        QCursor::setPos(port->mapToGlobal(
//                        port->mapFromScene(
//                        m_playhead->scenePos().x(), m_holdCursorSceneY)));


	// Mabye a technically more proper fix is to check if 
	// m_origXPos and the x pos in finish_hold() are equal or not ??
//	jog();
	return 1;
}

void PlayHeadMove::cancel_action()
{
	m_sv->start_shuttle(false);
        m_playhead->set_active(m_session->is_transport_rolling());
	if (!m_resync) {
                m_playhead->setPos(m_origXPos, 0);
	}
}


void PlayHeadMove::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	AbstractViewPort* viewPort = cpointer().get_viewport();
	if (viewPort)
	{
		viewPort->set_holdcursor(":/cursorHoldLr");
	}
}

int PlayHeadMove::jog()
{
	int x = cpointer().scene_x();
	int y = cpointer().scene_y();
	if (x < 0) {
		x = 0;
	}
	if (x == m_newXPos && y == m_newYPos) {
		return 0;
	}

	if (x != m_newXPos) {
                m_playhead->setPos(x, 0);

                m_newTransportLocation = TimeRef(x * m_sv->timeref_scalefactor);

                if (m_resync && m_session->is_transport_rolling()) {
                        m_session->set_transport_pos(m_newTransportLocation);
		}
		
		m_sv->update_shuttle_factor();
		m_sv->set_edit_cursor_text(timeref_to_text(m_newTransportLocation, m_sv->timeref_scalefactor));
	}
	
	if (cpointer().get_viewport())
	{
		cpointer().get_viewport()->set_holdcursor_pos(QPointF(x, y));
	}
	
	m_newXPos = x;
	m_newYPos = y;
	
	return 1;
}

void PlayHeadMove::move_left(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        if (m_doSnap) {
                return prev_snap_pos(autorepeat);
        }

        do_keyboard_move(m_newTransportLocation - (m_sv->timeref_scalefactor * m_speed));
}


void PlayHeadMove::move_right(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        if (m_doSnap) {
                return next_snap_pos(autorepeat);
        }
        do_keyboard_move(m_newTransportLocation + (m_sv->timeref_scalefactor * m_speed));
}


void PlayHeadMove::next_snap_pos(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        do_keyboard_move(m_session->get_snap_list()->next_snap_pos(m_newTransportLocation), true);
}

void PlayHeadMove::prev_snap_pos(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        do_keyboard_move(m_session->get_snap_list()->prev_snap_pos(m_newTransportLocation), true);
}

void PlayHeadMove::do_keyboard_move(TimeRef newLocation, bool centerInView)
{
        ied().bypass_jog_until_mouse_movements_exceeded_manhattenlength();

        if (newLocation < TimeRef()) {
                newLocation = TimeRef();
        }

        m_newTransportLocation = newLocation;

        if (m_resync && m_session->is_transport_rolling()) {
                m_session->set_transport_pos(m_newTransportLocation);
        } else {
                m_playhead->setPos(newLocation / m_sv->timeref_scalefactor, 0);

                int x = m_sv->get_clips_viewport()->mapFromScene(m_playhead->scenePos()).x();


                int canvasWidth = m_sv->get_clips_viewport()->width();
                int nearBorderMargin = 50;
                if (nearBorderMargin > (canvasWidth / 4))
                {
                        nearBorderMargin = 0;
                }

                if (x < (0 + nearBorderMargin) || x > (canvasWidth - nearBorderMargin)) {
                        m_sv->center_in_view(m_playhead, Qt::AlignHCenter);
                }
        }

        cpointer().get_viewport()->set_holdcursor_text(timeref_to_text(m_newTransportLocation, m_sv->timeref_scalefactor));
        cpointer().get_viewport()->set_holdcursor_pos(QPointF(m_playhead->scenePos().x(), m_holdCursorSceneY));
}

void PlayHeadMove::move_to_work_cursor(bool autorepeat)
{
        if (autorepeat) {
                return;
        }
        do_keyboard_move(m_session->get_work_location());
}
