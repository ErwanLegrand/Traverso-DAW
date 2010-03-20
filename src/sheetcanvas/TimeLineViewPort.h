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
 
    $Id: TimeLineViewPort.h,v 1.2 2009/01/19 21:07:19 r_sijrier Exp $
*/

#ifndef TIME_LINE_VIEW_PORT_H
#define TIME_LINE_VIEW_PORT_H

#include "ViewPort.h"

class SheetWidget;
class SheetView;
class TimeLineView;

#define TIMELINE_HEIGHT 32
		
class TimeLineViewPort : public ViewPort
{
public:
	TimeLineViewPort(QGraphicsScene* scene, SheetWidget* sw);
	~TimeLineViewPort();
	
	void set_sheetview(SheetView* view);
	void scale_factor_changed();
	TimeLineView* get_timeline_view() const {return m_timeLineView;}
	
	void get_pointed_context_items(QList<ContextItem* > &list);

protected:
	void wheelEvent ( QWheelEvent * e );

private:
	TimeLineView* 	m_timeLineView;
};

#endif

//eof
