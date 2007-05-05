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
#include "SongView.h"
#include "MarkerView.h"
#include "SnapList.h"

#include <ProjectManager.h>
#include <Project.h>
#include <Song.h>
#include <TimeLine.h>
#include <Marker.h>
#include <ContextPointer.h>
#include <Utils.h>
#include <Command.h>
#include <defines.h>
#include <AddRemove.h>
#include <CommandGroup.h>
#include <InputEngine.h>

#include <QDebug>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

#define TIMELINEHEIGHT 30

// TODO test if DragMarker class works as expected!!

class DragMarker : public Command
{
	Q_OBJECT
	Q_CLASSINFO("move_left", tr("Move Left"))
	Q_CLASSINFO("move_right", tr("Move right"))
	
public:
	DragMarker(MarkerView* mview, double scalefactor, const QString& des);

	int prepare_actions();
	int do_action();
	int undo_action();
	int finish_hold();
	int begin_hold();
	void cancel_action();
	int jog();

private :
	Marker*		m_marker;
	MarkerView*	m_view;
	nframes_t	m_origWhen;
	nframes_t	m_newWhen;
	double 		m_scalefactor;

public slots:
	void move_left(bool autorepeat);
	void move_right(bool autorepeat);
};


#include "TimeLineView.moc"

	
DragMarker::DragMarker(MarkerView* mview, double scalefactor, const QString& des)
	: Command(mview->get_marker(), des)
{
	m_view = mview;
	m_marker= m_view->get_marker();
	m_scalefactor = scalefactor;
}

int DragMarker::prepare_actions()
{
	return 1;
}

int DragMarker::begin_hold()
{
	m_origWhen = m_newWhen = m_marker->get_when();
	m_marker->set_snappable(false);
	m_view->get_songview()->start_shuttle(true, true);
	m_view->set_dragging(true);	
	return 1;
}

int DragMarker::finish_hold()
{
	m_marker->set_snappable(true);
	m_view->get_songview()->start_shuttle(false);
	m_view->set_dragging(false);
	return do_action();
}

int DragMarker::do_action()
{
	m_marker->set_when(m_newWhen);
	return 1;
}

int DragMarker::undo_action()
{
	m_marker->set_when(m_origWhen);
	return 1;
}

void DragMarker::cancel_action()
{
	finish_hold();
	undo_action();
}

void DragMarker::move_left(bool )
{
	// Move 1 pixel to the left
	long newpos = m_newWhen - (uint) ( 1 * m_scalefactor);
	if (newpos < 0) {
		newpos = 0;
	}
	m_newWhen = newpos;
	do_action();
}

void DragMarker::move_right(bool )
{
	// Move 1 pixel to the right
	m_newWhen = m_newWhen + (uint) ( 1 * m_scalefactor);
	do_action();
}

int DragMarker::jog()
{
	long newpos = (uint) (cpointer().scene_x() * m_scalefactor);

	if (m_marker->get_timeline()->get_song()->is_snap_on()) {
		SnapList* slist = m_marker->get_timeline()->get_song()->get_snap_list();
		newpos = slist->get_snap_value(newpos);
	}

	if (newpos < 0 ) {
		newpos = 0;
	}
	
	m_newWhen = newpos;
	m_view->set_position(int(m_newWhen / m_scalefactor));
	
	m_view->get_songview()->update_shuttle_factor();
	
	return 1;
}


// End DragMarker



