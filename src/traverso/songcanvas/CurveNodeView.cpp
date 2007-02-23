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

$Id: CurveNodeView.cpp,v 1.7 2007/02/23 13:54:33 r_sijrier Exp $
*/

#include "CurveNodeView.h"
#include "SongView.h"

#include <QPainter>
#include <QPen>
#include <CurveNode.h>
#include <Themer.h>
#include <Curve.h>

#include <Debugger.h>

CurveNodeView::CurveNodeView( SongView * sv, ViewItem * parentViewItem, CurveNode * node, Curve* guicurve)
	: ViewItem(parentViewItem, 0)
	, CurveNode(guicurve, node->get_when(), node->get_value())
	, m_node(node)
{
	PENTERCONS;
	m_sv = sv;
	m_boundingRect = QRectF(0, 0, 6, 6);
	load_theme_data();
	
	connect(m_node->m_curve, SIGNAL(nodePositionChanged()), this, SLOT(update_pos()));
}

CurveNodeView::~ CurveNodeView( )
{
	PENTERDES;
}

void CurveNodeView::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	painter->save();
	
	QPen pen;
	pen.setColor(m_color);
	
	QPointF mapped = mapToParent(QPointF(0, 0));
	int x = (int) mapped.x();
	int y = (int) mapped.y();
	int heightadjust = 0;
	int widthadjust = 0;
	
	if ( (y + m_boundingRect.height()) > (int) m_parentViewItem->boundingRect().height() ) {
		heightadjust = y - (int)m_parentViewItem->boundingRect().height() + (int) m_boundingRect.height();
	}
	
	if ( (x + m_boundingRect.width()) > (int) m_parentViewItem->boundingRect().width() ) {
		widthadjust = x - (int) m_parentViewItem->boundingRect().width() + (int) m_boundingRect.width();
	}
		
// 	printf("widthadjust is %d, heightadjust = %d\n", widthadjust, heightadjust);
	if (x > 0) x = 0;
	if (y > 0) y = 0;
	
	QRectF rect = m_boundingRect.adjusted(- x, - y, - widthadjust, -heightadjust);
	painter->setClipRect(rect);
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setPen(pen);
	
	QPainterPath path;
	path.addEllipse(m_boundingRect);
	
	painter->fillPath(path, QBrush(m_color));
	
	painter->restore();
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
	setPos( (m_node->get_when() / m_sv->scalefactor) - (m_boundingRect.width() / 2),
	 	m_parentViewItem->boundingRect().height() - (m_node->get_value() * m_parentViewItem->boundingRect().height() + m_boundingRect.height() / 2 ));
		
	set_when_and_value((m_node->get_when() / m_sv->scalefactor), m_node->get_value());
}

void CurveNodeView::increase_size( )
{
	m_boundingRect.setWidth(m_boundingRect.width() + 1);
	m_boundingRect.setHeight(m_boundingRect.height() + 1);
	update_pos();
}

void CurveNodeView::decrease_size( )
{
	m_boundingRect.setWidth(m_boundingRect.width() - 1);
	m_boundingRect.setHeight(m_boundingRect.height() - 1);
	update_pos();
}

void CurveNodeView::load_theme_data()
{
	m_color = themer()->get_color("CurveNode:default");
	calculate_bounding_rect();
}

//eof
