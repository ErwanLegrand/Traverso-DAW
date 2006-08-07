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

$Id: FadeContextDialogView.cpp,v 1.5 2006/08/07 21:33:05 r_sijrier Exp $
*/

#include "FadeContextDialogView.h"

#include "ViewPort.h"
#include "ColorManager.h"
#include "FadeCurve.h"
#include "CurveNode.h"

#include <QPainterPath>

#include <Debugger.h>

static const int GRID_LINES		= 4;
static const float CURSOR_SPEED		= 150.0;
static const float RASTER_SIZE		= 0.05;
static const int DOT_SIZE		= 6;
static const QString DOT_COLOR		= "#78817B";
static const QString INACTIVE_COLOR	= "#A6B2A9";


FadeContextDialogView::FadeContextDialogView(ViewPort* viewPort, FadeCurve* fadeCurve)
		: ViewItem(viewPort, 0, fadeCurve), m_vp(viewPort), m_fade(fadeCurve)
{
	m_vp->register_viewitem(this);
	init_context_menu( this );
	
	connect(m_vp, SIGNAL(resized()), this, SLOT(resize()));
	connect(m_fade, SIGNAL(stateChanged()), this, SLOT(schedule_for_repaint()));
}


FadeContextDialogView::~FadeContextDialogView()
{}


QRect FadeContextDialogView::draw( QPainter& p )
{

	// Draw the background pixmap
	p.drawPixmap(0, 0, background);

	p.setRenderHint(QPainter::Antialiasing);
	
	// Calculate and draw control points
	float h = m_vp->height() - 1;
	QList<QPointF> points = m_fade->get_control_points();
	QPoint p1(int(points.at(1).x() * m_vp->width()), m_vp->height()-1 - int(points.at(1).y() * h));
	QPoint p2(int(m_vp->width()-1 - (1.0 - points.at(2).x()) * (m_vp->width()-1)), int(float(1.0 - points.at(2).y()) * h));

	p.setPen(QColor(DOT_COLOR));
	p.setBrush(QColor(DOT_COLOR));
	
	if (m_fade->get_fade_type() == FadeCurve::FadeOut) {
		p1.setX(m_vp->width() - p1.x() - 1);
		p2.setX(m_vp->width() - p2.x() - 1);
		p.drawLine(m_vp->width()-1, m_vp->height()-1, p1.x(), p1.y());
		p.drawLine(0, 0, p2.x(), p2.y());
	} else {
		p.drawLine(0, m_vp->height()-1, p1.x(), p1.y());
		p.drawLine(m_vp->width()-1, 0, p2.x(), p2.y());
	}
	
	p.drawEllipse(p1.x() - DOT_SIZE/2, p1.y() - DOT_SIZE/2, DOT_SIZE, DOT_SIZE);
	p.drawEllipse(p2.x() - DOT_SIZE/2, p2.y() - DOT_SIZE/2, DOT_SIZE, DOT_SIZE);


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
	
	p.setPen(Qt::NoPen);
	p.setBrush(QColor(255, 0, 255, 80));
	
	p.drawPath(path);	

	return QRect();
}


float FadeContextDialogView::roundFloat( float f)
{
	return float(int(0.5 + f / RASTER_SIZE)) * RASTER_SIZE;
}


void FadeContextDialogView::schedule_for_repaint( )
{
	m_vp->schedule_for_repaint(this);
}


void FadeContextDialogView::create_background( )
{
	if (background.size() != m_vp->size()) {
		background = QPixmap(m_vp->width(), m_vp->height());
	}

	QPainter p(&background);
	
// 	p.setRenderHint(QPainter::Antialiasing);

	// Background color
	p.fillRect(0, 0, m_vp->width(), m_vp->height(), cm().get("CLIP_BG_DEFAULT"));

	// grid lines
	p.setPen(QColor(INACTIVE_COLOR));
	for (int i = 0; i <= GRID_LINES; i++) {
		float d = (float)i / (float)GRID_LINES;
		int x = (int)(m_vp->width() * d);
		int y = (int)(m_vp->height() * d);
		p.drawLine(x, 0, x, m_vp->height());
		p.drawLine(0, y, m_vp->width(), y);
	}

}



/******** SLOTS ***************/

void FadeContextDialogView::resize( )
{
	set_geometry(0, 0, m_vp->width(), m_vp->height());
	create_background();
	schedule_for_repaint();
}


Command * FadeContextDialogView::bend( )
{
	return new FadeBend(this, m_fade);
}

Command * FadeContextDialogView::strength( )
{
	return new FadeStrength(this, m_fade);
}


/****** Command classes used by FadeContextDialogView *******/

/********** FadeBend **********/
/******************************/

int FadeBend::begin_hold()
{
	PENTER;
	origY = cpointer().y();
	oldValue =  m_fade->get_bend_factor();
	return 1;
}


int FadeBend::jog()
{
	int direction = (m_fade->get_fade_type() == FadeCurve::FadeIn) ? 1 : -1;

	if (m_fade->get_raster()) {
		float value = m_view->roundFloat(oldValue + ((origY - cpointer().y()) / CURSOR_SPEED) * direction);
		m_fade->set_bend_factor(value);
	} else {
		m_fade->set_bend_factor(oldValue + (float(origY - cpointer().y()) / CURSOR_SPEED) * direction);
	}

	return 1;
}

/********** FadeStrength **********/
/******************************/

int FadeStrength::begin_hold()
{
	PENTER;
	origY = cpointer().y();
	oldValue =  m_fade->get_strenght_factor();
	return 1;
}


int FadeStrength::jog()
{
	if (m_fade->get_bend_factor() >= 0.5) {
		m_fade->set_strength_factor(oldValue + float(origY - cpointer().y()) / CURSOR_SPEED);
	} else {
		if (m_fade->get_raster()) {
			float value = m_view->roundFloat(oldValue + (origY - cpointer().y()) / CURSOR_SPEED);
			m_fade->set_strength_factor(value);
		} else {
			m_fade->set_strength_factor(oldValue - float(origY - cpointer().y()) / CURSOR_SPEED);
		}
	}
	

	return 1;
}

//eof
