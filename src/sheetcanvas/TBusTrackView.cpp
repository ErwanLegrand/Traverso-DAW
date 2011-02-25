/*
Copyright (C) 2010 Remon Sijrier

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


#include "TBusTrackView.h"

#include "TBusTrack.h"

#include "TrackPanelView.h"
#include "SheetView.h"

#include <Debugger.h>

TBusTrackView::TBusTrackView(SheetView* sv, TBusTrack* group)
        : TrackView(sv, group)
{
        PENTERCONS;

        m_sv = sv;

        load_theme_data();

        m_panel = new TBusTrackPanelView(this);
        calculate_bounding_rect();

	automation_visibility_changed();
}

void TBusTrackView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
        Q_UNUSED(widget);

        TrackView::paint(painter, option, widget);

// 	printf("TrackView:: PAINT :: exposed rect is: x=%f, y=%f, w=%f, h=%f\n", option->exposedRect.x(), option->exposedRect.y(), option->exposedRect.width(), option->exposedRect.height());

        int xstart = (int)option->exposedRect.x();
        int pixelcount = (int)option->exposedRect.width();

//        if (m_paintBackground) {
                QColor color = themer()->get_color("BusTrack:background");
                painter->fillRect(xstart, m_topborderwidth, pixelcount+1, m_sv->get_track_height(m_track) - m_bottomborderwidth, color);
//        }
}

void TBusTrackView::load_theme_data()
{
        m_paintBackground = themer()->get_property("Track:paintbackground").toInt();
        m_topborderwidth = themer()->get_property("Track:topborderwidth").toInt();
        m_bottomborderwidth = themer()->get_property("Track:bottomborderwidth").toInt();
}
