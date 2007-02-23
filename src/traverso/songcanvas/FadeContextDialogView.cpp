/*
Copyright (C) 2006 Remon Sijrier, Nicola Doebelin

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

$Id: FadeContextDialogView.cpp,v 1.4 2007/02/23 13:53:13 r_sijrier Exp $
*/

#include "FadeContextDialogView.h"

#include "ViewPort.h"
#include "Themer.h"
#include "FadeCurve.h"
#include "CurveNode.h"
#include "Command.h"
#include <ContextPointer.h>

#include <QPainterPath>

#include <Debugger.h>

static const int GRID_LINES		= 4;
static const float RASTER_SIZE		= 0.05;
static const int DOT_SIZE		= 6;
static const QString DOT_COLOR		= "#78817B";
static const QString INACTIVE_COLOR	= "#A6B2A9";


FadeContextDialogView::FadeContextDialogView(ViewPort* viewPort, FadeCurve* fadeCurve)
	: ViewItem(0, fadeCurve), m_vp(viewPort), m_fade(fadeCurve)
{
	connect(m_vp, SIGNAL(resized()), this, SLOT(resize()));
	connect(m_fade, SIGNAL(stateChanged()), this, SLOT(schedule_for_repaint()));
}


FadeContextDialogView::~FadeContextDialogView()
{}


void FadeContextDialogView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
/*
	// Draw the background pixmap
	painter->drawPixmap(0, 0, background);

	painter->setRenderHint(QPainter::Antialiasing);
	
	// Calculate and draw control points
	int h = m_vp->height() - 1;
	int w = m_vp->width() - 1;
	QList<QPointF> points = m_fade->get_control_points();
	QPoint p1(int(points.at(1).x() * w + 0.5), h - int(points.at(1).y() * h + 0.5));
	QPoint p2(w - int((1.0 - points.at(2).x()) * w + 0.5), int((1.0 - points.at(2).y()) * h + 0.5));

	painter->setPen(QColor(DOT_COLOR));
	painter->setBrush(QColor(DOT_COLOR));
	
	if (m_fade->get_fade_type() == FadeCurve::FadeOut) {
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


	QPolygonF polygon;
	float value[2];
	int step = 0;
	int height = m_vp->height();
	
	int zoom = (int) m_fade->get_range() / m_vp->width();
	
	// Populate the polygon with enough points to draw a smooth curve.
	// using a 2 pixel resolution, is sufficient enough
	for (int i=0; i < m_vp->width(); i+=2) {
		m_fade->get_vector(i * zoom, i * zoom + 1, value, 2);
	 	polygon << QPointF( step, height - (value[1] * height) );
	 	step += 2;
	}

	// Always add the uppermost point in the polygon path 
	// since the above routine potentially does not include it.
	polygon << QPointF(m_vp->width(), 0);
	
	// Draw the curve
	QPainterPath path;
	path.moveTo(0, m_vp->height());
	path.addPolygon(polygon);
	path.lineTo(0, 0);
	path.closeSubpath();
	
	painter->setPen(Qt::NoPen);
	painter->setBrush(QColor(255, 0, 255, 80));
	
	painter->drawPath(path);	
*/
}




void FadeContextDialogView::create_background( )
{
	if (background.size() != m_vp->size()) {
		background = QPixmap(m_vp->width(), m_vp->height());
	}

	QPainter painter(&background);
	
// 	painter->setRenderHint(QPainter::Antialiasing);

	// Background color
	painter.fillRect(0, 0, m_vp->width(), m_vp->height(), themer()->get_color("CLIP_BG_DEFAULT"));

	// grid lines
	painter.setPen(QColor(INACTIVE_COLOR));
	for (int i = 0; i <= GRID_LINES; i++) {
		float d = (float)i / (float)GRID_LINES;
		int x = (int)(m_vp->width() * d);
		int y = (int)(m_vp->height() * d);
		painter.drawLine(x, 0, x, m_vp->height());
		painter.drawLine(0, y, m_vp->width(), y);
	}

}



/******** SLOTS ***************/

void FadeContextDialogView::resize( )
{
	create_background();
}




//eof
