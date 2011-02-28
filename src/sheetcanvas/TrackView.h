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

#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

#include "ViewItem.h"

class AudioClip;
class AudioTrack;
class PluginChainView;
class CurveView;
class Track;
class TrackPanelView;
class TTrackLaneView;


class TrackView : public ViewItem
{
        Q_OBJECT

public:
	TrackView(SheetView* sv, Track* track);
	TrackView(ViewItem* parentView, Track* track);
	~TrackView();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        Track* get_track() const {return m_track;}
        TrackPanelView* get_panel_view() const {return m_panel;}
	TTrackLaneView* get_primary_lane_view() const {return m_primaryLaneView;}

        void set_moving(bool move);
	void move_to(int x, int y);
        bool is_moving() const {return m_isMoving;}
	virtual int get_height();
	virtual void set_height(int height);
	int get_total_height();
	void layout_lanes();

        void calculate_bounding_rect();
        void load_theme_data();

protected:
	TTrackLaneView*		m_primaryLaneView;
	TTrackLaneView*		m_volumeAutomationLaneView;
	PluginChainView*	m_pluginChainView;
	CurveView*              m_curveView;
	Track*                  m_track;
	TrackPanelView*		m_panel;
	int			m_height;
	int			m_paintBackground;
	int			m_topborderwidth;
	int			m_bottomborderwidth;
	bool                    m_isMoving;

	void add_lane_view(TTrackLaneView* laneView);

        friend class TrackPanelView;
        friend class AudioTrackPanelView;
        friend class TBusTrackPanelView;

private:
	QList<TTrackLaneView*>	m_laneViews;
	int	m_laneSpacing;
	int	m_cliptopmargin;
	int	m_clipbottommargin;
	int	m_visibleLanes;

public slots:
        TCommand* edit_properties();
        TCommand* add_new_plugin();

protected slots:
	virtual void automation_visibility_changed();

private slots:
        void active_context_changed() {update();}

signals:
	void totalTrackHeightChanged();
};


#endif // TRACK_VIEW_H

//eof
