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

$Id: CurveNodeView.cpp,v 1.5 2007/02/08 20:51:38 r_sijrier Exp $
*/

#include "CurveNodeView.h"
#include "SongView.h"

#include <QPainter>
#include <QPen>
#include <CurveNode.h>
#include <Themer.h>

#include <Debugger.h>

CurveNodeView::CurveNodeView( SongView * sv, ViewItem * parentViewItem, CurveNode * node )
	: ViewItem(parentViewItem, 0)
	, m_node(node)
{
	PENTERCONS;
	m_sv = sv;
	m_boundingRectangle = QRectF(0, 0, 6, 6);
	load_theme_data();
	// This actually calculates the position..
	calculate_bounding_rect();
	
	connect(m_node, SIGNAL(positionChanged()), this, SLOT(update_pos()));
}

CurveNodeView::~ CurveNodeView( )
{
	PENTERDES;
}

void CurveNodeView::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	QPen pen;
	pen.setColor(m_color);
	pen.setWidth(1);
	
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setPen(pen);
	QPainterPath path;
	path.addEllipse(m_boundingRectangle);
	painter->fillPath(path, QBrush(m_color));
} 


void CurveNodeView::calculate_bounding_rect()
{
	update_pos();
}


void CurveNodeView::set_color(QColor color)
{
	m_color = color;
}

void CurveNodeView::update_pos( )
{
	setPos( (m_node->get_when() / m_sv->scalefactor) - (m_boundingRectangle.width() / 2),
	 	parentItem()->boundingRect().height() - (m_node->get_value() * parentItem()->boundingRect().height() + m_boundingRectangle.height() / 2 ));
}

void CurveNodeView::increase_size( )
{
	m_boundingRectangle.setWidth(m_boundingRectangle.width() + 1);
	m_boundingRectangle.setHeight(m_boundingRectangle.height() + 1);
	update_pos();
}

void CurveNodeView::decrease_size( )
{
	m_boundingRectangle.setWidth(m_boundingRectangle.width() - 1);
	m_boundingRectangle.setHeight(m_boundingRectangle.height() - 1);
	update_pos();
}

void CurveNodeView::load_theme_data()
{
	m_color = themer()->get_color("CurveNode:default");
	update();
}

//eof
