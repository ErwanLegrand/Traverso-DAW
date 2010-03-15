/*
Copyright (C) 2010 Remon Sijrier

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


#include "TrackFinder.h"

#include <QCursor>
#include <QMenu>

#include "Track.h"
#include "Project.h"
#include "ProjectManager.h"
#include "Sheet.h"
#include "SheetView.h"

#include "Debugger.h"


TrackFinder::TrackFinder(SheetView* view)
{
        m_sv = view;
}

int TrackFinder::begin_hold()
{
        return -1;
}

int TrackFinder::finish_hold()
{
        return -1;
}

int TrackFinder::prepare_actions()
{
        Sheet* sheet = pm().get_project()->get_current_sheet();
        QList<Track*> tracks = sheet->get_tracks();
        QMenu menu;
        foreach(Track* track, tracks) {
                QAction* action = menu.addAction(track->get_name());
                action->setData(track->get_id());
        }

        QAction* action = menu.exec(QCursor::pos());

        if (!action) {
                return -1;
        }

        Track* selectedTrack = sheet->get_track(action->data().toLongLong());

        m_sv->browse_to_track(selectedTrack);

        return -1;
}

int TrackFinder::do_action()
{
        return 1;
}

int TrackFinder::undo_action()
{
        return 1;
}

void TrackFinder::cancel_action()
{
        undo_action();
}

int TrackFinder::jog()
{
        return -1;
}

