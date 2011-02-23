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

#ifndef TABSTRACTTRACKVIEW_H
#define TABSTRACTTRACKVIEW_H

#include "ViewItem.h"

class Track;
class TrackPanelView;

class TAbstractTrackView : public ViewItem
{
	Q_OBJECT

public:
	TAbstractTrackView(ViewItem* parentView, ContextItem* parentContext);
	~TAbstractTrackView() {}

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) = 0;

	Track* get_track() const {return m_track;}
	TrackPanelView* get_panel_view() const {return m_panel;}

	virtual void move_to(int x, int y);
	void set_moving(bool move);
	bool is_moving() const {return m_isMoving;}

protected:
	Track*                  m_track;
	TrackPanelView*		m_panel;
	int			m_height;
	int			m_paintBackground;
	int			m_topborderwidth;
	int			m_bottomborderwidth;
	bool                    m_isMoving;
};

#endif // TABSTRACTTRACKVIEW_H
