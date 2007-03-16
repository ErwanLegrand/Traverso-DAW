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

#include "MarkerView.h"
#include "SongView.h"
#include "Themer.h"
#include "ClipsViewPort.h"

#include <QColor>
#include <Marker.h>
#include <Utils.h>
#include <QDebug>

#define MARKER_WIDTH 10

MarkerView::MarkerView(Marker* marker, SongView* sv, ViewItem* parentView)
	: ViewItem(parentView, marker)
{
	m_sv = sv;
	m_marker = marker;
	m_active = false;

	load_theme_data();
	
	connect(m_marker, SIGNAL(positionChanged()), this, SLOT(update_position()));
}

void MarkerView::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	painter->save();

	painter->setRenderHint(QPainter::Antialiasing);
	painter->setFont( QFont( "Bitstream Vera Sans", 7) );
	
	painter->setPen(QColor("#000000"));
	painter->setBrush(m_fillColor);

	const QPointF pts[3] = {
			QPointF(0, 0),
			QPointF(MARKER_WIDTH, 0),
			QPointF(MARKER_WIDTH/2, 9) };

	painter->drawPolygon(pts, 3);
	painter->drawText(14, 8, m_marker->get_description());

	if (m_active) {
		painter->drawLine(MARKER_WIDTH/2, 9, MARKER_WIDTH/2, m_boundingRect.height());
	}

	painter->restore();
}

void MarkerView::calculate_bounding_rect()
{
	prepareGeometryChange();
	update_position();
	
	QFontMetrics fm( QFont( "Bitstream Vera Sans", 7) );
	int descriptionwidth = fm.width(m_marker->get_description()) + 1;

	if (m_active) {
		m_boundingRect = QRectF(0, 0, MARKER_WIDTH + descriptionwidth,
				m_sv->get_clips_viewport()->sceneRect().height());
	} else {
		m_boundingRect = QRectF(0, 0, MARKER_WIDTH + descriptionwidth, 9.0);
	}

}

void MarkerView::update_position()
{
	// markerwidth / 2 == center of markerview !
	setPos(m_marker->get_when() / m_sv->scalefactor - MARKER_WIDTH / 2, 0);
}

void MarkerView::set_position(int i)
{
	setPos(i - MARKER_WIDTH / 2, 0);
}

void MarkerView::load_theme_data()
{
	m_fillColor = themer()->get_color("Marker:default");
	calculate_bounding_rect();
}

void MarkerView::set_active(bool b)
{
	m_active = b;

	if (b) {
		m_fillColor = themer()->get_color("Marker:blink");
	} else {
		m_fillColor = themer()->get_color("Marker:default");
	}

	calculate_bounding_rect();
	update();
}



//eof
