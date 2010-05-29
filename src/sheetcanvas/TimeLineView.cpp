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

#include "TimeLineView.h"

#include <QPainter>

#include "MoveMarker.h"
#include "Themer.h"
#include "SheetView.h"
#include "MarkerView.h"
#include "TimeLineViewPort.h"

#include <Sheet.h>
#include <TimeLine.h>
#include <Marker.h>
#include <ContextPointer.h>
#include <Utils.h>
#include <defines.h>
#include <CommandGroup.h>
#include "Information.h"
#include "InputEngine.h"
#include <cstdlib>

#include <QFont>
#include <QDebug>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


#define MARKER_SOFT_SELECTION_DISTANCE 50


TimeLineView::TimeLineView(SheetView* view)
	: ViewItem(0, view->get_sheet()->get_timeline())
	, m_blinkingMarker(0)
{
	PENTERCONS2;

	m_sv = view;
	m_boundingRect = QRectF(0, 0, MAX_CANVAS_WIDTH, TIMELINE_HEIGHT);
	m_timeline = m_sv->get_sheet()->get_timeline();
	
	load_theme_data();
	
	// Create MarkerViews for existing markers
	foreach(Marker* marker, m_timeline->get_markers()) {
		add_new_marker_view(marker);
	}
	
	// Make connections to the 'core'
	connect(m_timeline, SIGNAL(markerAdded(Marker*)), this, SLOT(add_new_marker_view(Marker*)));
	connect(m_timeline, SIGNAL(markerRemoved(Marker*)), this, SLOT(remove_marker_view(Marker*)));
        connect(m_timeline, SIGNAL(activeContextChanged()), this, SLOT(active_context_changed()));

        m_hasMouseTracking = true;

	m_zooms[524288 * 640] = "20:00.000";
	m_zooms[262144 * 640] = "10:00.000";
	m_zooms[131072 * 640] = "5:00.000";
	m_zooms[ 65536 * 640] = "2:30.000";
	m_zooms[ 32768 * 640] = "1:00.000";
	m_zooms[ 16384 * 640] = "0:30.000";
	m_zooms[  8192 * 640] = "0:20.000";
	m_zooms[  4096 * 640] = "0:10.000";
	m_zooms[  2048 * 640] = "0:05.000";
	m_zooms[  1024 * 640] = "0:02.000";
	m_zooms[   512 * 640] = "0:01.000";
	m_zooms[   256 * 640] = "0:00.800";
	m_zooms[   128 * 640] = "0:00.400";
	m_zooms[    64 * 640] = "0:00.200";
	m_zooms[    32 * 640] = "0:00.100";
	m_zooms[    16 * 640] = "0:00.050";
	m_zooms[     8 * 640] = "0:00.020";
	m_zooms[     4 * 640] = "0:00.010";
	m_zooms[     2 * 640] = "0:00.005";
	m_zooms[     1 * 640] = "0:00.002";
}


TimeLineView::~TimeLineView()
{
	PENTERDES;
}


void TimeLineView::hzoom_changed( )
{
	update();
}

