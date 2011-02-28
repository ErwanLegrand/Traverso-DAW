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

#include "TTrackLaneView.h"

#include "Track.h"
#include "CurveView.h"
#include "SheetView.h"

#include "Debugger.h"

TTrackLaneView::TTrackLaneView(ViewItem* parent)
	: ViewItem(parent, 0)
{
	m_sv = parent->get_sheetview();
	m_height = 150;
	m_ignoreContext = true;
	m_panel = 0;
	m_childView = 0;
	setZValue(parent->zValue() + 1);
}

void TTrackLaneView::set_child_view(ViewItem *view)
{
	m_childView = view;

	m_panel = new TTrackLanePanelView(this);
	view->setZValue(zValue());
}

TTrackLaneView::~TTrackLaneView()
{
}


void TTrackLaneView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(widget);

//	int xstart = (int)option->exposedRect.x();
//	int pixelcount = (int)option->exposedRect.width();
}

void TTrackLaneView::set_height(int height)
{
	m_height = height;
	calculate_bounding_rect();
}


void TTrackLaneView::calculate_bounding_rect()
{
	prepareGeometryChange();
	m_boundingRect = QRectF(0, 0, MAX_CANVAS_WIDTH, m_height);
	if (m_panel) {
		m_panel->calculate_bounding_rect();
	}
	ViewItem::calculate_bounding_rect();
}

void TTrackLaneView::load_theme_data()
{
	m_paintBackground = themer()->get_property("TrackLane:paintbackground").toInt();
}

void TTrackLaneView::move_to( int x, int y )
{
	Q_UNUSED(x);
	setPos(0, y);
	if (m_panel) {
		m_panel->setPos(-180, 0);
	}
}

QString TTrackLaneView::get_name() const
{
	if (m_childView) {
		return m_childView->get_name();
	}

	return "";
}
