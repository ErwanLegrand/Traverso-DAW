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

#ifndef LINE_VIEW_H
#define LINE_VIEW_H

#include "ViewItem.h"

class LineView : public ViewItem
{
	Q_OBJECT
public:
		
	LineView(ViewItem* parent) : ViewItem(parent, 0) 
	{
		setZValue(parent->zValue() + 1);
		m_boundingRect = QRectF(0, 0, 1, parent->boundingRect().height());
	}
	void set_bounding_rect(QRectF rect) {m_boundingRect = rect;}
	void set_color(QColor color) {m_color = color;}
	void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) 
	{
		painter->setPen(m_color);
		QLineF line(0, 0, 0, m_boundingRect.height());
		painter->drawLine(line);
	}
	
private:
	QColor m_color;
};

#endif
