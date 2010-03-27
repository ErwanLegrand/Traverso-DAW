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

#include "ClipsViewPort.h"
#include "ContextPointer.h"
#include "Sheet.h"
#include "SheetView.h"
#include "Track.h"
#include "TrackView.h"
#include "Project.h"
#include "ProjectManager.h"

#include <QMenu>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MoveTrack::MoveTrack(TrackView* view)
        : Command(view->get_context(), "")
        , m_trackView(view)
{
        m_sv = m_trackView->get_sheetview();
}

MoveTrack::~MoveTrack()
{
}

int MoveTrack::begin_hold()
{
        m_sv->start_shuttle(true, true);
        m_trackView->set_moving(true);

        return 1;
}


int MoveTrack::finish_hold()
{
        m_sv->start_shuttle(false);
        m_trackView->set_moving(false);
        return 1;
}


int MoveTrack::prepare_actions()
{
        move_to_sheet();

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
        cpointer().get_viewport()->set_holdcursor_pos(cpointer().scene_pos());

        if (int(m_trackView->scenePos().y() + m_trackView->boundingRect().height()) < cpointer().scene_y()) {
                move_down();
        }

        if (int(m_trackView->scenePos().y()) > (cpointer().scene_y())) {
                move_up();
        }

        m_sv->update_shuttle_factor();

        return 1;
}


void MoveTrack::move_up()
{
        m_sv->move_trackview_up(m_trackView);
}

void MoveTrack::move_down()
{
        m_sv->move_trackview_down(m_trackView);
}


void MoveTrack::move_up(bool /*autorepeat*/)
{
        m_sv->move_trackview_up(m_trackView);
        m_sv->browse_to_track(m_trackView->get_track());
}

void MoveTrack::move_down(bool /*autorepeat*/)
{
        m_sv->move_trackview_down(m_trackView);
        m_sv->browse_to_track(m_trackView->get_track());
}

void MoveTrack::set_cursor_shape(int /*useX*/, int useY)
{
        if (useY) {
                cpointer().get_viewport()->set_holdcursor(":/cursorHoldUd");
        }
}

void MoveTrack::to_bottom(bool autorepeat)
{
        m_sv->to_bottom(m_trackView);
        m_sv->browse_to_track(m_trackView->get_track());
}

void MoveTrack::to_top(bool autorepeat)
{
        m_sv->to_top(m_trackView);
        m_sv->browse_to_track(m_trackView->get_track());
}

void MoveTrack::move_to_sheet()
{
        Project* project = pm().get_project();

        if (!project) {
                return;
        }

        QList<Sheet*> sheets = project->get_sheets();

        QMenu menu;

        foreach(Sheet* sheet, sheets) {
                QAction* action = menu.addAction(sheet->get_name());
                action->setData(sheet->get_id());
        }

        QAction* action = menu.exec(QCursor::pos());

        if (!action) {
                return;
        }

        qlonglong id = action->data().toLongLong();


        Track* track = m_trackView->get_track();
        Sheet* destination = project->get_sheet(id);
        Sheet* orig = track->get_sheet();

        if (! (destination || orig)) {
                return;
        }

        Command::process_command(orig->remove_track(track));
        Command::process_command(destination->add_track(track));

        m_sv->browse_to_track(track);
}
