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

#include "AudioTrackView.h"
#include "ClipsViewPort.h"
#include "ContextPointer.h"
#include "Sheet.h"
#include "SheetView.h"
#include "Track.h"
#include "TrackView.h"
#include "TrackPanelView.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MoveTrack::MoveTrack(TrackView* view)
        : Command(view->get_context(), "")
        , m_trackView(view)
{
        m_sv = m_trackView->get_sheetview();
        m_track = m_trackView->get_track();
}

MoveTrack::~MoveTrack()
{
}

int MoveTrack::begin_hold()
{
        m_sv->start_shuttle(true, true);

        return 1;
}


int MoveTrack::finish_hold()
{
        m_sv->start_shuttle(false);
        return 1;
}


int MoveTrack::prepare_actions()
{

        return -1;
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
        cpointer().get_viewport()->set_holdcursor_pos(cpointer().get_viewport()->mapToScene(cpointer().pos()).toPoint());

        TrackView* pointedView = m_sv->get_trackview_under(QPoint(0, cpointer().scene_y()));

        if (!pointedView) {
                return 1;
        }

        if (pointedView->get_track()->get_sort_index() > m_track->get_sort_index()) {
                move_down(false);
        }

        if (pointedView->get_track()->get_sort_index() < m_track->get_sort_index()) {
                move_up(false);
        }

        return 1;
}


void MoveTrack::move_up(bool /*autorepeat*/)
{
        m_sv->move_trackview_up(m_trackView);
}

void MoveTrack::move_down(bool /*autorepeat*/)
{
        m_sv->move_trackview_down(m_trackView);
}

void MoveTrack::set_cursor_shape(int /*useX*/, int useY)
{
        if (useY) {
                cpointer().get_viewport()->set_holdcursor(":/cursorHoldUd");
        }
}
