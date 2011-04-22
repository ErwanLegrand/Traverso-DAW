/*
    Copyright (C) 2007 Remon Sijrier 
 
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

#include "PositionIndicator.h"

#include "SheetView.h"

#include <QColor>
#include "Utils.h"
#include "Themer.h"

PositionIndicator::PositionIndicator(ViewItem* parentView)
	: ViewItem(parentView, 0)
{
	calculate_bounding_rect();
	setZValue(200);
}

void PositionIndicator::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	painter->drawPixmap(0, 0, m_background);
	painter->setPen(Qt::black);
	painter->setFont(themer()->get_font("TrackPanel:fontscale:name"));
        painter->drawText(m_boundingRect, Qt::AlignHCenter, m_value);
}

void PositionIndicator::calculate_bounding_rect()
{
	prepareGeometryChange();
	m_boundingRect = QRectF(0, 0, 70, 14);
	
	m_background = QPixmap((int)m_boundingRect.width(), (int)m_boundingRect.height());
	m_background.fill(QColor(Qt::transparent));
	
	QPainter painter(&m_background);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(QColor(255, 255, 255, 200));
	painter.setPen(Qt::NoPen);
	int rounding = m_boundingRect.height() / 2;
	painter.drawRoundedRect(m_boundingRect, rounding, rounding);
}

void PositionIndicator::set_value(const QString & value)
{
	m_value = value;
	update();
}

