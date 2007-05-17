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

#include <libtraversocore.h>

#include "MoveEdge.h"
#include <ViewPort.h>
#include <SongView.h>
#include <AudioClipView.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MoveEdge::MoveEdge(AudioClipView* cv, SongView* sv, QByteArray whichEdge)
	: Command(cv->get_clip(), tr("Move Clip Edge"))
{
        m_cv = cv;
        m_sv = sv;
	m_clip = cv->get_clip();
        m_edge = whichEdge;
}


MoveEdge::~MoveEdge()
{}

int MoveEdge::prepare_actions()
{
        PENTER;
	
	if (m_newPos == m_originalPos) {
		// Nothing happened!
		return -1;
	}
        
        return 1;
}

int MoveEdge::begin_hold()
{
	PENTER;
        if (m_edge == "set_left_edge") {
                m_newPos = m_originalPos = m_clip->get_track_start_frame();
	}
        if (m_edge == "set_right_edge") {
                m_newPos = m_originalPos = m_clip->get_track_end_frame();
	}

	m_clip->set_snappable(false);
	m_sv->stop_follow_play_head();

        return 1;
}


int MoveEdge::finish_hold()
{
	m_clip->set_snappable(true);

        return 1;
}


void MoveEdge::cancel_action()
{
	finish_hold();
	undo_action();
}


int MoveEdge::do_action()
{
        return QMetaObject::invokeMethod(m_clip, m_edge.data(), Q_ARG(long, m_newPos));
}


int MoveEdge::undo_action()
{
        return QMetaObject::invokeMethod(m_clip, m_edge.data(), Q_ARG(long, m_originalPos));
}


int MoveEdge::jog()
{
	m_newPos = cpointer().scene_x() * m_sv->scalefactor;

	if (m_sv->get_song()->is_snap_on()) {
		SnapList* slist = m_sv->get_song()->get_snap_list();
		m_newPos = slist->get_snap_value(m_newPos);
	}

        return do_action();
}

// eof

