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

$Id: CurveView.cpp,v 1.1 2006/11/28 14:06:12 r_sijrier Exp $
*/

#include "CurveView.h"
		
		
#include <Curve.h>

#include <Debugger.h>
		
CurveView::CurveView(SongView* sv, ViewItem* parentViewItem, Curve* curve)
	: ViewItem(parentViewItem, curve)
	, m_curve(curve)
{
	m_sv = sv;
	m_boundingRectangle = parentViewItem->boundingRect();
}

CurveView::~ CurveView( )
{
}

void CurveView::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	int xstart = (int) option->exposedRect.x();
	int pixelcount = (int) option->exposedRect.width();
	
	printf("painting curveview\n");
	painter->setPen(QColor(Qt::red));
// 	painter->drawLine(xstart, 50, xstart + pixelcount, 50);
} 

//eof
