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
 
    $Id: Shuttle.cpp,v 1.1 2006/04/20 14:51:13 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "Shuttle.h"
#include "SongView.h"
#include "ViewPort.h"
#include "TrackView.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Shuttle::Shuttle(SongView* sv, ViewPort* vp)
                : Command()
{
        m_sv = sv;
        m_vp = vp;
        connect(&m_timer, SIGNAL(timeout() ), m_sv, SLOT (update_shuttle()) );
}


Shuttle::~Shuttle()
{}

int Shuttle::prepare_actions()
{
        PENTER;
        jog();
        m_sv->update_shuttle();
        return 1;
}

int Shuttle::begin_hold()
{
        // Call jog() to get our initial values (song->shuttleFactor)
        jog();
        m_timer.start(40);
        return 1;
}


int Shuttle::finish_hold()
{
        m_timer.stop();
        return 1;
}


int Shuttle::jog()
{
        int shuttlespeed = 10;
        float f = (float) cpointer().clip_area_x() / (m_vp->width() - TrackView::CLIPAREABASEX);

        if ( f > 0.85 || f < 0.15)
                shuttlespeed = 15;

        if ( f > 0.95 || f < 0.05)
                shuttlespeed = 20;

        if ( f > 0.98 || f < 0.02)
                shuttlespeed = 30;

        m_sv->shuttleFactor = (int) ( (( f * 30 ) - 15) * shuttlespeed );
        return 1;
}

int Shuttle::do_action( )
{
	return -1;
}

int Shuttle::undo_action( )
{
	return -1;
}

// eof


