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


#include <QScrollBar>
#include <libtraversocore.h>

#include "SongView.h"
#include "SongWidget.h"
#include "TrackView.h"
#include "TrackPanelView.h"
#include "Cursors.h"
#include "ClipsViewPort.h"
#include "TimeLineViewPort.h"
#include "TimeLineView.h"
#include "TrackPanelViewPort.h"
#include "Themer.h"
		
#include <Song.h>
#include <Track.h>
#include <Peak.h>
#include <SnapList.h>
#include <ContextPointer.h>
#include <Zoom.h>
#include <PlayHeadMove.h>
#include <WorkCursorMove.h>
		
#include <Debugger.h>

class Shuttle : public Command
{
public :
	Shuttle(SongView* sv) : Command("Shuttle"), m_sv(sv) {}

	int begin_hold() {
		m_sv->update_shuttle_factor();
		m_sv->start_shuttle(true);
		return 1;
	}

	int finish_hold() {
		m_sv->start_shuttle(false);
		return 1;
	}
	
	int jog() {
		m_sv->update_shuttle_factor();
		return 1;
	}

private :
	SongView*	m_sv;
};



SongView::SongView(SongWidget* songwidget, 
	ClipsViewPort* viewPort,
	TrackPanelViewPort* tpvp,
	TimeLineViewPort* tlvp,
	Song* song)
	: ViewItem(0, song)
{
	setZValue(1);
	
	m_song = song;
	m_clipsViewPort = viewPort;
	m_tpvp = tpvp;
	m_tlvp = tlvp;
	m_vScrollBar = songwidget->m_vScrollBar;
	m_hScrollBar = songwidget->m_hScrollBar;
	
	m_song->set_editing_mode();
	
	m_clipsViewPort->scene()->addItem(this);
	
	m_playCursor = new PlayHead(this, m_song, m_clipsViewPort);
	m_workCursor = new WorkCursor(this, m_song);
	connect(m_song, SIGNAL(workingPosChanged()), m_workCursor, SLOT(update_position()));
	connect(m_song, SIGNAL(workingPosChanged()), m_playCursor, SLOT(work_moved()));
	connect(m_song, SIGNAL(transportPosSet()), this, SLOT(play_head_updated()));
	connect(m_song, SIGNAL(workingPosChanged()), this, SLOT(work_cursor_updated()));

	m_clipsViewPort->scene()->addItem(m_playCursor);
	m_clipsViewPort->scene()->addItem(m_workCursor);
	
	m_clipsViewPort->setSceneRect(0, 0, MAX_CANVAS_WIDTH, MAX_CANVAS_HEIGHT);
	m_tlvp->setSceneRect(0, -30, MAX_CANVAS_WIDTH, 0);
	m_tpvp->setSceneRect(-200, 0, 0, MAX_CANVAS_HEIGHT);
	
	
	scalefactor = Peak::zoomStep[m_song->get_hzoom()];
	song_mode_changed();
	
	foreach(Track* track, m_song->get_tracks()) {
		add_new_trackview(track);
	}
	
	connect(m_song, SIGNAL(hzoomChanged()), this, SLOT(scale_factor_changed()));
	connect(m_song, SIGNAL(trackAdded(Track*)), this, SLOT(add_new_trackview(Track*)));
	connect(m_song, SIGNAL(trackRemoved(Track*)), this, SLOT(remove_trackview(Track*)));
	connect(m_song, SIGNAL(lastFramePositionChanged()), this, SLOT(update_scrollbars()));
	connect(m_song, SIGNAL(modeChanged()), this, SLOT(song_mode_changed()));
	connect(m_hScrollBar, SIGNAL(valueChanged(int)), this,SLOT(set_snap_range(int))); 
	connect(&m_shuttletimer, SIGNAL(timeout() ), this, SLOT (update_shuttle()) );
	
	connect(m_hScrollBar, SIGNAL(valueChanged(int)), this, SLOT(hscrollbar_value_changed(int)));
	connect(m_vScrollBar, SIGNAL(valueChanged(int)),
		m_clipsViewPort->verticalScrollBar(), SLOT(setValue(int)));

	connect(m_clipsViewPort->horizontalScrollBar(), SIGNAL(valueChanged(int)),
		  m_hScrollBar, SLOT(setValue(int)));

	connect(m_clipsViewPort->verticalScrollBar(), SIGNAL(valueChanged(int)),
		  m_vScrollBar, SLOT(setValue(int))); 
	
	load_theme_data();
	
	// FIXME Center too position on song close!!
	center();
}

