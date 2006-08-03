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

$Id: FadeContextDialogView.cpp,v 1.2 2006/08/03 15:14:04 r_sijrier Exp $
*/

#include "FadeContextDialogView.h"

#include "ViewPort.h"
#include <QPainterPath>
#include <Debugger.h>
#include <FadeCurve.h>
#include <CurveNode.h>

static const int GRID_LINES		= 4;
static const float CURSOR_SPEED		= 150.0;
static const float RASTER_SIZE		= 0.05;
static const int DOT_SIZE		= 6;
static const int VMARGIN		= 25;
static const QString LINE_COLOR		= "#000000";
static const QString DOT_COLOR		= "#7b806f";
static const QString INACTIVE_COLOR	= "#7b806f";


FadeContextDialogView::FadeContextDialogView(ViewPort* viewPort, FadeCurve* fadeCurve)
		: ViewItem(viewPort, 0, fadeCurve), m_vp(viewPort), m_fade(fadeCurve)
{
	// initialize default values
	m_raster = false;
	m_mode = 0;

	points.append(QPointF(0.0, 0.0));
	points.append(QPointF(0.25, 0.25));
	points.append(QPointF(0.75, 0.75));
	points.append(QPointF(1.0, 1.0));

	m_vp->register_viewitem(this);
	init_context_menu( this );
	
	reset();

	connect(m_vp, SIGNAL(resized()), this, SLOT(resize()));
	connect(m_fade, SIGNAL(stateChanged()), this, SLOT(state_changed()));
}


FadeContextDialogView::~FadeContextDialogView()
{}


QRect FadeContextDialogView::draw( QPainter& p )
{

	// Draw the background pixmap
	p.drawPixmap(0, 0, background);

	p.setRenderHint(QPainter::Antialiasing);
	
	// Calculate and draw control points
	float h = float(m_vp->height() - 2*VMARGIN - 1);
	QPoint p1(int(points[1].x() * m_vp->width()), m_vp->height()-1-VMARGIN - int(points[1].y() * h));
	QPoint p2(int(m_vp->width()-1 - (1.0 - points[2].x()) * (m_vp->width()-1)), VMARGIN + int(float(1.0 - points[2].y()) * h));

	p.setPen(QColor(DOT_COLOR));
	p.setBrush(QColor(DOT_COLOR));
	p.drawLine(0, m_vp->height()-1-VMARGIN, p1.x(), p1.y());
	p.drawLine(m_vp->width()-1, VMARGIN, p2.x(), p2.y());
	p.drawEllipse(p1.x() - DOT_SIZE/2, p1.y() - DOT_SIZE/2, DOT_SIZE, DOT_SIZE);
	p.drawEllipse(p2.x() - DOT_SIZE/2, p2.y() - DOT_SIZE/2, DOT_SIZE, DOT_SIZE);


	QPolygonF polygon;
	float value[2];
	int step = 0;
	int height = m_vp->height()-2*VMARGIN;
	
	int zoom = (int) m_fade->get_range() / m_vp->width();
	
	// Populate the polygon with enough points to draw a smooth curve.
	// using a 2 pixel resolution, is sufficient enough
	for (int i=0; i < m_vp->width(); i+=2) {
		m_fade->get_vector(i * zoom, i * zoom + 1, value, 2);
	 	polygon << QPointF( step, height - (value[1] * height) + VMARGIN );
	 	step += 2;
	}

	// Always add the uppermost point in the polygon path 
	// since the above routine potentially does not include it.
	polygon << QPointF(m_vp->width(), VMARGIN);
	
	// Draw the curve
	QPainterPath path;
	path.moveTo(VMARGIN, m_vp->height());
	path.addPolygon(polygon);
	path.lineTo(0, VMARGIN);
	path.closeSubpath();
	
	p.setPen(Qt::NoPen);
	p.setBrush(QColor(255, 0, 255, 60));
	
	p.drawPath(path);	

	return QRect();
}


void FadeContextDialogView::solve( )
{
	// calculate points
	if (m_mode == 0) {
		points[1] = QPointF(m_fade->get_strenght_factor() * (1.0 - m_fade->get_bend_factor()), m_fade->get_strenght_factor() * m_fade->get_bend_factor());
		points[2] = QPointF(1.0 - (m_fade->get_strenght_factor() * m_fade->get_bend_factor()), 1.0 - (m_fade->get_strenght_factor() * (1.0 - m_fade->get_bend_factor())));
	}
	if (m_mode == 1) {
		points[1] = QPointF(m_fade->get_strenght_factor() * (1.0 - m_fade->get_bend_factor()), m_fade->get_strenght_factor() * m_fade->get_bend_factor());
		points[2] = QPointF(1.0 - (m_fade->get_strenght_factor() * (1.0 - m_fade->get_bend_factor())), 1.0 - (m_fade->get_strenght_factor() * m_fade->get_bend_factor()));
	}


	// calculate curve nodes
	float f = 0.0;
 	for (int i = 1; i < (m_fade->get_nodes()->size() -1); i++) {
		
		f += 1.0/ (m_fade->get_nodes()->size() - 1);
		
		CurveNode* node = m_fade->get_nodes()->at(i);
		QPointF p = getCurvePoint(f);
		
// 		printf("point is x=%f y=%f\n", p.x(), p.y());
		node->set_relative_when(p.x());
		node->set_value(p.y());
		
// 		printf("f is %f\n", f);
	}
	
// 	printf("\n\n");
}


float FadeContextDialogView::roundFloat( float f)
{
	return float(int(0.5 + f / RASTER_SIZE)) * RASTER_SIZE;
}


