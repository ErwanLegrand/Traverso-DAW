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
 
    $Id: MoveEdge.cpp,v 1.12 2007/01/16 15:18:37 r_sijrier Exp $
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
	: Command(cv->get_clip(), QObject::tr("Move Clip Edge"))
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
		return 0;
	}
        
        return 1;
}

int MoveEdge::begin_hold(int useX, int useY)
{
	PENTER;
	
	set_cursor_shape(useX, useY);
	
        if (m_edge == "set_left_edge") {
                m_newPos = m_originalPos = m_clip->get_track_start_frame();
	}
        if (m_edge == "set_right_edge") {
                m_newPos = m_originalPos = m_clip->get_track_end_frame();
	}
        return 1;
}


int MoveEdge::finish_hold()
{
	cpointer().get_viewport()->reset_context();
        return 1;
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
        return do_action();
}

// eof

