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

$Id: CurveView.cpp,v 1.3 2006/12/01 14:15:40 r_sijrier Exp $
*/

#include "CurveView.h"
#include "SongView.h"
#include "CurveNodeView.h"
#include "ClipsViewPort.h"
		
#include <Curve.h>
#include <CurveNode.h>
#include <ContextPointer.h>

#include <Debugger.h>
		
class DragNode : public Command
{
public:
	DragNode(CurveNode* node, CurveView* curveview, int scalefactor, const QString& des);
	
        int do_action();
        int undo_action();
	int finish_hold();
        int begin_hold(int useX = 0, int useY = 0);
        int jog();

private :
	CurveView*	m_curveView;
	CurveNode*	m_node;
	int		m_scalefactor;
	QPointF		m_origPos;
	QPointF 	m_newPos;
};



	
DragNode::DragNode(CurveNode* node, CurveView* curveview, int scalefactor, const QString& des)
	: Command(curveview->get_context(), des)
{
	m_node = node;
	m_curveView = curveview;
	m_scalefactor = scalefactor;
}

int DragNode::finish_hold()
{
	return 1;
}

int DragNode::begin_hold(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	m_origPos.setX(m_node->get_when());
	m_origPos.setY(m_node->get_value());
	return 1;
}


int DragNode::do_action()
{
	m_node->set_when(m_newPos.x());
	m_node->set_value(m_newPos.y());
	return 1;
}

int DragNode::undo_action()
{
	m_node->set_when(m_origPos.x());
	m_node->set_value(m_origPos.y());
	
	return 1;
}

int DragNode::jog()
{
	QPointF point = m_curveView->mapFromScene(cpointer().scene_pos());
	double x = point.x() * m_scalefactor;
	double y = (m_curveView->boundingRect().height() - point.y()) / m_curveView->boundingRect().height();
	m_newPos.setX(x);
	m_newPos.setY(y);
	return do_action();
}

		
CurveView::CurveView(SongView* sv, ViewItem* parentViewItem, Curve* curve)
	: ViewItem(parentViewItem, curve)
	, m_curve(curve)
{
	m_sv = sv;
	m_sv->scene()->addItem(this);
	m_boundingRectangle = parentViewItem->boundingRect();
	m_blinkColorDirection = 1;
	m_blinkingNode = 0;
	
	foreach(CurveNode* node, m_curve->get_nodes()) {
		add_curvenode_view(node);
	}
	
	connect(&m_blinkTimer, SIGNAL(timeout()), this, SLOT(update_blink_color()));
	connect(m_curve, SIGNAL(nodeAdded(CurveNode*)), this, SLOT(add_curvenode_view(CurveNode*)));
	connect(m_curve, SIGNAL(nodeRemoved(CurveNode*)), this, SLOT(remove_curvenode_view(CurveNode*)));
	connect(m_sv, SIGNAL(viewModeChanged()), this, SLOT(set_view_mode()));
	setAcceptsHoverEvents(true);
	
	set_view_mode();
}

CurveView::~ CurveView( )
{
}

static bool smallerNode(const QPointF& left, const QPointF& right) {
	return left.x() < right.x();
}

void CurveView::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);
	
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setPen(QColor(255, 0, 255));
	
	QPolygonF polygon;
	float value[2];
	int scaleFactor = m_sv->scalefactor;
	int xstart = (int) option->exposedRect.x();
	int pixelcount = (int) option->exposedRect.width();
	float height = m_boundingRectangle.height();
	
	// Populate the polygon with enough points to draw a smooth curve.
	// (actually a point for each pixel)
	int position = xstart;
	do {
		m_curve->get_vector(position*scaleFactor, position*scaleFactor + 1, value, 2);
		polygon <<  QPointF(position, height - (value[1] * height) );
	 	position ++;
	} while (position <= (xstart + pixelcount));

	
	// Depending on the zoom level, curve nodes can end up to be aligned 
	// vertically at the exact same x position. The curve line won't be painted
	// by the routine above (it doesn't catch the second node position obviously)
	// so we add curvenodes _always_ to solve this problem easily :-)
	foreach(CurveNodeView* view, m_nodeViews) {
		qreal x = view->x();
		if ( (x > xstart) && x < (xstart + pixelcount)) {
			polygon <<  QPointF(view->get_node()->get_when() / scaleFactor,
					    height - view->get_node()->get_value() * height);
		}
	}
	
	// Which means we have to sort the polygon *sigh* (rather cpu costly, but what can I do?)
	qSort(polygon.begin(), polygon.end(), smallerNode);
	
	QPainterPath path;
	
	path.addPolygon(polygon);
	
	painter->drawPath(path);	
}