SongView::~SongView()
{
}
		
void SongView::scale_factor_changed( )
{
	scalefactor = Peak::zoomStep[m_song->get_hzoom()];
	m_tlvp->scale_factor_changed();
	layout_tracks();
}

void SongView::song_mode_changed()
{
	int mode = m_song->get_mode();
	m_clipsViewPort->set_current_mode(mode);
	m_tlvp->set_current_mode(mode);
	m_tpvp->set_current_mode(mode);
}

TrackView* SongView::get_trackview_under( QPointF point )
{
	TrackView* view = 0;
	QList<QGraphicsItem*> views = m_clipsViewPort->items(m_clipsViewPort->mapFromScene(point));
	
	for (int i=0; i<views.size(); ++i) {
		view = dynamic_cast<TrackView*>(views.at(i));
		if (view) {
			return view;
		}
	}
	return  0;
	
}


void SongView::add_new_trackview(Track* track)
{
	TrackView* view = new TrackView(this, track);
	
	int sortIndex = track->get_sort_index();
	
	if (sortIndex < 0) {
		sortIndex = m_trackViews.size();
		track->set_sort_index(sortIndex);
	}
	
	m_trackViews.append(view);
	
	layout_tracks();
}

void SongView::remove_trackview(Track* track)
{
	foreach(TrackView* view, m_trackViews) {
		if (view->get_track() == track) {
			TrackPanelView* tpv = view->get_trackpanel_view();
			scene()->removeItem(tpv);
			scene()->removeItem(view);
			m_trackViews.removeAll(view);
			delete view;
			delete tpv;
			layout_tracks();
			return;
		}
	}
}

void SongView::update_scrollbars()
{
	int width = (m_song->get_last_frame() / scalefactor) - (m_clipsViewPort->width() / 4);
	
	m_hScrollBar->setRange(0, width);
	m_hScrollBar->setSingleStep(m_clipsViewPort->width() / 10);
	m_hScrollBar->setPageStep(m_clipsViewPort->width());
	
	m_vScrollBar->setRange(0, m_sceneHeight);
	m_vScrollBar->setSingleStep(m_clipsViewPort->height() / 10);
	m_vScrollBar->setPageStep(m_clipsViewPort->height());
	
	m_playCursor->set_bounding_rect(QRectF(0, 0, 2, m_sceneHeight));
	m_playCursor->update_position();
	m_workCursor->set_bounding_rect(QRectF(0, 0, 2, m_sceneHeight));
	m_workCursor->update_position();

	set_snap_range(m_hScrollBar->value());
}

void SongView::hscrollbar_value_changed(int value)
{
	if (!ie().is_holding()) {
		m_clipsViewPort->horizontalScrollBar()->setValue(value);
		set_snap_range(m_hScrollBar->value());
	}
}

Command* SongView::zoom()
{
	return new Zoom(this);
}

Command* SongView::hzoom_out()
{
	PENTER;
	m_song->set_hzoom(m_song->get_hzoom() + 1);
	center();
	return (Command*) 0;
}


Command* SongView::hzoom_in()
{
	PENTER;
	m_song->set_hzoom(m_song->get_hzoom() - 1);
	center();
	return (Command*) 0;
}


Command* SongView::vzoom_in()
{
	PENTER;
	for (int i=0; i<m_trackViews.size(); ++i) {
		TrackView* view = m_trackViews.at(i);
		Track* track = view->get_track();
		int height = track->get_height();
		height = (int) (height * 1.2);
		if (height > m_trackMaximumHeight) {
			height = m_trackMaximumHeight;
		}
		track->set_height(height);
	}
	
	layout_tracks();
	
	return (Command*) 0;
}


Command* SongView::vzoom_out()
{
	PENTER;
	for (int i=0; i<m_trackViews.size(); ++i) {
		TrackView* view = m_trackViews.at(i);
		Track* track = view->get_track();
		int height = track->get_height();
		height = (int) (height * 0.8);
		if (height < m_trackMinimumHeight) {
			height = m_trackMinimumHeight;
		}
		track->set_height(height);
	}
	
	layout_tracks();
	
	return (Command*) 0;
}


