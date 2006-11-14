/*
Copyright (C) 2006 Remon Sijrier 

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

$Id: ClipsViewPort.cpp,v 1.2 2006/11/14 14:59:07 r_sijrier Exp $
*/

#include "ClipsViewPort.h"

#include "SongWidget.h"
#include "SongView.h"

#include <ContextPointer.h>
		
#include <Debugger.h>
		
ClipsViewPort::ClipsViewPort(QGraphicsScene* scene, SongWidget* sw)
	: ViewPort(scene, sw)
{
	setBackgroundBrush(QBrush(QColor("#FBFCFF")));
	m_sw = sw;
}

void ClipsViewPort::get_pointed_view_items( QList< ViewItem * > & list )
{
	printf("ClipsViewPort::get_pointed_view_items\n");
	QList<QGraphicsItem *> itemlist = items(cpointer().x(), cpointer().y());
	foreach(QGraphicsItem* item, itemlist) {
		list.append((ViewItem*)item);
	}
	list.append(m_sw->m_songView);
	
	printf("itemlist size is %d\n", itemlist.size());
}

void ClipsViewPort::resizeEvent( QResizeEvent * e )
{
	ViewPort::resizeEvent(e);
	m_sw->m_songView->set_snap_range(horizontalScrollBar()->value());
}


//eof
