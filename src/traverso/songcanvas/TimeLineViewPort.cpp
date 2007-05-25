/*
    Copyright (C) 2006-2007 Remon Sijrier 
 
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

#include "TimeLineViewPort.h"
#include "SongView.h"
#include "SongWidget.h"
#include "TimeLineView.h"
#include <QScrollBar>
#include <QWheelEvent>
#include <ContextPointer.h>
		
#include <Debugger.h>

TimeLineViewPort::TimeLineViewPort(QGraphicsScene* scene, SongWidget* sw)
	: ViewPort(scene, sw)
{
	setMaximumHeight(TIMELINE_HEIGHT);
	setMinimumHeight(TIMELINE_HEIGHT);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}


void TimeLineViewPort::wheelEvent ( QWheelEvent * e )
{
	if (e->delta() > 0) {
		m_sv->scroll_left();
	} else {
		m_sv->scroll_right();
	}
}

void TimeLineViewPort::set_songview( SongView * view )
{
	m_timeLineView = new TimeLineView(view);
	scene()->addItem(m_timeLineView);
	m_timeLineView->setPos(0, -TIMELINE_HEIGHT);
	m_sv = view;
}

void TimeLineViewPort::get_pointed_context_items(QList<ContextItem* > &list)
{
	QList<QGraphicsItem *> itemlist = items(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
	foreach(QGraphicsItem* item, itemlist) {
		list.append((ViewItem*)item);
	}
	list.append(m_sv);
}

void TimeLineViewPort::scale_factor_changed()
{
	m_timeLineView->calculate_bounding_rect();
}


//eof