void TimeLineView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	PENTER3;
	Q_UNUSED(widget);
	
	painter->save();
	
	// When the scrollarea moves by a small value, the text
	// can be screwed up, so give it some room, 100 pixels should do!
	int xstart = (int) option->exposedRect.x() - 100;
	int pixelcount = (int) option->exposedRect.width() + 100;
	int expheight = (int) option->exposedRect.height();
	int top = (int) option->exposedRect.top();
	bool paintText = top > 28 &&  expheight < 2 ? false : true;
	
	if (xstart < 0) {
		xstart = 0;
	}
	
	painter->setClipRect(m_boundingRect);
	
	int height = TIMELINE_HEIGHT;
	
        QColor backgroundColor = themer()->get_color("Timeline:background");
        if (m_timeline->has_active_context()) {
                backgroundColor = backgroundColor.lighter(250);
        }
        painter->fillRect(xstart, 0,  pixelcount, height, backgroundColor );
	
	painter->setPen(themer()->get_color("Timeline:text"));
	painter->setFont( themer()->get_font("Timeline:fontscale:label") );
	
	TimeRef major;
	
	if (m_zooms.contains(m_sv->timeref_scalefactor)) {
		major = msms_to_timeref(m_zooms[m_sv->timeref_scalefactor]);
	} else {
		major = 120 * m_sv->timeref_scalefactor;
	}

	// minor is double so they line up right with the majors,
	// despite not always being an even number of frames
	// @Ben : is still still the same when using TimeRef based calculations?
	double minor = double(major/double(10));

	TimeRef firstLocation = TimeRef(xstart * m_sv->timeref_scalefactor);
	TimeRef lastLocation = TimeRef(xstart * m_sv->timeref_scalefactor + pixelcount * m_sv->timeref_scalefactor);
	int xstartoffset = m_sv->hscrollbar_value();
	
	painter->setMatrixEnabled(false);

	TimeRef factor = (firstLocation/major)*major;
	// Draw minor ticks
	TimeRef range((lastLocation-firstLocation+major) / minor);
	for (qint64 i = 0; i < range.universal_frame(); i++ ) {
		int x = (int)((factor + i * minor) / m_sv->timeref_scalefactor) - xstartoffset;
		painter->drawLine(x, height - 5, x, height - 1);
	}
	
	// Draw major ticks
	for (TimeRef location = factor; location < lastLocation; location += major) {
		int x = int(location/m_sv->timeref_scalefactor - xstartoffset);
		painter->drawLine(x, height - 13, x, height - 1);
		if (paintText) {
			painter->drawText(x + 4, height - 8, timeref_to_text(location, m_sv->timeref_scalefactor));
		}
	}
	
	painter->restore();
}

void TimeLineView::calculate_bounding_rect()
{
	update();
	ViewItem::calculate_bounding_rect();
}


void TimeLineView::add_new_marker_view(Marker * marker)
{
	MarkerView* view = new MarkerView(marker, m_sv, this);
	view->set_active(false);
	m_markerViews.append(view);
	view->update();
}

void TimeLineView::remove_marker_view(Marker * marker)
{
	foreach(MarkerView* view, m_markerViews) {
		if (view->get_marker() == marker) {
			m_markerViews.removeAll(view);
			scene()->removeItem(view);
			m_blinkingMarker = 0;
			delete view;
			return;
		}
	}
}

Command* TimeLineView::add_marker()
{
	QPointF point = mapFromScene(cpointer().scene_pos());
	
        qreal x = point.x();
        if (x < 0) {
                return 0;
        }
        TimeRef when(x * m_sv->timeref_scalefactor);
	
	return add_marker_at(when);
}

Command* TimeLineView::add_marker_at_playhead()
{
	return add_marker_at(m_sv->get_sheet()->get_transport_location());
}

Command* TimeLineView::add_marker_at_work_cursor()
{
        return add_marker_at(m_sv->get_sheet()->get_work_location());
}

Command* TimeLineView::add_marker_at(const TimeRef when)
{
	CommandGroup* group = new CommandGroup(m_timeline, "");

	// check if it is the first marker added to the timeline
	if (!m_timeline->get_markers().size()) {
		if (when > TimeRef()) {  // add one at the beginning of the sheet
			Marker* m = new Marker(m_timeline, TimeRef(), Marker::CDTRACK);
			m->set_description("");
			group->add_command(m_timeline->add_marker(m));
		}

		TimeRef lastlocation = m_sv->get_sheet()->get_last_location();
		if (when < lastlocation) {  // add one at the end of the sheet
			Marker* me = new Marker(m_timeline, lastlocation, Marker::ENDMARKER);
			me->set_description(tr("End"));
			group->add_command(m_timeline->add_marker(me));
		}
	}

	Marker* marker = new Marker(m_timeline, when, Marker::CDTRACK);
	marker->set_description("");
	
	group->setText(tr("Add Marker"));
	group->add_command(m_timeline->add_marker(marker));
	
	return group;
}

