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

#include "Themer.h"
#include "SheetView.h"
#include "MarkerView.h"
#include "TimeLineViewPort.h"
#include "SnapList.h"

#include <ProjectManager.h>
#include <Project.h>
#include <Sheet.h>
#include <TimeLine.h>
#include <Marker.h>
#include <ContextPointer.h>
#include <Utils.h>
#include <defines.h>
#include <AddRemove.h>
#include <CommandGroup.h>
#include "Information.h"
#include "InputEngine.h"

#include <QFont>
#include <QDebug>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


#define MARKER_SOFT_SELECTION_DISTANCE 50


DragMarker::DragMarker(MarkerView* mview, qint64 scalefactor, const QString& des)
	: Command(mview->get_marker(), des)
{
	d = new Data;
	d->view = mview;
	m_marker= d->view->get_marker();
	d->scalefactor = scalefactor;
	d->bypassjog = false;
}

int DragMarker::prepare_actions()
{
	return 1;
}

int DragMarker::begin_hold()
{
	m_origWhen = m_newWhen = m_marker->get_when();
	m_marker->set_snappable(false);
	d->view->get_sheetview()->start_shuttle(true, true);
	d->view->set_dragging(true);	
	return 1;
}

int DragMarker::finish_hold()
{
	d->view->get_sheetview()->start_shuttle(false);
	d->view->set_dragging(false);
	delete d;
	
	return 1;
}

int DragMarker::do_action()
{
	m_marker->set_when(m_newWhen);
	m_marker->set_snappable(true);
	return 1;
}

int DragMarker::undo_action()
{
	m_marker->set_when(m_origWhen);
	m_marker->set_snappable(true);
	return 1;
}

void DragMarker::cancel_action()
{
	finish_hold();
	undo_action();
}

void DragMarker::move_left(bool )
{
	d->bypassjog = true;
	// Move 1 pixel to the left
	TimeRef newpos = TimeRef(m_newWhen - d->scalefactor);
	if (newpos < TimeRef()) {
		newpos = TimeRef();
	}
	m_newWhen = newpos;
	do_action();
}

void DragMarker::move_right(bool )
{
	d->bypassjog = true;
	// Move 1 pixel to the right
	m_newWhen = m_newWhen + d->scalefactor;
	do_action();
}

int DragMarker::jog()
{
	if (d->bypassjog) {
		int diff = d->jogBypassPos - cpointer().x();
		if (abs(diff) > 15) {
			d->bypassjog = false;
		} else {
			return 0;
		}
	}
	
	d->jogBypassPos = cpointer().x();
	TimeRef newpos = TimeRef(cpointer().scene_x() * d->scalefactor);

	if (m_marker->get_timeline()->get_sheet()->is_snap_on()) {
		SnapList* slist = m_marker->get_timeline()->get_sheet()->get_snap_list();
		newpos = slist->get_snap_value(newpos);
	}

	if (newpos < TimeRef()) {
		newpos = TimeRef();
	}
	
	m_newWhen = newpos;
	d->view->set_position(int(m_newWhen / d->scalefactor));
	
	d->view->get_sheetview()->update_shuttle_factor();
	
	return 1;
}


// End DragMarker



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

	setAcceptsHoverEvents(true);

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
	
	painter->fillRect(xstart, 0,  pixelcount, height, themer()->get_color("Timeline:background") );
	
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
	
	TimeRef when(point.x() * m_sv->timeref_scalefactor);
	
	return add_marker_at(when);
}

Command* TimeLineView::add_marker_at_playhead()
{
	return add_marker_at(m_sv->get_sheet()->get_transport_location());
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
	update_softselected_marker(QPoint(cpointer().on_first_input_event_scene_x(), cpointer().on_first_input_event_scene_y()));

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

void TimeLineView::update_softselected_marker(QPoint pos)
{
	MarkerView* prevMarker = m_blinkingMarker;
	if (m_markerViews.size()) {
		m_blinkingMarker = m_markerViews.first();
	}
	
	if (! m_blinkingMarker) {
		return;
	}
	
	int x = pos.x();
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

void TimeLineView::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
{
	Q_UNUSED(event);

	if (m_blinkingMarker) {
		m_blinkingMarker->set_active(true);
	}
}

void TimeLineView::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
{
	Q_UNUSED(event);
	
	if (ie().is_holding()) {
		event->ignore();
		return;
	}
	
	if (m_blinkingMarker) {
		// TODO add these functions, or something else to 
		// let the user know which marker is to be moved!
		m_blinkingMarker->set_active(false);
		m_blinkingMarker = 0;
	}
}
		
		
void TimeLineView::hoverMoveEvent ( QGraphicsSceneHoverEvent * event )
{
	QPoint pos((int)event->pos().x(), (int)event->pos().y());

	update_softselected_marker(pos);
}


Command * TimeLineView::drag_marker()
{
	update_softselected_marker(QPoint(cpointer().on_first_input_event_scene_x(), cpointer().on_first_input_event_scene_y()));

	if (m_blinkingMarker) {
		return new DragMarker(m_blinkingMarker, m_sv->timeref_scalefactor, tr("Drag Marker"));
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

