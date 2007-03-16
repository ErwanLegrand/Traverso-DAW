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
 
    $Id: SongView.h,v 1.13 2007/03/16 13:15:54 r_sijrier Exp $
*/

#ifndef SONG_VIEW_H
#define SONG_VIEW_H

#include "ViewItem.h"
#include <QTimer>

class Song;
class TrackView;
class ClipsViewPort;
class TrackPanelViewPort;
class TimeLineViewPort;
class Track;
class SongWidget;
class TrackView;
class PlayHead;
class WorkCursor;

class SongView : public ViewItem
{
        Q_OBJECT
	Q_CLASSINFO("touch", tr("Touch"))
	Q_CLASSINFO("hzoom_out", tr("Horizontal Out"))
	Q_CLASSINFO("hzoom_in", tr("Horizontal In"))
	Q_CLASSINFO("vzoom_out", tr("Vertical Out"))
	Q_CLASSINFO("vzoom_in", tr("Vertical In"))
	Q_CLASSINFO("zoom", tr("Omnidirectional"))
	Q_CLASSINFO("center", tr("Center View"))
	Q_CLASSINFO("scroll_right", tr("Right"))
	Q_CLASSINFO("scroll_left", tr("Left"))
	Q_CLASSINFO("scroll_up", tr("Up"))
	Q_CLASSINFO("scroll_down", tr("Down"))
	Q_CLASSINFO("shuttle", tr("Shuttle"))
	Q_CLASSINFO("goto_begin", tr("To start"))
	Q_CLASSINFO("goto_end", tr("To end"))
	Q_CLASSINFO("play_cursor_move", tr("Playcursor: Move"))
	Q_CLASSINFO("set_editing_mode", tr("Mode: Edit"))
	Q_CLASSINFO("set_curve_mode", tr("Mode: Curve"))
	Q_CLASSINFO("add_marker", tr("Add Marker"))

public :

        SongView(ClipsViewPort* viewPort, TrackPanelViewPort* tpvp, TimeLineViewPort* tlvp, Song* song);
        ~SongView();
	
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {};
	QRectF boundingRect() const {return QRectF();}
	
	Song* get_song() const {return m_song;}
	TrackPanelViewPort* get_trackpanel_view_port() const;
	ClipsViewPort* get_clips_viewport() const;
	
	TrackView* get_trackview_under(QPointF point);
	
	void load_theme_data();
	void start_shuttle(bool start, bool drag=false);
	void update_shuttle_factor();
	
	int hscrollbar_value() const;
	int vscrollbar_value() const;

	int		scalefactor;
	ViewMode	viewmode;

private:
        Song* 			m_song;
	PlayHead*		m_playCursor;
	ClipsViewPort* 		m_clipsViewPort;
	TrackPanelViewPort*	m_tpvp;
	TimeLineViewPort*	m_tlvp;
	QList<TrackView*>	m_trackViews;
	WorkCursor*		m_workCursor;
	int			m_shuttleXfactor;
	int			m_shuttleYfactor;
	bool			m_dragShuttle;
	QTimer			m_shuttletimer;
	
	// Themeing data
	int	m_trackSeperatingHeight;
	int	m_trackMinimumHeight;
	int	m_trackMaximumHeight;
	int	m_trackTopIndent;
	
	void layout_tracks();


public slots:
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
	Command* add_marker();
	
private slots:
	void scale_factor_changed();
	void add_new_trackview(Track*);
	void remove_trackview(Track*);
	void calculate_scene_rect();
	void update_shuttle();
	
signals:
	void viewModeChanged();
};


#endif