Command* TimeLineView::playhead_to_marker()
{
        update_softselected_marker(cpointer().on_first_input_event_scene_pos());

	if (m_blinkingMarker) {
		m_sv->get_sheet()->set_transport_pos(m_blinkingMarker->get_marker()->get_when());
		return 0;
	}

	return ie().did_not_implement();
}

Command* TimeLineView::remove_marker()
{
	if (m_blinkingMarker) {
		Marker* marker = m_blinkingMarker->get_marker();
		if (marker->get_type() == Marker::ENDMARKER && m_markerViews.size() > 1) {
			info().information(tr("You have to remove all other markers first."));
			return ie().failure();
		}
		return m_timeline->remove_marker(marker);
	}

	return 0;
}

void TimeLineView::update_softselected_marker(QPointF pos)
{
        // TODO : pos is scene_pos, but Marker positions are relative
        // to parent, not to scene, but since TimeLineView spans the scene
        // they happen to be the same now. Could change in the future?

	MarkerView* prevMarker = m_blinkingMarker;
	if (m_markerViews.size()) {
		m_blinkingMarker = m_markerViews.first();
	}
	
	if (! m_blinkingMarker) {
		return;
	}
	
        int x = int(pos.x());
	int blinkMarkerDist = abs(x - m_blinkingMarker->position());
	
	foreach(MarkerView* markerView, m_markerViews) {
		int markerDist = abs(x - markerView->position());

		fflush(stdout);
		if (markerDist < blinkMarkerDist) {
			m_blinkingMarker = markerView;
			blinkMarkerDist = abs(x - m_blinkingMarker->position());
		}
	}
	
	if (blinkMarkerDist > MARKER_SOFT_SELECTION_DISTANCE) {
		m_blinkingMarker = 0;
	}
	
	if (prevMarker && (prevMarker != m_blinkingMarker) ) {
		prevMarker->set_active(false);
		if (m_blinkingMarker) {
			m_blinkingMarker->set_active(true);
		}
	}
	
	if (!prevMarker && m_blinkingMarker) {
		m_blinkingMarker->set_active(true);
	}
}


void TimeLineView::active_context_changed()
{
        PENTER;
        if (has_active_context()) {
                if (m_blinkingMarker) {
                        m_blinkingMarker->set_active(true);
                }
        } else {
                if (ie().is_holding()) {
                        return;
                }

                if (m_blinkingMarker) {
                        // TODO add these functions, or something else to
                        // let the user know which marker is to be moved!
                        m_blinkingMarker->set_active(false);
                        m_blinkingMarker = 0;
                }

        }

        update();
}
		

void TimeLineView::mouse_hover_move_event()
{
        update_softselected_marker(cpointer().scene_pos());
}

Command * TimeLineView::drag_marker()
{
        update_softselected_marker(cpointer().on_first_input_event_scene_pos());

	if (m_blinkingMarker) {
                return new MoveMarker(m_blinkingMarker, m_sv->timeref_scalefactor, tr("Drag Marker"));
	}
	
	return ie().did_not_implement();
}

Command * TimeLineView::clear_markers()
{
	CommandGroup* group = new CommandGroup(m_timeline, tr("Clear Markers"));

	foreach(Marker *m, m_timeline->get_markers()) {
		group->add_command(m_timeline->remove_marker(m));
	}

	return group;
}

void TimeLineView::load_theme_data()
{
	// TODO Load pixmap, fonts, colors from themer() !!
	calculate_bounding_rect();
}

MarkerView* TimeLineView::get_marker_view_after(TimeRef location)
{
        foreach(MarkerView* markerView, m_markerViews) {
                if (markerView->get_marker()->get_when() > location) {
                        return markerView;
                }
        }
        return 0;
}

MarkerView* TimeLineView::get_marker_view_before(TimeRef location)
{
        for (int i=m_markerViews.size() - 1; i>= 0; --i) {
                MarkerView* markerView = m_markerViews.at(i);
                if (markerView->get_marker()->get_when() < location) {
                        return markerView;
                }
        }

        return 0;
}
