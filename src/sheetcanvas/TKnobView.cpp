/*
Copyright (C) 2011 Remon Sijrier

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

#include "TKnobView.h"

#include "Track.h"
#include "math.h"

TKnobView::TKnobView(ViewItem *parent)
	: ViewItem(parent, 0)
{
	m_boundingRect = QRectF(0, 0, 22, 22);

	m_minValue = -1.0f;
	m_maxValue = 1.0f;
	m_totalAngle = 270;
}

void TKnobView::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);

	int borderWidth = 3;

	painter->setRenderHint(QPainter::Antialiasing);

	QPen pen(QColor(100, 100, 100, 220));
	pen.setWidth(1);
	painter->setPen(pen);
	painter->setBrush(QColor(Qt::transparent));

	int radius;
	double rb, re;
	double rarc;

	rarc = m_angle * M_PI / 180.0;
	double ca = cos(rarc);
	double sa = -sin(rarc);
	radius = m_boundingRect.width() / 2 - borderWidth;
	if (radius < 3) radius = 3;
	int ym = m_boundingRect.y() + radius + borderWidth;
	int xm = m_boundingRect.x() + radius + borderWidth;

	int penWidth = 2;
	pen.setWidth(penWidth);
	painter->setPen(pen);

	rb = qMax(double((radius - penWidth) / 3.0), 0.0);
	re = qMax(double(radius - penWidth), 0.0);

	QPoint center;
	center.setX(m_boundingRect.width() / 2);
	center.setY(m_boundingRect.height() / 2);
	painter->drawEllipse(center, radius, radius);

	painter->drawLine(xm - int(rint(sa * rb)),
			ym - int(rint(ca * rb)),
			xm - int(rint(sa * re)),
			ym - int(rint(ca * re)));

	QFont font = themer()->get_font("TrackPanel:fontscale:name");
	font.setPixelSize(8);
	painter->setFont(font);
	painter->drawText(m_boundingRect.width() / 2 - 3, -1, "0");
	font.setPixelSize(7);
	painter->setFont(font);
	painter->drawText(0, m_boundingRect.height() + 3, "L");
	painter->drawText(m_boundingRect.width() - 3, m_boundingRect.height() + 3, "R");

}

void TKnobView::set_width(int width)
{
	m_boundingRect = QRectF(0, 0, width, 9);
	load_theme_data();
}

void TKnobView::load_theme_data()
{
	m_gradient2D.setColorAt(0.0, themer()->get_color("PanSlider:-1"));
	m_gradient2D.setColorAt(0.5, themer()->get_color("PanSlider:0"));
	m_gradient2D.setColorAt(1.0, themer()->get_color("PanSlider:1"));
	m_gradient2D.setStart(QPointF(m_boundingRect.width() - 40, 0));
	m_gradient2D.setFinalStop(31, 0);
}


void TKnobView::update_angle()
{
	m_angle = (get_value() - 0.5 * (min_value() + max_value()))
			/ (max_value() - min_value()) * m_totalAngle;
	m_nTurns = floor((m_angle + 180.0) / 360.0);
	m_angle = m_angle - m_nTurns * 360.0;

}


TPanKnobView::TPanKnobView(ViewItem* parent, Track* track)
	: TKnobView(parent)
	, m_track(track)
{
	connect(m_track, SIGNAL(panChanged()), this, SLOT(track_pan_changed()));
	update_angle();
}

double TPanKnobView::get_value() const
{
	return m_track->get_pan();
}

void TPanKnobView::track_pan_changed()
{
	update_angle();
	update();
}

TCommand* TPanKnobView::pan_left()
{
	m_track->set_pan(m_track->get_pan() - 0.05);
	return 0;
}

TCommand* TPanKnobView::pan_right()
{
	m_track->set_pan(m_track->get_pan() + 0.05);
	return 0;
}