void SongView::layout_tracks()
{
	int verticalposition = m_trackTopIndent;
	for (int i=0; i<m_trackViews.size(); ++i) {
		TrackView* view = m_trackViews.at(i);
		view->calculate_bounding_rect();
		view->move_to(0, verticalposition);
		verticalposition += (view->get_track()->get_height() + m_trackSeperatingHeight);
	}
	
	m_sceneHeight = verticalposition;
	update_scrollbars();
}


Command* SongView::center()
{
	PENTER2;
	QScrollBar* scrollbar = m_clipsViewPort->horizontalScrollBar();

	nframes_t centerX;
	if (m_song->is_transporting() && m_actOnPlayHead) { 
		centerX = m_song->get_transport_frame();
	} else {
		centerX = m_song->get_working_frame();
	}

	scrollbar->setValue(centerX / scalefactor - m_clipsViewPort->width() / 2);
	return (Command*) 0;
}


void SongView::work_cursor_updated()
{
	m_actOnPlayHead = false;
	m_playCursor->disable_follow();
}


void SongView::play_head_updated()
{
	m_actOnPlayHead = true;
	m_playCursor->enable_follow();
}


Command* SongView::shuttle()
{
 	return new Shuttle(this);
}


void SongView::start_shuttle(bool start, bool drag)
{
	if (start) {
		m_shuttletimer.start(40);
		m_dragShuttle = drag;
		m_shuttleYfactor = m_shuttleXfactor = 0;
	} else {
		m_shuttletimer.stop();
	}
}

void SongView::update_shuttle_factor()
{
	int shuttlespeed;
	
// TODO Interpolate normalized value from a (Fade)Curve, anyone ?

	if(m_dragShuttle) {
		float normalizedX = (float) cpointer().x() / m_clipsViewPort->width();
		shuttlespeed = 0;
		if ( normalizedX > 0.87 || normalizedX < 0.13)
			shuttlespeed = 3;
	
		else if ( normalizedX > 0.90 || normalizedX < 0.10)
			shuttlespeed = 8;
	
		else if ( normalizedX > 0.95 || normalizedX < 0.05)
			shuttlespeed = 20;
		
		else if ( normalizedX > 0.98 || normalizedX < 0.02)
			shuttlespeed = 30;
	
		m_shuttleXfactor = (int) ( (( normalizedX * 30 ) - 15) * shuttlespeed / 2 );
		
	
		shuttlespeed = 0;
		float normalizedY = (float) cpointer().y() / m_clipsViewPort->height();
		
		if ( normalizedY > 0.80 || normalizedY < 0.20)
			shuttlespeed = 3;
	
		else if ( normalizedY > 0.90 || normalizedY < 0.10)
			shuttlespeed = 8;
	
		else if ( normalizedY > 0.95 || normalizedY < 0.05)
			shuttlespeed = 20;
		
		else if ( normalizedY > 0.98 || normalizedY < 0.02)
			shuttlespeed = 30;
		
		m_shuttleYfactor = (int) ( (( normalizedY * 30 ) - 15) * shuttlespeed / 2 );
	
	} else {
		shuttlespeed = 0;
		float normalizedX = (float) cpointer().x() / m_clipsViewPort->width();
		
		if ( normalizedX > 0.6 || normalizedX < 0.4)
			shuttlespeed = 7;
	
		else if ( normalizedX > 0.85 || normalizedX < 0.15)
			shuttlespeed = 15;
	
		else if ( normalizedX > 0.95 || normalizedX < 0.05)
			shuttlespeed = 20;
	
		else if ( normalizedX > 0.98 || normalizedX < 0.02)
			shuttlespeed = 30;
	
		m_shuttleXfactor = (int) ( (( normalizedX * 30 ) - 15) * shuttlespeed / 2 );
		
		
		shuttlespeed = 0;
		float normalizedY = (float) cpointer().y() / m_clipsViewPort->height();
		
		if ( normalizedY > 0.6 || normalizedY < 0.4)
			shuttlespeed = 5;
		
		else if ( normalizedY > 0.80 || normalizedY < 0.20)
			shuttlespeed = 15;
	
		else if ( normalizedY > 0.90 || normalizedY < 0.10)
			shuttlespeed = 20;
	
		else if ( normalizedY > 0.98 || normalizedY < 0.02)
			shuttlespeed = 30;
		
		m_shuttleYfactor = (int) ( (( normalizedY * 30 ) - 15) * shuttlespeed / 2 );
	}
	
}


