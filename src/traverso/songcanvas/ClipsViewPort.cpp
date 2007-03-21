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

$Id: ClipsViewPort.cpp,v 1.9 2007/03/21 15:11:34 r_sijrier Exp $
*/

#include "ClipsViewPort.h"

#include "SongWidget.h"
#include "SongView.h"
#include "Cursors.h"

#include <ContextPointer.h>
#include <QScrollBar>
#include <QSet>
#include <QPaintEngine>
		
#include <Debugger.h>
		
ClipsViewPort::ClipsViewPort(QGraphicsScene* scene, SongWidget* sw)
	: ViewPort(scene, sw)
{

	m_sw = sw;
	viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
	
// 	setViewportUpdateMode(SmartViewportUpdate);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void ClipsViewPort::get_pointed_context_items(QList<ContextItem* > &list)
{
	printf("ClipsViewPort::get_pointed_view_items\n");
	QList<QGraphicsItem *> itemlist = items(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
	foreach(QGraphicsItem* item, itemlist) {
		list.append((ViewItem*)item);
	}
	list.append(m_sw->get_songview());
}

void ClipsViewPort::resizeEvent( QResizeEvent * e )
{
	ViewPort::resizeEvent(e);
	m_sw->get_songview()->update_scrollbars();
}


void ClipsViewPort::paintEvent(QPaintEvent * e)
{
// 	printf("ClipsViewPort::paintEvent\n");
	QGraphicsView::paintEvent(e);
}

//eof
