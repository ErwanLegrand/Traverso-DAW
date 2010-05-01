/*
    Copyright (C) 2007 Remon Sijrier 
 
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

#include "WorkCursorMove.h"

#include "ContextPointer.h"
#include "ContextItem.h"
#include "InputEngine.h"
#include "ClipsViewPort.h"
#include "Marker.h"
#include "Sheet.h"
#include "SnapList.h"
#include "Snappable.h"
#include "SheetView.h"
#include "TimeLineViewPort.h"
#include "TimeLineView.h"
#include "MarkerView.h"
#include "TimeLine.h"
#include "Cursors.h"

#include <Debugger.h>

WorkCursorMove::WorkCursorMove(WorkCursor* wc, PlayHead* cursor, SheetView* sv)
        : MoveCommand("Play Cursor Move")
	, m_sheet(sv->get_sheet())
	, m_sv(sv)
	, m_playCursor(cursor)
        , m_workCursor(wc)
        , m_browseMarkers(false)
{
        m_holdCursorSceneY = cpointer().scene_y();
}

int WorkCursorMove::finish_hold()
{
	m_sheet->get_work_snap()->set_snappable(true);
	m_sv->start_shuttle(false);
	return -1;
}


int WorkCursorMove::begin_hold()
{
        if (m_sheet->is_transport_rolling()) {
		m_playCursor->disable_follow();
	}

	m_sheet->get_work_snap()->set_snappable(false);
	m_sv->start_shuttle(true, true);
	m_origPos = m_sheet->get_work_location();

        ClipsViewPort* port = m_sv->get_clips_viewport();
        port->set_holdcursor_pos(QPointF(m_workCursor->scenePos().x(), -20));
        int x = port->mapFromScene(m_workCursor->scenePos()).x();

        if (x < 0 || x > port->width()) {
                m_sv->center_in_view(m_workCursor, Qt::AlignHCenter);
        }

        QCursor::setPos(port->mapToGlobal(
                        port->mapFromScene(
                        m_workCursor->scenePos().x(), m_holdCursorSceneY)));

	return 1;
}

void WorkCursorMove::cancel_action()
{
	m_sheet->set_work_at(m_origPos);
        finish_hold();
}

void WorkCursorMove::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	cpointer().get_viewport()->set_holdcursor(":/cursorHoldLr");
}

int WorkCursorMove::jog()
{
        PENTER;
	int x = cpointer().scene_x();

	if (x < 0) {
		x = 0;
	}

	TimeRef newLocation(x * m_sv->timeref_scalefactor);

        if (newLocation == m_sheet->get_work_location()) {
                return 1;
        }

        if (m_sheet->is_snap_on() || m_doSnap) {
		SnapList* slist = m_sheet->get_snap_list();
		newLocation = slist->get_snap_value(newLocation);
	}

	m_sheet->set_work_at(newLocation);

	m_sv->update_shuttle_factor();
        cpointer().get_viewport()->set_holdcursor_text(timeref_to_text(newLocation, m_sv->timeref_scalefactor));
        cpointer().get_viewport()->set_holdcursor_pos(QPointF(m_workCursor->scenePos().x(), m_holdCursorSceneY));
	
	return 1;
}

void WorkCursorMove::move_left(bool autorepeat)
{
        Q_UNUSED(autorepeat);

        if (m_browseMarkers) {
                return browse_to_previous_marker();
        }

        // FIXME this should be done automatically when moving
        // the WC by means of setting the active items under the
        // edit point!!!
        remove_markers_from_active_context();

        if (m_doSnap) {
                return prev_snap_pos(autorepeat);
        }
        do_keyboard_move(m_sheet->get_work_location() - (m_sv->timeref_scalefactor * m_speed));
}


void WorkCursorMove::move_right(bool autorepeat)
{
        Q_UNUSED(autorepeat);

        if (m_browseMarkers) {
                return browse_to_next_marker();
        }

        // FIXME this should be done automatically when moving
        // the WC by means of setting the active items under the
        // edit point!!!
        remove_markers_from_active_context();

        if (m_doSnap) {
                return next_snap_pos(autorepeat);
        }
        do_keyboard_move(m_sheet->get_work_location() + (m_sv->timeref_scalefactor * m_speed));
}


void WorkCursorMove::next_snap_pos(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        do_keyboard_move(m_sheet->get_snap_list()->next_snap_pos(m_sheet->get_work_location()), true);
}

void WorkCursorMove::prev_snap_pos(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        do_keyboard_move(m_sheet->get_snap_list()->prev_snap_pos(m_sheet->get_work_location()), true);
}

void WorkCursorMove::do_keyboard_move(TimeRef newLocation, bool centerInView)
{
        ie().bypass_jog_until_mouse_movements_exceeded_manhattenlength();

        m_sheet->set_work_at(newLocation);

        if (centerInView) {
                m_sv->center_in_view(m_workCursor, Qt::AlignHCenter);
        }

        ClipsViewPort* port = m_sv->get_clips_viewport();
        QPoint point = port->mapToGlobal(port->mapFromScene(m_sheet->get_work_location() / m_sv->timeref_scalefactor, cpointer().scene_y()));
        QCursor::setPos(point);

        cpointer().get_viewport()->set_holdcursor_text(timeref_to_text(m_sheet->get_work_location(), m_sv->timeref_scalefactor));
        cpointer().get_viewport()->set_holdcursor_pos(QPointF(m_workCursor->scenePos().x(), m_holdCursorSceneY));
}

void WorkCursorMove::toggle_browse_markers(bool autorepeat)
{
        if (autorepeat) {
                return;
        }

        m_browseMarkers = !m_browseMarkers;
}

void WorkCursorMove::browse_to_next_marker()
{
        QList<Marker*> markers = m_sheet->get_timeline()->get_markers();
        QList<ContextItem*> contexts = cpointer().get_active_context_items();
        MarkerView* view;
        foreach(ContextItem* item, contexts) {
                view = qobject_cast<MarkerView*>(item);
                if (view) {
                        cpointer().remove_from_active_context_list(item);
                        contexts.removeAll(item);
                }
        }

        Marker* next = 0;
        foreach(Marker* marker, markers) {
                if (marker->get_when() > m_sheet->get_work_location()) {
                        next = marker;
                        break;
                }
        }

        if (next) {
                QList<MarkerView*> markerViews = m_sv->get_timeline_viewport()->get_timeline_view()->get_marker_views();
                foreach(MarkerView* view, markerViews) {
                        if (view->get_marker() == next) {
                                contexts.prepend(view);
                                break;
                        }
                }
                do_keyboard_move(next->get_when());
        }

        cpointer().set_active_context_items_by_keyboard_input(contexts);
}

void WorkCursorMove::browse_to_previous_marker()
{
        QList<Marker*> markers = m_sheet->get_timeline()->get_markers();
        QList<ContextItem*> contexts = cpointer().get_active_context_items();
        MarkerView* view;
        foreach(ContextItem* item, contexts) {
                view = qobject_cast<MarkerView*>(item);
                if (view) {
                        cpointer().remove_from_active_context_list(item);
                        contexts.removeAll(item);
                }
        }

        Marker* prev = 0;
        for (int i=markers.size() - 1; i>= 0; --i) {
                Marker* marker = markers.at(i);
                if (marker->get_when() < m_sheet->get_work_location()) {
                        prev = marker;
                        break;
                }
        }

        if (prev) {
                QList<MarkerView*> markerViews = m_sv->get_timeline_viewport()->get_timeline_view()->get_marker_views();
                foreach(MarkerView* view, markerViews) {
                        if (view->get_marker() == prev) {
                                contexts.prepend(view);
                                break;
                        }
                }

                do_keyboard_move(prev->get_when());
        }

        cpointer().set_active_context_items_by_keyboard_input(contexts);
}

void WorkCursorMove::remove_markers_from_active_context()
{
        QList<ContextItem*> contexts = cpointer().get_active_context_items();
        foreach(ContextItem* item, contexts) {
                if (item->inherits("MarkerView")) {
                        cpointer().remove_from_active_context_list(item);
                }
        }
}
