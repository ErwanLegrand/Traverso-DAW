/*
Copyright (C) 2005-2006 Remon Sijrier 

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

$Id: SongView.cpp,v 1.16 2007/02/08 20:51:38 r_sijrier Exp $
*/


#include <QScrollBar>

#include "SongView.h"
#include "TrackView.h"
#include "Cursors.h"
#include "ClipsViewPort.h"
#include "TimeLineViewPort.h"
#include "TrackPanelViewPort.h"
#include "Themer.h"
		
#include <Song.h>
#include <Track.h>
#include <Peak.h>
#include <SnapList.h>
#include <ContextPointer.h>
#include <Zoom.h>
		
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


class PlayHeadMove : public Command
{
public :
        PlayHeadMove(PlayHead* cursor, Song* song, SongView* sv);
        ~PlayHeadMove(){PENTERDES;};

	int finish_hold();
        int begin_hold();
        int jog();

private :
	PlayHead*	m_cursor;
	Song*		m_song;
	SongView*	m_sv;
};

PlayHeadMove::PlayHeadMove(PlayHead* cursor, Song* song, SongView* sv)
	: Command("Play Cursor Move")
	, m_cursor(cursor)
	, m_song(song)
	, m_sv(sv)
{
}

int PlayHeadMove::finish_hold()
{
	m_cursor->set_active(m_song->is_transporting());
	m_song->set_transport_pos( (nframes_t) (cpointer().scene_x() * m_sv->scalefactor));
	return -1;
}

int PlayHeadMove::begin_hold()
{
	m_cursor->set_active(false);
	return 1;
}

int PlayHeadMove::jog()
{
	int x = cpointer().scene_x();
	if (x < 0) {
		x = 0;
	}
	m_cursor->setPos(x, 0);
	return 1;
}


		
SongView::SongView(ClipsViewPort* viewPort, TrackPanelViewPort* tpvp, TimeLineViewPort* tlvp, Song* song)
	: ViewItem(0, song)
{
	setZValue(1);
	
	m_song = song;
	m_clipsViewPort = viewPort;
	m_tpvp = tpvp;
	m_tlvp = tlvp;
	
	set_editing_mode();
	
	m_clipsViewPort->scene()->addItem(this);
	
	m_playCursor = new PlayHead(this, m_song, m_clipsViewPort);
	m_workCursor = new WorkCursor(this, m_song);
	
	m_clipsViewPort->scene()->addItem(m_playCursor);
	m_clipsViewPort->scene()->addItem(m_workCursor);
	
	scale_factor_changed();
	
	foreach(Track* track, m_song->get_tracks()) {
		add_new_trackview(track);
	}
	
	calculate_scene_rect();
	load_theme_data();
	
	// FIXME Center too position on song close!!
	center();
	
	connect(m_song, SIGNAL(hzoomChanged()), this, SLOT(scale_factor_changed()));
	connect(m_song, SIGNAL(trackAdded(Track*)), this, SLOT(add_new_trackview(Track*)));
	connect(m_song, SIGNAL(trackRemoved(Track*)), this, SLOT(remove_trackview(Track*)));
	connect(m_song, SIGNAL(lastFramePositionChanged()), this, SLOT(calculate_scene_rect()));
	connect(m_clipsViewPort->horizontalScrollBar(), 
		SIGNAL(valueChanged(int)),
		this, 
		SLOT(set_snap_range(int)));
	connect(&m_shuttletimer, SIGNAL(timeout() ), this, SLOT (update_shuttle()) );

}

SongView::~SongView()
{
}
		
void SongView::scale_factor_changed( )
{
	scalefactor = Peak::zoomStep[m_song->get_hzoom()];
	
	QList<QGraphicsItem*> list = scene()->items(m_clipsViewPort->sceneRect());
	
	for (int i=list.size() - 1; i>=0; --i) {
		ViewItem* item = (ViewItem*)list.at(i);
		item->prepare_geometry_change();
		item->calculate_bounding_rect();
	}
	
	calculate_scene_rect();
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
	
	view->move_to(0, sortIndex * (track->get_height() + 6) + 6);
	m_trackViews.append(view);
}