TimeLineView::TimeLineView(SongView* view)
	: ViewItem(0, 0)
	, m_blinkingMarker(0)
{
	PENTERCONS2;
	
	m_sv = view;
	m_boundingRect = QRectF(0, 0, MAX_CANVAS_WIDTH, TIMELINEHEIGHT);
	m_timeline = m_sv->get_song()->get_timeline();
	m_samplerate = pm().get_project()->get_rate();
	
	view->scene()->addItem(this);
	
	load_theme_data();
	
	// Create MarkerViews for existing markers
	foreach(Marker* marker, m_timeline->get_markers()) {
		add_new_marker_view(marker);
	}
	
	// Make connections to the 'core'
	connect(m_timeline, SIGNAL(markerAdded(Marker*)), this, SLOT(add_new_marker_view(Marker*)));
	connect(m_timeline, SIGNAL(markerRemoved(Marker*)), this, SLOT(remove_marker_view(Marker*)));

	setAcceptsHoverEvents(true);

	m_zooms[131072] = "5:00.000";
	m_zooms[ 65536] = "2:30.000";
	m_zooms[ 32768] = "1:00.000";
	m_zooms[ 16384] = "0:30.000";
	m_zooms[  8192] = "0:20.000";
	m_zooms[  4096] = "0:10.000";
	m_zooms[  2048] = "0:05.000";
	m_zooms[  1024] = "0:02.000";
	m_zooms[   512] = "0:01.000";
	m_zooms[   256] = "0:00.800";
	m_zooms[   128] = "0:00.400";
	m_zooms[    64] = "0:00.200";
	m_zooms[    32] = "0:00.100";
	m_zooms[    16] = "0:00.050";
	m_zooms[     8] = "0:00.020";
	m_zooms[     4] = "0:00.010";
	m_zooms[     2] = "0:00.005";
	m_zooms[     1] = "0:00.002";
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
	
	// When the scrollarea moves by a small value, the text
	// can be screwed up, so give it some room, 100 pixels should do!
	int xstart = (int) option->exposedRect.x() - 100;
	int pixelcount = (int) option->exposedRect.width() + 100;
	if (xstart < 0) {
		xstart = 0;
	}
	
	int height = TIMELINEHEIGHT;
	
	painter->fillRect(xstart, 0,  pixelcount, height, themer()->get_color("Timeline:background") );
	
	painter->setPen(themer()->get_color("Timeline:text"));
	painter->setFont( QFont( "Bitstream Vera Sans", 9) );
	
	nframes_t major;
	
	if (m_zooms.contains(m_sv->scalefactor)) {
		major = smpte_to_frame(m_zooms[m_sv->scalefactor], m_samplerate);
	} else {
		major = 120 * m_sv->scalefactor;
	}

	bool showMs = (m_sv->scalefactor < 512);

	// minor is double so they line up right with the majors,
	// despite not always being an even number of frames
	double minor = major/10.0;

	nframes_t firstFrame = xstart * m_sv->scalefactor;
	nframes_t lastFrame = xstart * m_sv->scalefactor + pixelcount * m_sv->scalefactor;

	// Draw minor ticks
	for (int i = 0; i < (lastFrame-firstFrame+major) / minor; i++ ) {
		int x = (((int)(firstFrame/major))*major + i * minor)/m_sv->scalefactor;
		painter->drawLine(x, height - 5, x, height - 1);
	}
	
	// Draw major ticks
	for (nframes_t t = ((int)(firstFrame/major))*major; t < lastFrame; t += major ) {
		painter->drawLine(t/m_sv->scalefactor, height - 13, t/m_sv->scalefactor, height - 1);
		painter->drawText(t/m_sv->scalefactor + 4, height - 8, (showMs) ? frame_to_smpte(t, m_samplerate) : frame_to_ms(t, m_samplerate));
	}
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
	m_sv->scene()->addItem(view);
	update();
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
	
	nframes_t when = (uint) (point.x() * m_sv->scalefactor);
	
	CommandGroup* group = new CommandGroup(m_timeline, "");

	// check if it is the first marker added to the timeline
	if (!m_timeline->get_markers().size()) {
		if (when > 0) {  // add one at the beginning of the song
			Marker* m = new Marker(m_timeline, 0);
			m->set_description("");
			group->add_command(m_timeline->add_marker(m));
		}

		if (when < m_sv->get_song()->get_last_frame()) {  // add one at the end of the song
			Marker* me = new Marker(m_timeline, m_sv->get_song()->get_last_frame(), 10);
			me->set_description(tr("End"));
			group->add_command(m_timeline->add_marker(me));
		}
	}

	Marker* marker = new Marker(m_timeline, when);
	marker->set_description("");
	
	group->setText(tr("Add Marker"));
	group->add_command(m_timeline->add_marker(marker));
	
	return group;
}

Command* TimeLineView::remove_marker()
{
	if (m_blinkingMarker) {
		Marker* marker = m_blinkingMarker->get_marker();
		return m_timeline->remove_marker(marker);
	}

	return 0;
}

void TimeLineView::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
{
	PENTER;
	Q_UNUSED(event);

	if (m_blinkingMarker) {
		m_blinkingMarker->set_active(true);
	}
}

void TimeLineView::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
{
	PENTER;
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
	
	MarkerView* prevMarker = m_blinkingMarker;
	if (m_markerViews.size()) {
		m_blinkingMarker = m_markerViews.first();
	}
	
	if (! m_blinkingMarker) {
		return;
	}
	
	foreach(MarkerView* markerView, m_markerViews) {
		
		QPoint nodePos((int)markerView->pos().x(), (int)markerView->pos().y());
		
		int markerDist = (pos - nodePos).manhattanLength();
		int blinkNodeDist = (pos - QPoint((int)m_blinkingMarker->x(), (int)m_blinkingMarker->y())).manhattanLength();
		
		if (markerDist < blinkNodeDist) {
			m_blinkingMarker = markerView;
		}
	}
	

	if (prevMarker && (prevMarker != m_blinkingMarker) ) {
		prevMarker->set_active(false);
		m_blinkingMarker->set_active(true);
	}
	
	if (!prevMarker && m_blinkingMarker) {
		m_blinkingMarker->set_active(true);
	}
}


Command * TimeLineView::drag_marker()
{
	if (m_blinkingMarker) {
		return new DragMarker(m_blinkingMarker, m_sv->scalefactor, tr("Drag Marker"));
	}
	
	return 0;
}

Command * TimeLineView::clear_markers()
{
	QList<Marker*> lst = m_timeline->get_markers();
	
	CommandGroup* group = new CommandGroup(m_timeline, tr("Clear Markers"));

	foreach(Marker *m, lst) {
		group->add_command(m_timeline->remove_marker(m));
	}

	return group;
}

void TimeLineView::load_theme_data()
{
	// TODO Load pixmap, fonts, colors from themer() !!
	calculate_bounding_rect();
}

//eof

