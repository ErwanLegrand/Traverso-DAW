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
#include "PositionIndicator.h"

#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <Song.h>
#include <Marker.h>
#include <Utils.h>
#include <QDebug>

MarkerView::MarkerView(Marker* marker, SongView* sv, ViewItem* parentView)
	: ViewItem(parentView, marker)
	, m_dragging(false)
{
	m_sv = sv;
	m_marker = marker;
	m_active = false;
	m_posIndicator = 0;

	QFontMetrics fm(themer()->get_font("Timeline:marker"));
	m_ascent = fm.ascent();
	m_width = fm.width("NI"); // use any two letters to set the width of the marker indicator

	load_theme_data();
	
	connect(m_marker, SIGNAL(positionChanged(Snappable*)), this, SLOT(update_position()));
	connect(m_marker, SIGNAL(descriptionChanged()), this, SLOT(update_drawing()));
}

void MarkerView::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	painter->save();
	
	int xstart = (int)option->exposedRect.x();
	int clipx = (int)mapToParent(xstart, 0).x();
	if (clipx < 0) {
		painter->setClipRect(-clipx, 0, 30, (int)m_boundingRect.height());
	}

	if (m_active) {
		painter->drawLine(m_width/2, m_ascent, m_width/2, (int)m_boundingRect.height());
	}
	
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setFont(themer()->get_font("Timeline:fontscale:marker"));
	
	painter->setPen(QColor(Qt::black));
	painter->setBrush(m_fillColor);

	const QPointF pts[3] = {
			QPointF(0, 0),
			QPointF(m_width + 0.5, 0),
			QPointF((m_width+ 0.5)/2, m_ascent) };

	painter->drawPolygon(pts, 3);
	painter->drawText(m_width + 1, m_ascent, m_marker->get_description());

	if (m_dragging) {
		m_posIndicator->set_value(frame_to_text( (x() + m_boundingRect.width() / 2 ) * m_sv->scalefactor,
					  m_sv->get_song()->get_rate(), m_sv->scalefactor));
	}

	painter->restore();
}

void MarkerView::calculate_bounding_rect()
{
	prepareGeometryChange();
	update_position();
	
	QFontMetrics fm(themer()->get_font("Timeline:fontscale:marker"));
	int descriptionwidth = fm.width(m_marker->get_description()) + 1;

	if (m_active) {
		m_boundingRect = QRectF(-1, 0, m_width + descriptionwidth,
				m_sv->get_clips_viewport()->sceneRect().height());
	} else {
		m_boundingRect = QRectF(-1, 0, m_width + descriptionwidth, m_ascent);
	}

}

void MarkerView::update_position()
{
	// markerwidth / 2 == center of markerview !
	setPos( (long)(m_marker->get_when() / m_sv->scalefactor) - (m_width / 2), 0);
}

void MarkerView::set_position(int i)
{
	setPos(i - m_width / 2, 0);
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

void MarkerView::update_drawing()
{
	calculate_bounding_rect();
	update();
}

void MarkerView::set_dragging(bool dragging)
{
	if (dragging) {
		if (! m_posIndicator) {
			m_posIndicator = new PositionIndicator(this);
			scene()->addItem(m_posIndicator);
			m_posIndicator->set_position(15, 0);
		}
	} else {
		if (m_posIndicator) {
			scene()->removeItem(m_posIndicator);
			delete m_posIndicator;
			m_posIndicator = 0;
		}
	}
	
	m_dragging = dragging;
}

//eof