void SongView::remove_trackview(Track* track)
{
	foreach(TrackView* view, m_trackViews) {
		if (view->get_track() == track) {
			scene()->removeItem(view);
			delete view;
			return;
		}
	}
}

void SongView::calculate_scene_rect()
{
	m_clipsViewPort->setUpdatesEnabled(false);
	int totalheight = 0;
	foreach(Track* track, m_song->get_tracks()) {
		totalheight += track->get_height() + m_trackSeperatingHeight;
	}
	totalheight += 150;
	int width = m_song->get_last_frame() / scalefactor + m_clipsViewPort->height() / 2;
	
	m_clipsViewPort->setSceneRect(0, 0, width, totalheight);
	m_tlvp->setSceneRect(0, -2, width, -23);
	m_tpvp->setSceneRect(-2, 0, -200, totalheight);
	
	m_playCursor->set_bounding_rect(QRectF(0, 0, 2, totalheight));
	m_playCursor->update_position();
	m_workCursor->set_bounding_rect(QRectF(0, 0, 2, totalheight));
	m_workCursor->update_position();
	
// 	center();
	m_clipsViewPort->setUpdatesEnabled(true);

}


Command* SongView::zoom()
{
	return new Zoom(this);
}

Command* SongView::hzoom_out()
{
	PENTER;
	m_song->set_hzoom(m_song->get_hzoom() + 1);
	return (Command*) 0;
}


Command* SongView::hzoom_in()
{
	PENTER;
	m_song->set_hzoom(m_song->get_hzoom() - 1);
	return (Command*) 0;
}


Command* SongView::vzoom_out()
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
	scale_factor_changed();
	
	return (Command*) 0;
}


Command* SongView::vzoom_in()
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
	scale_factor_changed();
	
	return (Command*) 0;
}


void SongView::layout_tracks()
{
	int verticalposition = m_trackTopIndent;
	for (int i=0; i<m_trackViews.size(); ++i) {
		TrackView* view = m_trackViews.at(i);
		view->prepare_geometry_change();
		view->calculate_bounding_rect();
		view->move_to(0, verticalposition);
		verticalposition += (view->get_track()->get_height() + m_trackSeperatingHeight);
	}
}


Command* SongView::center()
{
	PENTER2;
	QScrollBar* scrollbar = m_clipsViewPort->horizontalScrollBar();
	scrollbar->setValue(m_song->get_working_frame() / scalefactor - m_clipsViewPort->width() / 2);
	return (Command*) 0;
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
	m_song->set_work_at((uint) (point.x() * scalefactor));

	return 0;
}

Command * SongView::play_cursor_move( )
{
	return new PlayHeadMove(m_playCursor, m_song, this);
}

void SongView::set_snap_range(int start)
{
	m_song->get_snap_list()->set_range(start * scalefactor, 
					(start + m_clipsViewPort->viewport()->width()) * scalefactor,
					scalefactor);
}

Command * SongView::set_editing_mode( )
{
	viewmode = EditMode;
	emit viewModeChanged();
	return 0;
}

Command * SongView::set_curve_mode( )
{
	viewmode = CurveMode;
	emit viewModeChanged();
	return 0;
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

void SongView::load_theme_data()
{
	m_trackSeperatingHeight = themer()->get_property("Song:track:seperatingheight", 0).toInt();
	m_trackMinimumHeight = themer()->get_property("Song:track:minimumheight", 22).toInt();
	m_trackMaximumHeight = themer()->get_property("Song:track:maximumheight", 22).toInt();
	m_trackTopIndent = themer()->get_property("Song:track:topindent", 6).toInt();
	
	m_clipsViewPort->setBackgroundBrush(themer()->get_color("Song:background"));
	m_tpvp->setBackgroundBrush(themer()->get_color("Song:background"));

	layout_tracks();
}


//eof


