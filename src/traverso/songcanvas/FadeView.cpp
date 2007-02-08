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

$Id: FadeView.cpp,v 1.6 2007/02/08 20:51:38 r_sijrier Exp $
*/

#include "FadeView.h"

#include <QPainter>

#include "FadeCurve.h"
#include "AudioClipView.h"
#include "FadeContextDialog.h"
#include "SongView.h"
#include <Themer.h>

#include "Song.h"
#include <InputEngine.h>

#include <Debugger.h>

static const int DOT_SIZE		= 6;
static const QString DOT_COLOR		= "#78817B";


FadeView::FadeView(SongView* sv, AudioClipView* parent, FadeCurve * fadeCurve )
	: ViewItem(parent, fadeCurve)
	, m_clipView(parent)
	, m_fadeCurve(fadeCurve)
{
	PENTERCONS;
	m_dialog = 0;
	m_sv = sv;
	
	calculate_bounding_rect();	
	setAcceptsHoverEvents(true);
	
	connect(m_fadeCurve, SIGNAL(stateChanged()), this, SLOT(state_changed()));
	connect(m_fadeCurve, SIGNAL(rangeChanged()), this, SLOT(state_changed()));
}


FadeView::~ FadeView( )
{
	PENTERDES;
}


void FadeView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QPolygonF polygon;
	float value[2];
	int scaleFactor = m_sv->scalefactor;
	int xstart = (int) option->exposedRect.x();
	int pixelcount = (int) option->exposedRect.width();
	
	// Populate the polygon with enough points to draw a smooth curve.
	// We add 1 to position and xstart to compensate for an off by 1 pixel offset.
	// Don't know exactly why this offset is there....
	int position = xstart;
	do {
		m_fadeCurve->get_vector(position*scaleFactor, position*scaleFactor + 1, value, 2);
	 	polygon << QPointF(position + 1, m_clipView->get_height() - (value[1] * m_clipView->get_height()) );
// 		printf("x, y: %d, %f \n", position, m_clipView->get_height() - (value[1] * m_clipView->get_height()));
	 	position ++;
	} while (position <= (xstart + pixelcount));
	
	
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);
	
	QPainterPath path;
	
	path.addPolygon(polygon);
	path.lineTo(xstart + 1 + pixelcount, 0);
	path.lineTo(xstart + 1, 0);
	path.closeSubpath();
	
	painter->setPen(Qt::NoPen);
	
	QColor color = m_fadeCurve->is_bypassed() ? themer()->get_color("Fade:bypassed") : themer()->get_color("Fade:default");
	
	if (option->state & QStyle::State_MouseOver) {
		color.setAlpha(color.alpha() + 10);
	}
	
	painter->setBrush(color);
	painter->drawPath(path);	


	if ( (option->state & QStyle::State_MouseOver) && ie().is_holding()) {
		// Calculate and draw control points
		int h = (int) m_boundingRectangle.height() - 1;
		int w = (int) m_boundingRectangle.width() - 1;
		QList<QPointF> points = m_fadeCurve->get_control_points();
		QPoint p1(int(points.at(1).x() * w + 0.5), h - int(points.at(1).y() * h + 0.5));
		QPoint p2(w - int((1.0 - points.at(2).x()) * w + 0.5), int((1.0 - points.at(2).y()) * h + 0.5));
	
		painter->setPen(QColor(DOT_COLOR));
		painter->setBrush(QColor(DOT_COLOR));
		
		if (m_fadeCurve->get_fade_type() == FadeCurve::FadeOut) {
			p1.setX(w - int((1 - points.at(2).x()) * w + 0.5));
			p1.setY(h - int((1 - points.at(2).y()) * h + 0.5));
			p2.setX(int((points.at(1).x()) * w + 0.5));
			p2.setY(int((points.at(1).y()) * h + 0.5));
			painter->drawLine(w, h, p1.x(), p1.y());
			painter->drawLine(0, 0, p2.x(), p2.y());
		} else {
			painter->drawLine(0, h, p1.x(), p1.y());
			painter->drawLine(w, 0, p2.x(), p2.y());
		}
		
		painter->drawEllipse(p1.x() - DOT_SIZE/2, p1.y() - DOT_SIZE/2, DOT_SIZE, DOT_SIZE);
		painter->drawEllipse(p2.x() - DOT_SIZE/2, p2.y() - DOT_SIZE/2, DOT_SIZE, DOT_SIZE);
	}
	
	painter->restore();
}


Command* FadeView::edit_properties()
{
	if (!m_dialog) {
		m_dialog = new FadeContextDialog(m_fadeCurve);
	}
	
	m_dialog->show();
	
	return 0;
}


void FadeView::calculate_bounding_rect()
{
	m_boundingRectangle = QRectF(0, 0, m_fadeCurve->get_range() / m_sv->scalefactor, m_clipView->m_height);
	
	if (m_fadeCurve->get_fade_type() == FadeCurve::FadeOut) {
		setPos(m_clipView->boundingRect().width() - m_boundingRectangle.width(), m_clipView->get_fade_y_offset());
	} else {
		setPos(0, m_clipView->get_fade_y_offset());
	}
}


void FadeView::state_changed( )
{
	PENTER;
	prepareGeometryChange();
	calculate_bounding_rect();
}

//eof
