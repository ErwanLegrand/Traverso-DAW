/*
Copyright (C) 2005-2007 Remon Sijrier 

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

#include "SplitClip.h"
		
#include <libtraversocore.h>
#include <SheetView.h>
#include <AudioClipView.h>
#include "LineView.h"
#include <ViewItem.h>
#include "Fade.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


SplitClip::SplitClip(AudioClipView* view)
        : MoveCommand(view->get_clip(), tr("Split Clip"))
{
	m_clip = view->get_clip();
	m_sv = view->get_sheetview();
        m_session = m_sv->get_sheet();
	m_cv = view;
	m_track = m_clip->get_track();
	leftClip = 0;
	rightClip = 0;
	m_splitPoint = TimeRef();
	Q_ASSERT(m_clip->get_sheet());
}


int SplitClip::prepare_actions()
{
	if (m_splitPoint == qint64(0)) {
		m_splitPoint = TimeRef(cpointer().scene_x() * m_sv->timeref_scalefactor);
	}

	if (m_splitPoint <= m_clip->get_track_start_location() || m_splitPoint >= m_clip->get_track_start_location() + m_clip->get_length()) {
		return -1;
	}

	leftClip = resources_manager()->get_clip(m_clip->get_id());
	rightClip = resources_manager()->get_clip(m_clip->get_id());
	
	leftClip->set_sheet(m_clip->get_sheet());
	leftClip->set_track_start_location(m_clip->get_track_start_location());
	leftClip->set_right_edge(m_splitPoint);
	if (leftClip->get_fade_out()) {
		FadeRange* cmd = (FadeRange*)leftClip->reset_fade_out();
		cmd->set_historable(false);
		Command::process_command(cmd);
	}
	
	rightClip->set_sheet(m_clip->get_sheet());
	rightClip->set_left_edge(m_splitPoint);
	rightClip->set_track_start_location(m_splitPoint);
	if (rightClip->get_fade_in()) {
		FadeRange* cmd = (FadeRange*)rightClip->reset_fade_in();
		cmd->set_historable(false);
		Command::process_command(cmd);
	}
	
	return 1;
}


int SplitClip::do_action()
{
	PENTER;

	Command::process_command(m_track->add_clip(leftClip, false));
	Command::process_command(m_track->add_clip(rightClip, false));
	
	Command::process_command(m_track->remove_clip(m_clip, false));
	
	return 1;
}

int SplitClip::undo_action()
{
	PENTER;

	Command::process_command(m_track->add_clip(m_clip, false));
	
	Command::process_command(m_track->remove_clip(leftClip, false));
	Command::process_command(m_track->remove_clip(rightClip, false));
	
	return 1;
}

int SplitClip::begin_hold()
{
	m_sv->start_shuttle(true, true);
	m_splitcursor = new LineView(m_cv);
	m_splitcursor->set_color(themer()->get_color("AudioClip:contour"));
	m_cv->scene()->addItem(m_splitcursor);
	return 1;
}

int SplitClip::finish_hold()
{
	delete m_splitcursor;
	m_cv->update();
	m_sv->start_shuttle(false);
	return 1;
}

void SplitClip::cancel_action()
{
	finish_hold();
}

void SplitClip::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	cpointer().get_viewport()->set_holdcursor(":/cursorHoldLr");
}


int SplitClip::jog()
{
	int x = cpointer().scene_x();

	if (x < 0) {
		x = 0;
	}

	m_splitPoint = x * m_sv->timeref_scalefactor;

	if (m_clip->get_sheet()->is_snap_on()) {
		SnapList* slist = m_clip->get_sheet()->get_snap_list();
		m_splitPoint = slist->get_snap_value(m_splitPoint);
	}
	
	QPointF point = m_cv->mapFromScene(m_splitPoint / m_sv->timeref_scalefactor, cpointer().y());
	int xpos = (int) point.x();
	if (xpos < 0) {
		xpos = 0;
	}
	if (xpos > m_cv->boundingRect().width()) {
		xpos = (int)m_cv->boundingRect().width();
	}
	m_splitcursor->setPos(xpos, 0);
	m_sv->update_shuttle_factor();
	cpointer().get_viewport()->set_holdcursor_text(timeref_to_text(m_splitPoint, m_sv->timeref_scalefactor));
        cpointer().get_viewport()->set_holdcursor_pos(cpointer().scene_pos());
	
	return 1;
}


void SplitClip::move_left(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        if (m_doSnap) {
                return prev_snap_pos(autorepeat);
        }
        do_keyboard_move(m_splitPoint - (m_sv->timeref_scalefactor * m_speed));
}


void SplitClip::move_right(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        if (m_doSnap) {
                return next_snap_pos(autorepeat);
        }
        do_keyboard_move(m_splitPoint + (m_sv->timeref_scalefactor * m_speed));
}


void SplitClip::next_snap_pos(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        do_keyboard_move(m_session->get_snap_list()->next_snap_pos(m_splitPoint));
}

void SplitClip::prev_snap_pos(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        do_keyboard_move(m_session->get_snap_list()->prev_snap_pos(m_splitPoint));
}

void SplitClip::do_keyboard_move(TimeRef location)
{
        m_splitPoint = location;

        if (m_splitPoint < m_clip->get_track_start_location()) {
                m_splitPoint = m_clip->get_track_start_location();
        }
        if (m_splitPoint > m_clip->get_track_end_location()) {
                m_splitPoint = m_clip->get_track_end_location();
        }

        QPointF pos = m_cv->mapFromScene(m_splitPoint / m_sv->timeref_scalefactor, m_splitcursor->scenePos().y());
        m_splitcursor->setPos(pos);
}

// eof

