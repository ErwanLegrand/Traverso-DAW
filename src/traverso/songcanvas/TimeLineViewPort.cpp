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
 
    $Id: TimeLineViewPort.cpp,v 1.1 2006/11/08 14:45:22 r_sijrier Exp $
*/

#include "TimeLineViewPort.h"
#include "ClipsViewPort.h"
#include "SongWidget.h"
#include "TimeLineView.h"
		
#include <Debugger.h>

TimeLineViewPort::TimeLineViewPort(QGraphicsScene* scene, SongWidget* sw, ClipsViewPort* view)
	: ViewPort(scene, sw)
{
	clipView = view;
	
	setMaximumHeight(21);
	setMinimumHeight(21);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}


void TimeLineViewPort::wheelEvent ( QWheelEvent * e )
{
	if (e->delta() > 0)
		clipView->horizontalScrollBar()->setValue(clipView->horizontalScrollBar()->value() - clipView->horizontalScrollBar()->pageStep() / 6);
	else
		clipView->horizontalScrollBar()->setValue(clipView->horizontalScrollBar()->value() + clipView->horizontalScrollBar()->pageStep() / 6);
}

void TimeLineViewPort::set_songview( SongView * view )
{
	m_timeLineView = new TimeLineView(view);
	scene()->addItem(m_timeLineView);
	m_timeLineView->setPos(0, -23);
}

//eof
