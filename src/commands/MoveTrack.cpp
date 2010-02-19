/*
Copyright (C) 2005-2008 Remon Sijrier

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

#include "MoveTrack.h"

#include "Sheet.h"
#include "Track.h"
#include "ViewItem.h"
#include "ContextPointer.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MoveTrack::MoveTrack(ViewItem* view)
        : Command(view->get_context(), "")
{
}

MoveTrack::~MoveTrack()
{
}

int MoveTrack::begin_hold()
{
        return 1;
}


int MoveTrack::finish_hold()
{

        return 1;
}


int MoveTrack::prepare_actions()
{

        return 1;
}


int MoveTrack::do_action()
{
        PENTER;

        return 1;
}


int MoveTrack::undo_action()
{
        PENTER;

        return 1;
}

void MoveTrack::cancel_action()
{
        finish_hold();
        undo_action();
}

int MoveTrack::jog()
{

        return 1;
}


void MoveTrack::move_up(bool autorepeat)
{
}

void MoveTrack::move_down(bool autorepeat)
{
}

void MoveTrack::set_cursor_shape(int useX, int useY)
{
        if (useX && useY) {
                cpointer().get_viewport()->set_holdcursor(":/cursorHoldUd");
        }
}
