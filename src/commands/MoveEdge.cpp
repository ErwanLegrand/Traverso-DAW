/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: MoveEdge.cpp,v 1.7 2006/08/31 17:54:51 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "MoveEdge.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MoveEdge::MoveEdge(AudioClip* clip, QByteArray whichEdge)
                : Command(clip, tr("Move Clip Edge"))
{
        m_clip = clip;
        m_song = clip->get_song();
        m_edge = whichEdge;
}


MoveEdge::~MoveEdge()
{}

int MoveEdge::prepare_actions()
{
        PENTER;
	
	if (m_newPos == m_originalPos) {
		// Nothing happened!
		return 0;
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
	m_song->update_snaplist(m_clip);
        return 1;
}


int MoveEdge::finish_hold()
{
        return 1;
}


int MoveEdge::do_action()
{
        return QMetaObject::invokeMethod(m_clip, m_edge.data(), Q_ARG(nframes_t, m_newPos));
}


int MoveEdge::undo_action()
{
        return QMetaObject::invokeMethod(m_clip, m_edge.data(), Q_ARG(nframes_t, m_originalPos));
}


int MoveEdge::jog()
{
        m_newPos = m_song->xpos_to_frame(m_song->snapped_x(cpointer().clip_area_x()));
        return do_action();
}

// eof

