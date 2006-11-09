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
 
    $Id: SongView.h,v 1.2 2006/11/09 15:45:42 r_sijrier Exp $
*/

#ifndef SONG_VIEW_H
#define SONG_VIEW_H

#include "ViewItem.h"

class Song;
class TrackView;
class ClipsViewPort;
class TrackPanelViewPort;
class TimeLineViewPort;
class Track;
class SongWidget;
class TrackView;
class PlayCursor;
class WorkCursor;

class SongView : public ViewItem
{
        Q_OBJECT

public :

        SongView(ClipsViewPort* viewPort, TrackPanelViewPort* tpvp, TimeLineViewPort* tlvp, Song* song);
        ~SongView();
	
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {};
	QRectF boundingRect() const {return QRectF();}
	
	Song* get_song() const {return m_song;}
	ClipsViewPort* get_clips_viewport() const {return m_clipsViewPort;}
	TrackPanelViewPort* get_trackpanel_view_port() const;
	
	void update_zoom(int xFactor, int vZoomDirection);
	TrackView* get_trackview_under(QPointF point);

	int	scalefactor;

private:
        Song* 			m_song;
	ClipsViewPort* 		m_clipsViewPort;
	TrackPanelViewPort*	m_tpvp;
	TimeLineViewPort*	m_tlvp;
	QList<TrackView*>	m_trackViews;
	PlayCursor*		m_playCursor;
	WorkCursor*		m_workCursor;


public slots:
        void update_shuttle();

	Command* touch();
        Command* hzoom_out();
        Command* hzoom_in();
        Command* vzoom_out();
        Command* vzoom_in();
        Command* zoom();
        Command* center();
        Command* scroll_right();
        Command* scroll_left();
        Command* shuttle();
        Command* goto_begin();
        Command* goto_end();
	Command* play_cursor_move();
	
private slots:
	void scale_factor_changed();
	void set_snap_range(int);
	void add_new_trackview(Track*);
	void remove_trackview(Track*);
	void calculate_scene_rect();
};


#endif
