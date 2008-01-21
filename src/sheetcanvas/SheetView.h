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

#ifndef SONG_VIEW_H
#define SONG_VIEW_H

#include "ViewItem.h"
#include <QTimer>

class Sheet;
class TrackView;
class ClipsViewPort;
class TrackPanelViewPort;
class TimeLineViewPort;
class Track;
class SheetWidget;
class TrackView;
class PlayHead;
class WorkCursor;
class Curve;

class SheetView : public ViewItem
{
        Q_OBJECT
	Q_CLASSINFO("touch", tr("Set"))
	Q_CLASSINFO("touch_play_cursor", tr("Set"))
	Q_CLASSINFO("center", tr("Center View"))
	Q_CLASSINFO("scroll_right", tr("Right"))
	Q_CLASSINFO("scroll_left", tr("Left"))
	Q_CLASSINFO("scroll_up", tr("Up"))
	Q_CLASSINFO("scroll_down", tr("Down"))
	Q_CLASSINFO("scroll_right_hold", tr("Right"))
	Q_CLASSINFO("scroll_left_hold", tr("Left"))
	Q_CLASSINFO("scroll_up_hold", tr("Up"))
	Q_CLASSINFO("scroll_down_hold", tr("Down"))
	Q_CLASSINFO("shuttle", tr("Shuttle"))
	Q_CLASSINFO("goto_begin", tr("To start"))
	Q_CLASSINFO("goto_end", tr("To end"))
	Q_CLASSINFO("play_to_begin", tr("To Start"))
	Q_CLASSINFO("play_cursor_move", tr("Move"))
	Q_CLASSINFO("work_cursor_move", tr("Move"))
	Q_CLASSINFO("add_marker", tr("Add Marker"))
	Q_CLASSINFO("add_marker_at_playhead", tr("Add Marker at Playhead"))
	Q_CLASSINFO("playhead_to_workcursor", tr("To workcursor"))
	Q_CLASSINFO("center_playhead", tr("Center"))

public :

        SheetView(SheetWidget* sheetwidget,
		 	ClipsViewPort* viewPort,
    			TrackPanelViewPort* tpvp, 
       			TimeLineViewPort* tlvp, 
	  		Sheet* sheet);
        ~SheetView();
	
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {};
	QRectF boundingRect() const {return QRectF();}
	
	Sheet* get_sheet() const {return m_sheet;}
	TrackPanelViewPort* get_trackpanel_view_port() const;
	ClipsViewPort* get_clips_viewport() const;
	
	TrackView* get_trackview_under(QPointF point);
	
	void load_theme_data();
	void start_shuttle(bool start, bool drag=false);
	void update_shuttle_factor();
	void set_shuttle_factor_values(int x, int y);
	void vzoom(qreal scale);
	void hzoom(qreal scale);
	int hscrollbar_value() const;
	int vscrollbar_value() const;
	int get_peak_cache_zoom_factor() const {return m_peakCacheZoomFactor;}

	qint64		timeref_scalefactor;

private:
        Sheet* 			m_sheet;
	Curve*			m_shuttleCurve;
	Curve*			m_dragShuttleCurve;
	PlayHead*		m_playCursor;
	ClipsViewPort* 		m_clipsViewPort;
	TrackPanelViewPort*	m_tpvp;
	TimeLineViewPort*	m_tlvp;
	QList<TrackView*>	m_trackViews;
	WorkCursor*		m_workCursor;
	int			m_shuttleXfactor;
	int			m_shuttleYfactor;
	int			m_sceneHeight;
	int			m_peakCacheZoomFactor;
	bool			m_dragShuttle;
	QTimer			m_shuttletimer;
	QScrollBar*		m_vScrollBar;
	QScrollBar*		m_hScrollBar;
	bool			m_actOnPlayHead;
	
	// Themeing data
	int	m_trackSeperatingHeight;
	int	m_trackMinimumHeight;
	int	m_trackMaximumHeight;
	int	m_trackTopIndent;
	
	void layout_tracks();
	void set_hscrollbar_value(int value);
	void set_vscrollbar_value(int value);
	
	int mean_track_height();
	
	friend class PlayHead;


public slots:
	void set_snap_range(int);
	void update_scrollbars();
	void stop_follow_play_head();
	void follow_play_head();
	void set_follow_state(bool state);

	Command* touch();
	Command* touch_play_cursor();
        Command* center();
        Command* scroll_right();
        Command* scroll_left();
        Command* scroll_up();
        Command* scroll_down();
        Command* scroll_right_hold();
        Command* scroll_left_hold();
        Command* scroll_up_hold();
        Command* scroll_down_hold();
        Command* shuttle();
        Command* goto_begin();
        Command* goto_end();
	Command* play_to_begin();
	Command* play_cursor_move();
	Command* work_cursor_move();
	Command* add_marker();
	Command* add_marker_at_playhead();
	Command* playhead_to_workcursor();
	Command* center_playhead();
	
private slots:
	void scale_factor_changed();
	void add_new_trackview(Track*);
	void remove_trackview(Track*);
	void update_shuttle();
	void sheet_mode_changed();
	void hscrollbar_value_changed(int);
	void hscrollbar_action(int);
};


#endif