QPointF FadeContextDialogView::getCurvePoint( float f)
{
	float x = points[0].x() * pow((1.0 - f), 3.0)
		+ 3 * points[1].x() * f * pow((1.0 - f), 2.0)
		+ 3 * points[2].x() * pow(f, 2.0) * (1.0 - f)
		+ points[3].x() * pow(f, 3.0);

	float y = points[0].y() * pow((1.0 - f), 3.0)
		+ 3 * points[1].y() * f * pow((1.0 - f), 2.0)
		+ 3 * points[2].y() * pow(f, 2.0) * (1.0 - f)
		+ points[3].y() * pow(f, 3.0);

	return QPointF(x, y);
}

void FadeContextDialogView::schedule_for_repaint( )
{
	set_geometry(0, 0, m_vp->width(), m_vp->height());
	m_vp->schedule_for_repaint(this);
}


void FadeContextDialogView::create_background( )
{
	if (background.size() != m_vp->size()) {
		background = QPixmap(m_vp->width(), m_vp->height());
	}

	QPainter p(&background);

	p.setRenderHint(QPainter::Antialiasing);

	QString m = "", r = "";
	if (m_fade->get_bend_factor() == 0.5) {
		m = "Mode: linear";
	} else {
		if (m_mode == 0)
			m = "Mode: bended";
		if (m_mode == 1)
			m = "Mode: s-shape";
	}

	if (m_raster) {
		r = "Raster: on";
	} else {
		r = "Raster: off";
	}

	QFontMetrics ftm(p.font());
	int dist = ftm.descent() + 3;

	// Background color
	p.fillRect(0, 0, m_vp->width(), m_vp->height(), QColor("#a4aa94"));

	// grid lines
	p.setPen(QColor(INACTIVE_COLOR));
	for (int i = 0; i <= GRID_LINES; i++) {
		float d = (float)i / (float)GRID_LINES;
		int x = int(((float) m_vp->width() * d) + 0.5);
		int y = int(((float)(m_vp->height() - 2*VMARGIN) * d) + 0.5) + VMARGIN;
		p.drawLine(x, VMARGIN, x, m_vp->height()-VMARGIN);
		p.drawLine(0, y, m_vp->width(), y);
	}

	// text labels
	p.setPen(QColor(LINE_COLOR));
	p.drawText(          5, m_vp->height() - dist, QString("Bending: %1").arg(2.0 * (m_fade->get_bend_factor() - 0.5), 0, 'f', 2));
	p.drawText(1*m_vp->width()/4, m_vp->height() - dist, QString("Strength: %1").arg(2.0 * m_fade->get_strenght_factor(), 0, 'f', 2));
	p.drawText(2*m_vp->width()/4, m_vp->height() - dist, m);
	p.drawText(3*m_vp->width()/4, m_vp->height() - dist, r);

	p.setPen(QColor(INACTIVE_COLOR));
	dist = ftm.ascent() + 3;
	p.drawText(          5, dist, "Raster: <r>");
	p.drawText(1*m_vp->width()/5, dist, "Bending: [b]");
	p.drawText(2*m_vp->width()/5, dist, "Strength: [s]");
	p.drawText(3*m_vp->width()/5, dist, "Mode: <m>");
	p.drawText(4*m_vp->width()/5, dist, "Reset: <l>");
}



/******** SLOTS ***************/

void FadeContextDialogView::resize( )
{
	create_background();
	solve();
	schedule_for_repaint();
}


Command* FadeContextDialogView::set_mode( )
{
	PENTER;

	if (m_mode < 1) {
		m_mode++;
	} else {
		m_mode = 0;
	}

	solve();

	create_background();
	schedule_for_repaint();

	return 0;
}

void FadeContextDialogView::state_changed( )
{
	solve();
	schedule_for_repaint();
}


Command * FadeContextDialogView::reset( )
{
	m_fade->set_bend_factor(0.5);
	m_fade->set_strength_factor(0.5);

	solve();

	create_background();
	schedule_for_repaint();

	return 0;
}

Command * FadeContextDialogView::bend( )
{
	return new FadeBend(this, m_fade);
}

Command * FadeContextDialogView::strength( )
{
	return new FadeStrength(this, m_fade);
}

Command * FadeContextDialogView::toggle_raster( )
{
	m_raster = ! m_raster;
	create_background();
	schedule_for_repaint();
	
	return 0;
}



/****** Command classes used by FadeContextDialogView *******/

/********** FadeBend **********/
/******************************/

int FadeBend::begin_hold()
{
	PENTER;
	origY = cpointer().y();
	oldValue =  m_curve->get_bend_factor();
	return 1;
}


int FadeBend::jog()
{

	if (m_view->get_raster()) {
		m_curve->set_bend_factor(m_view->roundFloat(m_curve->get_bend_factor()));
	} else {
		m_curve->set_bend_factor(oldValue + float(origY - cpointer().y()) / CURSOR_SPEED);
	}

	return 1;
}

/********** FadeStrength **********/
/******************************/

int FadeStrength::begin_hold()
{
	PENTER;
	origY = cpointer().y();
	oldValue =  m_curve->get_strenght_factor();
	return 1;
}


int FadeStrength::jog()
{
	if (m_curve->get_bend_factor() >= 0.5) {
		m_curve->set_strength_factor(oldValue + float(origY - cpointer().y()) / CURSOR_SPEED);
	} else {
		if (m_view->get_raster()) {
			m_curve->set_strength_factor(m_view->roundFloat(m_curve->get_strenght_factor()));
		} else {
			m_curve->set_strength_factor(oldValue - float(origY - cpointer().y()) / CURSOR_SPEED);
		}
	}
	

	return 1;
}

//eof
