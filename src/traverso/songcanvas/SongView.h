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
 
    $Id: SongView.h,v 1.6 2007/01/22 15:12:08 r_sijrier Exp $
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
	Q_CLASSINFO("touch", tr("Touch"))
	Q_CLASSINFO("hzoom_out", tr("Zoom: Horizontal Out"))
	Q_CLASSINFO("hzoom_in", tr("Zoom: Horizontal In"))
	Q_CLASSINFO("vzoom_out", tr("Zoom: Vertical Out"))
	Q_CLASSINFO("vzoom_in", tr("Zoom: Vertical In"))
	Q_CLASSINFO("zoom", tr("Zoom"))
	Q_CLASSINFO("center", tr("Center View"))
	Q_CLASSINFO("scroll_right", tr("Scroll: right"))
	Q_CLASSINFO("scroll_left", tr("Scroll: left"))
	Q_CLASSINFO("scroll_up", tr("Scroll: up"))
	Q_CLASSINFO("scroll_down", tr("Scroll: down"))
	Q_CLASSINFO("shuttle", tr("Shuttle"))
	Q_CLASSINFO("goto_begin", tr("Workcursor: To start"))
	Q_CLASSINFO("goto_end", tr("Workcurosr: To end"))
	Q_CLASSINFO("play_cursor_move", tr("Playcursor: Move"))
	Q_CLASSINFO("set_editing_mode", tr("Mode: Edit"))
	Q_CLASSINFO("set_curve_mode", tr("Mode: Curve"))

public :

        SongView(ClipsViewPort* viewPort, TrackPanelViewPort* tpvp, TimeLineViewPort* tlvp, Song* song);
        ~SongView();
	
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {};
	QRectF boundingRect() const {return QRectF();}
	
	Song* get_song() const {return m_song;}
	TrackPanelViewPort* get_trackpanel_view_port() const;
	
	TrackView* get_trackview_under(QPointF point);

	int		scalefactor;
	ViewMode	viewmode;

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
	void set_snap_range(int);

	Command* touch();
        Command* hzoom_out();
        Command* hzoom_in();
        Command* vzoom_out();
        Command* vzoom_in();
        Command* zoom();
        Command* center();
        Command* scroll_right();
        Command* scroll_left();
        Command* scroll_up();
        Command* scroll_down();
        Command* shuttle();
        Command* goto_begin();
        Command* goto_end();
	Command* play_cursor_move();
	Command* set_editing_mode();
	Command* set_curve_mode();
	
private slots:
	void scale_factor_changed();
	void add_new_trackview(Track*);
	void remove_trackview(Track*);
	void calculate_scene_rect();
	
signals:
	void viewModeChanged();
};


#endif
