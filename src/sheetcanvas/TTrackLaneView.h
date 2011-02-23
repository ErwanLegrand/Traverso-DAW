/*
Copyright (C) 2011 Remon Sijrier

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

#ifndef TTRACKLANEVIEW_H
#define TTRACKLANEVIEW_H

#include "TrackView.h"
#include "TrackPanelView.h"

class Track;
class TAutomationTrackPanelView;

class TTrackLaneView : public TAbstractTrackView
{
	Q_OBJECT

public:
	TTrackLaneView(ViewItem* parentView, Track* track);
	~TTrackLaneView();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	int get_height() { return m_height;}
	void set_height(int height);
	void set_curve_view(CurveView* view);
	QString get_name() const;

	void move_to(int x, int y);
	void calculate_bounding_rect();
	void load_theme_data();

private:
	TAutomationTrackPanelView*	m_lanePanel;
	QString		m_name;
	CurveView*	m_curveView;
};

#endif // TTRACKLANEVIEW_H
