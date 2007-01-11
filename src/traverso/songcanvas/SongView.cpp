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

$Id: SongView.cpp,v 1.6 2007/01/11 20:11:26 r_sijrier Exp $
*/


#include <QScrollBar>

#include "SongView.h"
#include "TrackView.h"
#include "Cursors.h"
#include "ClipsViewPort.h"
#include "TimeLineViewPort.h"
#include "TrackPanelViewPort.h"
		
#include <Song.h>
#include <Track.h>
#include <Peak.h>
#include <SnapList.h>
#include <ContextPointer.h>
#include <Zoom.h>
		
#include <Debugger.h>

class PlayCursorMove : public Command
{
public :
        PlayCursorMove(PlayCursor* cursor, Song* song, SongView* sv);
        ~PlayCursorMove(){PENTERDES;};

	int finish_hold();
        int begin_hold(int useX = 0, int useY = 0);
        int jog();

private :
	PlayCursor*	m_cursor;
	Song*		m_song;
	SongView*	m_sv;
	bool 		wasActive;
};

PlayCursorMove::PlayCursorMove(PlayCursor* cursor, Song* song, SongView* sv)
	: Command("Play Cursor Move")
	, m_cursor(cursor)
	, m_song(song)
	, m_sv(sv)
{
}

int PlayCursorMove::finish_hold()
{
	m_cursor->set_active(wasActive);
	m_song->set_transport_pos( (nframes_t) (cpointer().scene_x() * m_sv->scalefactor));
	return -1;
}

int PlayCursorMove::begin_hold(int useX, int useY)
{
	wasActive = m_cursor->is_active();
	m_cursor->set_active(false);
	return 1;
}

int PlayCursorMove::jog()
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
	m_song = song;
	m_clipsViewPort = viewPort;
	m_tpvp = tpvp;
	m_tlvp = tlvp;
	
	set_editing_mode();
	
	m_clipsViewPort->scene()->addItem(this);
	
	m_playCursor = new PlayCursor(this, m_song);
	m_workCursor = new WorkCursor(this, m_song);
	
	m_clipsViewPort->scene()->addItem(m_playCursor);
	m_clipsViewPort->scene()->addItem(m_workCursor);
	
	scale_factor_changed();
	
	foreach(Track* track, m_song->get_tracks()) {
		add_new_trackview(track);
	}
	
	calculate_scene_rect();
	
	connect(m_song, SIGNAL(hzoomChanged()), this, SLOT(scale_factor_changed()));
	connect(m_song, SIGNAL(trackAdded(Track*)), this, SLOT(add_new_trackview(Track*)));
	connect(m_song, SIGNAL(trackRemoved(Track*)), this, SLOT(remove_trackview(Track*)));
	connect(m_song, SIGNAL(lastFramePositionChanged()), this, SLOT(calculate_scene_rect()));
	connect(m_clipsViewPort->horizontalScrollBar(), 
		SIGNAL(valueChanged(int)),
		this, 
		SLOT(set_snap_range(int)));


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
		view = qgraphicsitem_cast<TrackView*>(views.at(i));
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
	int totalheight = 0;
	foreach(Track* track, m_song->get_tracks()) {
		totalheight += track->get_height() + 6;
	}
	totalheight += 150;
	int width = m_song->get_last_frame() / scalefactor + 500;
	
	m_clipsViewPort->setSceneRect(0, 0, width, totalheight);
	m_tlvp->setSceneRect(0, -2, width, -23);
	m_tpvp->setSceneRect(-2, 0, -200, totalheight);
	
	m_playCursor->set_bounding_rect(QRectF(0, 0, 2, totalheight));
	m_playCursor->update_position();
	m_workCursor->set_bounding_rect(QRectF(0, 0, 2, totalheight));
	m_workCursor->update_position();

// 	m_clipsViewPort->centerOn(m_song->get_working_frame() / scalefactor, 0);
}


Command* SongView::zoom()
{
	return new Zoom(this);
}

Command* SongView::hzoom_out()
{
	PENTER;
	m_song->set_hzoom(m_song->get_hzoom() + 1);
// 	center();
	return (Command*) 0;
}


Command* SongView::hzoom_in()
{
	PENTER;
	m_song->set_hzoom(m_song->get_hzoom() - 1);
// 	center();
	return (Command*) 0;
}


Command* SongView::vzoom_out()
{
	PENTER;
	int verticalposition = 6;
	for (int i=0; i<m_trackViews.size(); ++i) {
		TrackView* view = m_trackViews.at(i);
		Track* track = view->get_track();
		int height = track->get_height();
		height *= 1.2;
		if (height > 400) {
			break;
		}
		track->set_height(height);
		view->prepare_geometry_change();
		view->calculate_bounding_rect();
		view->move_to(0, verticalposition);
		verticalposition += height + 6;
	}
	
	scale_factor_changed();
	
	return (Command*) 0;
}


Command* SongView::vzoom_in()
{
	PENTER;
	int verticalposition = 6;
	for (int i=0; i<m_trackViews.size(); ++i) {
		TrackView* view = m_trackViews.at(i);
		Track* track = view->get_track();
		int height = track->get_height();
		height *= 0.8;
		if (height < 25) {
			break;
		}
		track->set_height(height);
		view->prepare_geometry_change();
		view->calculate_bounding_rect();
		view->move_to(0, verticalposition);
		verticalposition += height + 6;
	}
	
	scale_factor_changed();
	
	return (Command*) 0;
}

Command* SongView::center()
{
	PENTER2;
	m_clipsViewPort->centerOn(m_song->get_working_frame() / scalefactor, 0);
	return (Command*) 0;
}


Command* SongView::scroll_right()
{
	PENTER3;
	return (Command*) 0;
}


Command* SongView::scroll_left()
{
	PENTER3;
	return (Command*) 0;
}


Command* SongView::shuttle()
{
// 	return new Shuttle(this, m_vp);
}

void SongView::update_shuttle()
{
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


 
void SongView::update_zoom( int xFactor, int vZoomDirection )
{
	m_song->set_hzoom(xFactor);
}

TrackPanelViewPort* SongView::get_trackpanel_view_port( ) const
{
	return m_tpvp;
}

Command * SongView::touch( )
{
	QPointF point = m_clipsViewPort->mapToScene(cpointer().pos());
	m_song->set_work_at((uint) (point.x() * scalefactor));

	return 0;
}

Command * SongView::play_cursor_move( )
{
	return new PlayCursorMove(m_playCursor, m_song, this);
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


//eof
