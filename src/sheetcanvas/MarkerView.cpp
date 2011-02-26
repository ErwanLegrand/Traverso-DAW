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
#include "SheetView.h"
#include "LineView.h"
#include "Themer.h"
#include "ClipsViewPort.h"
#include "PositionIndicator.h"
#include "MoveMarker.h"

#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <Sheet.h>
#include <Marker.h>
#include <Utils.h>
#include <QDebug>

#include "Debugger.h"

MarkerView::MarkerView(Marker* marker, SheetView* sv, ViewItem* parentView)
	: ViewItem(parentView, marker)
	, m_dragging(false)
{
	PENTERCONS2;
	m_sv = sv;
	m_marker = marker;
	m_line = new LineView(this);
	m_posIndicator = 0;

	QFontMetrics fm(themer()->get_font("Timeline:marker"));
	m_ascent = fm.ascent();
	m_width = fm.width("NI"); // use any two letters to set the width of the marker indicator
	m_line->setPos(m_width / 2, m_ascent);
	
	load_theme_data();
	
	
	connect(m_marker, SIGNAL(positionChanged()), this, SLOT(update_position()));
	connect(m_marker, SIGNAL(descriptionChanged()), this, SLOT(update_drawing()));
	connect(m_marker, SIGNAL(indexChanged()), this, SLOT(update_drawing()));
        connect(this, SIGNAL(activeContextChanged()), this, SLOT(active_context_changed()));
}

void MarkerView::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	painter->save();
	
	int xstart = (int)option->exposedRect.x();
	int clipx = (int)mapToParent(xstart, 0).x();
	if (clipx < 0) {
		painter->setClipRect(-clipx, 0, (int)m_boundingRect.width(), (int)m_boundingRect.height());
	}

	painter->setRenderHint(QPainter::Antialiasing);
	painter->setFont(themer()->get_font("Timeline:fontscale:marker"));
	
	painter->setBrush(m_fillColor);
	painter->setPen(themer()->get_color("Marker:outline"));

	const QPointF pts[3] = {
			QPointF(0, 0),
			QPointF(m_width + 0.5, 0),
			QPointF((m_width+ 0.5)/2, m_ascent) };

	painter->drawPolygon(pts, 3);

	painter->setPen(themer()->get_color("Timeline:text"));

	if (m_marker->get_type() == Marker::ENDMARKER) {
		painter->drawText(m_width + 1, m_ascent-2, m_marker->get_description());
	} else {
		if (m_marker->get_description().length() > 0) {
			painter->drawText(m_width + 1, m_ascent-2, QString("%1: %2").arg(m_marker->get_index()).arg(m_marker->get_description()));
		} else {
			painter->drawText(m_width + 1, m_ascent-2, QString("%1").arg(m_marker->get_index()));
		}
	}

	if (m_dragging) {
                m_posIndicator->set_value(timeref_to_text(m_marker->get_when(), m_sv->timeref_scalefactor));
	}

	painter->restore();
}

void MarkerView::calculate_bounding_rect()
{
	prepareGeometryChange();
	update_position();

	QString desc;
	if (m_marker->get_type() == Marker::ENDMARKER) {
		desc = m_marker->get_description();
	} else {
		desc = QString("%1: %2").arg(m_marker->get_index()).arg(m_marker->get_description());
	}


	QFontMetrics fm(themer()->get_font("Timeline:fontscale:marker"));
	int descriptionwidth = fm.width(desc) + 1;

	m_line->set_bounding_rect(QRectF(0, 0, 1, m_sv->get_clips_viewport()->sceneRect().height()));
	m_line->setPos(m_width / 2, m_ascent);
	m_boundingRect = QRectF(-1, 0, m_width + descriptionwidth, m_ascent);
}

void MarkerView::update_position()
{
	// markerwidth / 2 == center of markerview !
	setPos((m_marker->get_when() / m_sv->timeref_scalefactor) - (m_width / 2), 0);
}

int MarkerView::position()
{
	return (int)(pos().x() + m_width / 2);
}

void MarkerView::set_position(int i)
{
	setPos(i - m_width / 2, 0);
}

void MarkerView::load_theme_data()
{
	if (m_marker->get_type() == Marker::ENDMARKER) {
		m_fillColor = themer()->get_color("Marker:end");
	} else {
		m_fillColor = themer()->get_color("Marker:default");
	}
	calculate_bounding_rect();
}

void MarkerView::set_active(bool b)
{
	m_active = b;

	if (b) {
		if (m_marker->get_type() == Marker::ENDMARKER) {
			m_fillColor = themer()->get_color("Marker:blinkend");
		} else {
			m_fillColor = themer()->get_color("Marker:blink");
		}
		m_line->set_color(themer()->get_color("Marker:line:active"));
	} else {
		if (m_marker->get_type() == Marker::ENDMARKER) {
			m_fillColor = themer()->get_color("Marker:end");
		} else {
			m_fillColor = themer()->get_color("Marker:default");
		}
		m_line->set_color(themer()->get_color("Marker:line:inactive"));
	}

        set_dragging(b);
        m_line->update();
	update();
}

void MarkerView::update_drawing()
{
	calculate_bounding_rect();
	update();
}

void MarkerView::set_dragging(bool dragging)
{
        if (! m_posIndicator) {
                m_posIndicator = new PositionIndicator(this);
        }

        if (dragging) {
                m_posIndicator->set_position(- (m_posIndicator->boundingRect().width() + 4), 0);
                m_posIndicator->show();
        } else {
                m_posIndicator->hide();
	}
	
	m_dragging = dragging;
}

void MarkerView::active_context_changed()
{
        if(has_active_context()) {
                set_active(true);
        } else {
                set_active(false);
        }
}

TCommand* MarkerView::drag_marker()
{
        return new MoveMarker(this, m_sv->timeref_scalefactor, tr("Drag Marker"));
}
