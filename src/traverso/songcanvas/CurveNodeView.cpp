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

$Id: CurveNodeView.cpp,v 1.1 2006/11/28 14:06:12 r_sijrier Exp $
*/

#include "CurveNodeView.h"

#include <CurveNode.h>

#include <Debugger.h>

CurveNodeView::CurveNodeView( SongView * sv, ViewItem * parentViewItem, CurveNode * node )
	: ViewItem(parentViewItem, 0)
	, m_node(node)
{
	m_sv = sv;
	m_boundingRectangle = parentViewItem->boundingRect();
}

CurveNodeView::~ CurveNodeView( )
{
}

void CurveNodeView::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
} 

//eof
