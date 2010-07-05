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

#include "TSession.h"

#include "AddRemove.h"
#include "Config.h"
#include "Peak.h"
#include "Utils.h"
#include "SubGroup.h"
#include "SnapList.h"
#include "Snappable.h"
#include "TimeLine.h"

#include "Debugger.h"

TSession::TSession()
        : ContextItem()
{
        m_timeline = new TimeLine(this);
        m_snaplist = new SnapList(this);
        m_workSnap = new Snappable();
        m_workSnap->set_snap_list(m_snaplist);

        // TODO seek to old position on project exit ?
        m_workLocation = TimeRef();
        m_transportLocation = TimeRef();
        m_mode = EDIT;
        m_sbx = m_sby = 0;
        m_hzoom = config().get_property("Sheet", "hzoomLevel", 8192).toInt();
        m_transport = 0;
        m_isSnapOn=true;
}

QList<Track*> TSession::get_tracks() const
{
        QList<Track*> list;
        apill_foreach(SubGroup* group, SubGroup, m_subGroups) {
                list.append(group);
        }
        return list;

}

QList<SubGroup*> TSession::get_subgroups() const
{
        QList<SubGroup*> list;
        apill_foreach(SubGroup* group, SubGroup, m_subGroups) {
                list.append(group);
        }

        return list;
}

SnapList* TSession::get_snap_list() const
{
        return m_snaplist;
}


void TSession::set_hzoom( qreal hzoom )
{
        // Traverso <= 0.42.0 doesn't store the real zoom factor, but an
        // index. This currently causes problems as there is no real support
        // (yet) for zoomlevels other then powers of 2, so we force that for now.
        // NOTE: Remove those 2 lines when floating point zoomlevel is implemented!
        int highbit;
        hzoom = nearest_power_of_two(hzoom, highbit);


        if (hzoom > Peak::max_zoom_value()) {
                hzoom = Peak::max_zoom_value();
        }

        if (hzoom < 1.0) {
                hzoom = 1.0;
        }

        if (m_hzoom == hzoom) {
                return;
        }

        m_hzoom = hzoom;

        emit hzoomChanged();
}

void TSession::set_work_at(TimeRef location, bool isFolder)
{
        PERROR("implement me!");
}

void TSession::set_temp_follow_state(bool state)
{
        emit tempFollowChanged(state);
}

void TSession::set_transport_pos(TimeRef location)
{
        PERROR("implement me!");
}

Command* TSession::set_editing_mode( )
{
        m_mode = EDIT;
        emit modeChanged();
        return 0;
}

Command* TSession::set_effects_mode( )
{
        m_mode = EFFECTS;
        emit modeChanged();
        return 0;
}

Command* TSession::toggle_effects_mode()
{
        if (m_mode == EDIT) {
                set_effects_mode();
        } else {
                set_editing_mode();
        }
        return 0;
}

Command* TSession::start_transport()
{
        PERROR("Implement me!");
        return 0;
}


Command* TSession::add_track(Track* track, bool historable)
{
        return new AddRemove(this, track, historable, this,
                "private_add_track(Track*)", "trackAdded(Track*)",
                "private_remove_track(Track*)", "trackRemoved(Track*)",
                tr("Added %1: %2").arg(track->metaObject()->className()).arg(track->get_name()));
}


Command* TSession::remove_track(Track* track, bool historable)
{
        return new AddRemove(this, track, historable, this,
                "private_remove_track(Track*)", "trackRemoved(Track*)",
                "private_add_track(Track*)", "trackAdded(Track*)",
                tr("Removed %1: %2").arg(track->metaObject()->className()).arg(track->get_name()));
}

void TSession::private_add_track(Track* track)
{
        switch (track->get_type()) {
        case Track::SUBGROUP: m_subGroups.append(track);
              break;
        default: ;// do nothing
        }
}

void TSession::private_remove_track(Track* track)
{
        switch (track->get_type()) {
        case Track::SUBGROUP: m_subGroups.remove(track);
              break;
        default: ;// do nothing
        }
}
