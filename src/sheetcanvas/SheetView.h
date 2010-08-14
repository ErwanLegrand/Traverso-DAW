/*
    Copyright (C) 2005-2010 Remon Sijrier
 
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

class AudioClip;
class Sheet;
class AudioClipView;
class AudioTrackView;
class ClipsViewPort;
class CurveView;
class CurveNodeView;
class MarkerView;
class TrackPanelViewPort;
class TimeLineViewPort;
class TimeLineView;
class TrackView;
class TSession;
class SheetWidget;
class AudioTrackView;
class Track;
class PlayHead;
class WorkCursor;
class TEditCursor;
class Curve;

struct ItemBrowserData {
        ItemBrowserData() {
                acv = 0;
                atv = 0;
                tv = 0;
                curveView = 0;
                markerView = 0;
                timeLineView = 0;
        }

        TimeLineView* timeLineView;
        MarkerView* markerView;
        TrackView* tv;
        AudioTrackView* atv;
        AudioClipView* acv;
        CurveView* curveView;
        QString currentContext;
};

class SheetView : public ViewItem
{
        Q_OBJECT

public :

        SheetView(SheetWidget* sheetwidget,
		 	ClipsViewPort* viewPort,
    			TrackPanelViewPort* tpvp, 
       			TimeLineViewPort* tlvp, 
                        TSession* sheet);
        ~SheetView();
	
        void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}
	QRectF boundingRect() const {return QRectF();}
	
        TSession* get_sheet() const {return m_session;}
	TrackPanelViewPort* get_trackpanel_view_port() const;
	ClipsViewPort* get_clips_viewport() const;
        TimeLineViewPort* get_timeline_viewport() const;
	
        AudioTrackView* get_audio_trackview_under(QPointF point);
        TrackView* get_trackview_under(QPointF point);
        QList<TrackView*> get_track_views() const;
        int get_track_height(Track* track) const;
	
	void load_theme_data();
	void start_shuttle(bool start, bool drag=false);
	void update_shuttle_factor();
	void set_shuttle_factor_values(int x, int y);
	void vzoom(qreal scale);
	void hzoom(qreal scale);
	void clipviewport_resize_event();
	int hscrollbar_value() const;
	int vscrollbar_value() const;
        void move_trackview_up(TrackView* trackView);
        void move_trackview_down(TrackView* trackView);
        void to_top(TrackView* trackView);
        void to_bottom(TrackView* trackView);
        void set_track_height(TrackView* view, int newheight);
        void set_hscrollbar_value(int value);
        void set_vscrollbar_value(int value);

        void set_cursor_shape(const QString& cursor);


        void browse_to_track(Track* track);
        void browse_to_audio_clip_view(AudioClipView* acv);
        void browse_to_curve_view(CurveView* curveView);
        void browse_to_curve_node_view(CurveNodeView* nodeView);
        void browse_to_marker_view(MarkerView* markerView);
        void center_in_view(ViewItem* item, enum Qt::AlignmentFlag = Qt::AlignHCenter);
        void move_edit_point_to(TimeRef location, int sceneY);

	qint64		timeref_scalefactor;

private:
        TSession*		m_session;
	Curve*			m_shuttleCurve;
	Curve*			m_dragShuttleCurve;
	PlayHead*		m_playCursor;
	ClipsViewPort* 		m_clipsViewPort;
	TrackPanelViewPort*	m_tpvp;
	TimeLineViewPort*	m_tlvp;
        QList<TrackView*>	m_audioTrackViews;
        QList<TrackView*>	m_busTrackViews;
        TrackView*              m_sheetMasterOutView;
        TrackView*              m_projectMasterOutView;
        WorkCursor*		m_workCursor;
        TEditCursor*            m_editCursor;
	int			m_shuttleXfactor;
	int			m_shuttleYfactor;
	int			m_sceneHeight;
        int                     m_meanTrackHeight;
        bool			m_dragShuttle;
	QTimer			m_shuttletimer;
	QScrollBar*		m_vScrollBar;
	QScrollBar*		m_hScrollBar;
	bool			m_actOnPlayHead;
	bool			m_viewportReady;
	
	// Themeing data
	int	m_trackSeperatingHeight;
	int	m_trackMinimumHeight;
	int	m_trackMaximumHeight;
	int	m_trackTopIndent;
	
	void layout_tracks();
        void update_tracks_bounding_rect();
        void collect_item_browser_data(ItemBrowserData& data);


public slots:
	void set_snap_range(int);
	void update_scrollbars();
	void stop_follow_play_head();
	void follow_play_head();
	void set_follow_state(bool state);
	void transport_position_set();

	TCommand* touch();
	TCommand* touch_play_cursor();
        TCommand* center();
        TCommand* scroll_right();
        TCommand* scroll_left();
        TCommand* scroll_up();
        TCommand* scroll_down();
        TCommand* to_upper_context_level();
        TCommand* to_lower_context_level();
        TCommand* browse_to_previous_context_item();
        TCommand* browse_to_next_context_item();
        TCommand* browse_to_context_item_above();
        TCommand* browse_to_context_item_below();
        TCommand* browse_to_time_line();
        TCommand* goto_begin();
        TCommand* goto_end();
        TCommand* play_to_begin();
        TCommand* play_to_end();
        TCommand* play_cursor_move();
	TCommand* work_cursor_move();
	TCommand* add_marker();
        TCommand* add_marker_at_playhead();
        TCommand* add_marker_at_work_cursor();
        TCommand* playhead_to_workcursor();
        TCommand* workcursor_to_playhead();
	TCommand* center_playhead();
        TCommand* toggle_expand_all_tracks(int height = -1);
        TCommand* edit_properties();
	
private slots:
	void scale_factor_changed();
        void add_new_track_view(Track*);
        void remove_track_view(Track*);
	void update_shuttle();
	void sheet_mode_changed();
	void hscrollbar_value_changed(int);
	void hscrollbar_action(int);
        void session_vertical_scrollbar_position_changed();
        void session_horizontal_scrollbar_position_changed();
};


#endif