void SongView::update_shuttle()
{
	QScrollBar* hscrollbar = m_clipsViewPort->horizontalScrollBar();
	hscrollbar->setValue(hscrollbar->value() + m_shuttleXfactor);
	
	QScrollBar* vscrollbar = m_clipsViewPort->verticalScrollBar();
	vscrollbar->setValue(vscrollbar->value() + m_shuttleYfactor);
	
	if (m_shuttleXfactor != 0 || m_shuttleYfactor != 0) {
		ie().jog();
	}
}


Command* SongView::goto_begin()
{
	m_song->set_work_at(0);
	center();
	return (Command*) 0;
}


Command* SongView::goto_end()
{
	m_song->set_work_at(m_song->get_last_frame());
	center();
	return (Command*) 0;
}


TrackPanelViewPort* SongView::get_trackpanel_view_port( ) const
{
	return m_tpvp;
}

ClipsViewPort * SongView::get_clips_viewport() const
{
	return m_clipsViewPort;
}


Command * SongView::touch( )
{
	QPointF point = m_clipsViewPort->mapToScene(cpointer().pos());
	m_song->set_work_at((nframes_t) (point.x() * scalefactor));

	if (!m_song->is_transporting()) {
		m_playCursor->setPos(point.x(), 0);
		m_song->set_transport_pos( (nframes_t) (point.x() * scalefactor));
	}

	return 0;
}

Command * SongView::play_cursor_move( )
{
	return new PlayHeadMove(m_playCursor, this);
}

Command * SongView::work_cursor_move( )
{
	return new WorkCursorMove(m_playCursor, this);
}

void SongView::set_snap_range(int start)
{
// 	printf("SongView::set_snap_range\n");
	m_song->get_snap_list()->set_range(start * scalefactor, 
					(start + m_clipsViewPort->viewport()->width()) * scalefactor,
					scalefactor);
}

Command* SongView::scroll_up( )
{
	QScrollBar* scrollbar = m_clipsViewPort->verticalScrollBar();
	scrollbar->setValue(scrollbar->value() - 50);
	return 0;
}

Command* SongView::scroll_down( )
{
	QScrollBar* scrollbar = m_clipsViewPort->verticalScrollBar();
	scrollbar->setValue(scrollbar->value() + 50);
	return 0;
}

Command* SongView::scroll_right()
{
	PENTER3;
	QScrollBar* scrollbar = m_clipsViewPort->horizontalScrollBar();
	scrollbar->setValue(scrollbar->value() + 50);
	return (Command*) 0;
}


Command* SongView::scroll_left()
{
	PENTER3;
	QScrollBar* scrollbar = m_clipsViewPort->horizontalScrollBar();
	scrollbar->setValue(scrollbar->value() - 50); 
	return (Command*) 0;
}

int SongView::hscrollbar_value() const
{
	return m_clipsViewPort->horizontalScrollBar()->value();
}

int SongView::vscrollbar_value() const
{
	return m_clipsViewPort->verticalScrollBar()->value();
}

void SongView::load_theme_data()
{
	m_trackSeperatingHeight = themer()->get_property("Song:track:seperatingheight", 0).toInt();
	m_trackMinimumHeight = themer()->get_property("Song:track:minimumheight", 16).toInt();
	m_trackMaximumHeight = themer()->get_property("Song:track:maximumheight", 300).toInt();
	m_trackTopIndent = themer()->get_property("Song:track:topindent", 6).toInt();
	
	m_clipsViewPort->setBackgroundBrush(themer()->get_color("Song:background"));
	m_tpvp->setBackgroundBrush(themer()->get_color("Song:background"));

	layout_tracks();
}

Command * SongView::add_marker()
{
	return m_tlvp->get_timeline_view()->add_marker();
}

Command * SongView::playhead_to_workcursor( )
{
	nframes_t work = m_song->get_working_frame();

	m_song->set_transport_pos( work );
	m_playCursor->setPos(work / scalefactor, 0);
	m_playCursor->work_moved();

	return (Command*) 0;
}

//eof