void CurveView::add_curvenode_view(CurveNode* node)
{
	CurveNodeView* view = new CurveNodeView(m_sv, this, node);
	m_sv->scene()->addItem(view);
	m_nodeViews.append(view);
	connect(node, SIGNAL(positionChanged()), this, SLOT(curve_changed()));
}

void CurveView::remove_curvenode_view(CurveNode* node)
{
	foreach(CurveNodeView* view, m_nodeViews) {
		if (view->get_node() == node) {
			m_nodeViews.removeAll(view);
			delete view;
			update();
			return;
		}
	}
}

void CurveView::calculate_bounding_rect()
{
	m_boundingRectangle = parentItem()->boundingRect();
}

void CurveView::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
{
	PENTER;
	Q_UNUSED(event);
	
	m_blinkColor = QColor(255, 0, 255, 220);
	m_blinkTimer.start(40);
}

void CurveView::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
{
	PENTER;
	Q_UNUSED(event);
	
	m_blinkTimer.stop();
	if (m_blinkingNode) {
		m_blinkingNode->set_color(QColor(255, 0, 255, 140));
	}
}
		
		
void CurveView::hoverMoveEvent ( QGraphicsSceneHoverEvent * event )
{
	PENTER;
	QPoint point((int)event->pos().x(), (int)event->pos().y());
// 	printf("mouse x,y pos %d,%d\n", point.x(), point.y());
	CurveNodeView* prevNode = m_blinkingNode;
	m_blinkingNode = m_nodeViews.first();
	
	if (! m_blinkingNode)
		return;
	
	foreach(CurveNodeView* nodeView, m_nodeViews) {
		
		QPoint nodePos((int)nodeView->pos().x(), (int)nodeView->pos().y());
// 		printf("node x,y pos %d,%d\n", nodePos.x(), nodePos.y());
		
		int nodeDist = (point - nodePos).manhattanLength();
		int blinkNodeDist = (point - QPoint((int)m_blinkingNode->x(), (int)m_blinkingNode->y())).manhattanLength();
		
		if (nodeDist < blinkNodeDist) {
			m_blinkingNode = nodeView;
		}
	}
	
	if (prevNode && (prevNode != m_blinkingNode) ) {
		prevNode->set_color(QColor(255, 0, 255, 140));
		prevNode->update();
	}
	
// 	printf("\n");
}

void CurveView::update_blink_color()
{
	if (!m_blinkingNode) {
		return;
	}
	
	int red = m_blinkColor.red();
	int blue = m_blinkColor.blue();
	
	red += (15 * m_blinkColorDirection);
	blue += (15 * m_blinkColorDirection);
	
	if (red > 255) {
		m_blinkColorDirection *= -1;
		red = 255;
		blue = 255;
	} else if (red < 130) {
		m_blinkColorDirection *= -1;
	}
	
	m_blinkColor.setRed(red);
	m_blinkColor.setBlue(blue);
	
	m_blinkingNode->set_color(m_blinkColor);
	
	m_blinkingNode->update();
}


Command* CurveView::add_node()
{
	PENTER;
	QPointF point = mapFromScene(cpointer().scene_pos());
	
	return m_curve->add_node(point.x() * m_sv->scalefactor, (m_boundingRectangle.height() - point.y()) / m_boundingRectangle.height() );
}


Command* CurveView::remove_item()
{
	PENTER;
	if (m_blinkingNode) {
		CurveNode* node = m_blinkingNode->get_node();
		m_blinkingNode = 0;
		return m_curve->remove_node(node);
	}
	return 0;
}

Command* CurveView::drag()
{
	PENTER;
	if (m_blinkingNode) {
		return new DragNode(m_blinkingNode->get_node(), this, m_sv->scalefactor, tr("Drag Node"));
	}
	return 0;
}

void CurveView::curve_changed( )
{
	update();
}

void CurveView::set_view_mode()
{
	if (m_sv->viewmode == CurveMode) {
		show();
	} else {
		hide();
	}
}

//eof
