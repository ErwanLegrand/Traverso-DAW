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

$Id: FadeView.cpp,v 1.11 2007/03/06 15:28:12 r_sijrier Exp $
*/

#include "FadeView.h"

#include <QPainter>

#include "FadeCurve.h"
#include "AudioClipView.h"
#include "FadeContextDialog.h"
#include "SongView.h"
#include <Themer.h>
#include <Fade.h>
#include <InputEngine.h>
#include <AddRemove.h>

#include "Song.h"

#include <Debugger.h>

static const int DOT_SIZE		= 6;
static const QString DOT_COLOR		= "#78817B";


FadeView::FadeView(SongView* sv, AudioClipView* parent, FadeCurve * fadeCurve )
	: ViewItem(parent, fadeCurve)
	, m_fadeCurve(fadeCurve)
{
	PENTERCONS;
	m_sv = sv;
	m_holdactive = false;
	m_guicurve = new Curve(0, m_sv->get_song());
	
	foreach(CurveNode* node, *m_fadeCurve->get_nodes()) {
		CurveNode* guinode = new CurveNode(m_guicurve, 
				node->get_when() / m_sv->scalefactor,
				node->get_value());
		AddRemove* cmd = (AddRemove*) m_guicurve->add_node(guinode, false);
		cmd->set_instantanious(true);
		ie().process_command(cmd);
	}
	
	load_theme_data();
	setAcceptsHoverEvents(true);
	
	connect(m_fadeCurve, SIGNAL(stateChanged()), this, SLOT(state_changed()));
	connect(m_fadeCurve, SIGNAL(rangeChanged()), this, SLOT(state_changed()));
}


FadeView::~ FadeView( )
{
	PENTERDES;
	delete m_guicurve;
}


void FadeView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(widget);
	
	
	int pixelcount = (int) option->exposedRect.width();
	
	if (pixelcount == 0) {
		return;
	}
	
	pixelcount += 1;
	
	QPolygonF polygon;
	int xstart = (int)option->exposedRect.x();
	int height = (int)m_boundingRect.height();
	float vector[pixelcount];
	
	m_guicurve->get_vector(xstart, xstart + pixelcount, vector, pixelcount);
	
	for (int i=0; i<pixelcount; i++) {
		polygon <<  QPointF(xstart + i, height - (vector[i] * height) );
	}
	
	
	painter->save();
	painter->setClipRect(m_boundingRect.intersect(m_parentViewItem->boundingRect()));
	painter->setRenderHint(QPainter::Antialiasing);
	
	QPainterPath path;
	
	path.addPolygon(polygon);
	path.lineTo(xstart + 1 + pixelcount, 0);
	path.lineTo(xstart + 1, 0);
	path.closeSubpath();
	
	painter->setPen(Qt::NoPen);
	
	QColor color = m_fadeCurve->is_bypassed() ? 
			themer()->get_color("Fade:bypassed") :
			themer()->get_color("Fade:default");
	
	if (option->state & QStyle::State_MouseOver) {
		color.setAlpha(color.alpha() + 10);
	}
	
	painter->setBrush(color);
	painter->drawPath(path);	


	if (m_holdactive) {
		// Calculate and draw control points
		int h = (int) m_boundingRect.height() - 1;
		int w = (int) m_boundingRect.width() - 1;
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

int FadeView::get_vector(int xstart, int pixelcount, float * arg)
{
	if (m_fadeCurve->get_fade_type() == FadeCurve::FadeOut) {
		int mappedx = - (int) mapFromParent(0, 0).x();
		// CurveView adjusts xstart with -1 and pixelcount with +2
		// compensate for this!
		if (false) {
			m_guicurve->get_vector(mappedx, mappedx + pixelcount, arg, pixelcount);
			return 1;
		} else {
			return 0;
		}
	}
	
	if (xstart < m_boundingRect.width()) {
		m_guicurve->get_vector(xstart, xstart + pixelcount, arg, pixelcount);
		return 1;
	}
	
	return 0;
}

/*
Command* FadeView::edit_properties()
{
	if (!m_dialog) {
		m_dialog = new FadeContextDialog(m_fadeCurve);
	}
	
	m_dialog->show();
	
	return 0;
}
*/

void FadeView::calculate_bounding_rect()
{
	QList<CurveNode*>* guinodes = m_guicurve->get_nodes();
	QList<CurveNode*>* nodes = m_fadeCurve->get_nodes();
	for (int i=0; i<guinodes->size(); ++i) {
		CurveNode* node = nodes->at(i);
		CurveNode* guinode = guinodes->at(i);
		guinode->set_when_and_value(node->get_when() / m_sv->scalefactor, node->get_value());
	}
	
	m_boundingRect = QRectF( 0, 0,
	 			m_guicurve->get_range(), 
				m_parentViewItem->get_height() );
	
	if (m_fadeCurve->get_fade_type() == FadeCurve::FadeOut) {
		setPos(m_parentViewItem->boundingRect().width() - m_boundingRect.width(), 
		       m_parentViewItem->get_childview_y_offset());
	} else {
		setPos(0, m_parentViewItem->get_childview_y_offset());
	}
}


void FadeView::state_changed( )
{
	PENTER;
	prepareGeometryChange();
	calculate_bounding_rect();
	update();
}


Command* FadeView::bend()
{
	return new FadeBend(this);
}

Command* FadeView::strength()
{
	return new FadeStrength(this);
}

void FadeView::set_holding(bool hold)
{
	m_holdactive = hold;
	update(m_boundingRect);
}


void FadeView::load_theme_data()
{
	calculate_bounding_rect();
}

//eof

