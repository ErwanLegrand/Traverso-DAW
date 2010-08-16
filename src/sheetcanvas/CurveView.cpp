/*
Copyright (C) 2006-2007 Remon Sijrier

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

#include "CurveView.h"
#include "SheetView.h"
#include "CurveNodeView.h"
#include <Themer.h>
#include "AudioDevice.h"
		
#include <Curve.h>
#include <CurveNode.h>
#include <ContextPointer.h>
#include <Sheet.h>
#include <InputEngine.h>

#include <AddRemove.h>
#include "CommandGroup.h"
#include "MoveCurveNode.h"

#include <Debugger.h>


#define NODE_SOFT_SELECTION_DISTANCE 40
	

CurveView::CurveView(SheetView* sv, ViewItem* parentViewItem, Curve* curve)
	: ViewItem(parentViewItem, curve)
	, m_curve(curve)
{
	setZValue(parentViewItem->zValue() + 1);
        setFlags(QGraphicsItem::ItemUsesExtendedStyleOption);

	m_sv = sv;
	load_theme_data();
	
	m_blinkColorDirection = 1;
	m_blinkingNode = 0;
	m_startoffset = TimeRef();
	m_guicurve = new Curve(0);
	m_guicurve->set_sheet(sv->get_sheet());
	
	apill_foreach(CurveNode* node, CurveNode, m_curve->get_nodes()) {
		add_curvenode_view(node);
	}
	
	connect(&m_blinkTimer, SIGNAL(timeout()), this, SLOT(update_blink_color()));
	connect(m_curve, SIGNAL(nodeAdded(CurveNode*)), this, SLOT(add_curvenode_view(CurveNode*)));
	connect(m_curve, SIGNAL(nodeRemoved(CurveNode*)), this, SLOT(remove_curvenode_view(CurveNode*)));
	connect(m_curve, SIGNAL(nodePositionChanged()), this, SLOT(node_moved()));
        connect(m_curve, SIGNAL(activeContextChanged()), this, SLOT(active_context_changed()));
	connect(m_sv->get_sheet(), SIGNAL(modeChanged()), this, SLOT(set_view_mode()));
	
        m_hasMouseTracking = true;
	
	set_view_mode();
}

CurveView::~ CurveView( )
{
	m_guicurve->clear_curve();
	delete m_guicurve;
}

static bool smallerpoint(const QPointF& left, const QPointF& right) {
	return left.x() < right.x();
}

void CurveView::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);
	PENTER2;
	
	
	int xstart = (int) option->exposedRect.x();
	int pixelcount = (int) option->exposedRect.width()+1;
	int height = int(m_boundingRect.height());
	int offset = int(m_startoffset / m_sv->timeref_scalefactor);
	

        if (m_curve->has_active_context()) {
                painter->fillRect(option->exposedRect, QColor(255,255,255,70));
        }

        QPen pen;
	
        pen.setColor(themer()->get_color("Curve:active"));
	
        if (m_boundingRect.height() > 40) {
                pen.setWidth(2);
        }
	painter->save();
	painter->setPen(pen);
	painter->setClipRect(m_boundingRect);
	
	
	if (m_nodeViews.size() == 1) {
		int y = int(height - (m_nodeViews.first()->value * height));
		painter->drawLine(xstart, y, xstart + pixelcount, y);
		painter->restore();
		return;
	}
	
	if (m_nodeViews.first()->when > xstart) {
		int y = int(height - (m_nodeViews.first()->value * height));
		int length = int(m_nodeViews.first()->when) - xstart - offset;
		if (length > 0) {
			painter->drawLine(xstart, y, xstart + length, y);
			xstart += length;
			pixelcount -= length;
		}
		if (pixelcount <= 0) {
			painter->restore();
			return;
		}
	}
	
	if (m_nodeViews.last()->when < (xstart + pixelcount + offset)) {
		int y = int(height - (m_nodeViews.last()->value * height));
		int x = int(m_nodeViews.last()->when) - offset;
		int length = (xstart + pixelcount) - int(m_nodeViews.last()->when) + offset;
		if (length > 0) {
			painter->drawLine(x, y, x + length - 1, y);
			pixelcount -= length;
		}
		if (pixelcount <= 0) {
			painter->restore();
			return;
		}
	}
	
	
	// Path's need an additional pixel righ/left to be painted correctly.
	// FadeCurveView get_curve adjusts for this, if changing these 
	// values, also change the adjustment in FadeCurveView::get_curve() !!!
	pixelcount += 2;
	xstart -= 1;
	if (xstart < 0) {
		xstart = 0;
	}
	
	painter->setRenderHint(QPainter::Antialiasing);
	
	QPolygonF polygon;
	float vector[pixelcount];
	
// 	printf("range: %d\n", (int)m_nodeViews.last()->pos().x());
	m_guicurve->get_vector(xstart + offset,
				xstart + pixelcount + offset,
    				vector,
    				pixelcount);
	
	for (int i=0; i<pixelcount; i++) {
                polygon <<  QPointF(xstart + i, height - (vector[i] * height) );
	}
	
	// Depending on the zoom level, curve nodes can end up to be aligned 
	// vertically at the exact same x position. The curve line won't be painted
	// by the routine above (it doesn't catch the second node position obviously)
	// so we add curvenodes _always_ to solve this problem easily :-)
	apill_foreach(CurveNodeView* view, CurveNodeView, m_nodeViews) {
		qreal x = view->x();
		if ( (x > xstart) && x < (xstart + pixelcount)) {
			polygon <<  QPointF( x + view->boundingRect().width() / 2,
                                (height - (view->get_curve_node()->get_value() * height)) );
		}
	}
	
	// Which means we have to sort the polygon *sigh* (rather cpu costly, but what can I do?)
	qSort(polygon.begin(), polygon.end(), smallerpoint);
	
/*	for (int i=0; i<polygon.size(); ++i) {
		printf("polygin %d, x=%d, y=%d\n", i, (int)polygon.at(i).x(), (int)polygon.at(i).y());
	}*/

        QPainterPath path;
        path.addPolygon(polygon);

        painter->drawPath(path);
	
	if (xstart <= 100) {
		painter->setFont(themer()->get_font("CurveView:fontscale:label"));
		painter->drawText(10, (int)(m_boundingRect.height() - 14), "Gain Curve");
	}

	painter->restore();
}

int CurveView::get_vector(int xstart, int pixelcount, float* arg)
{
	if (m_guicurve->get_nodes().size() == 1 && ((CurveNode*)m_guicurve->get_nodes().first())->value == 1.0) {
		return 0;
	}
	
	m_guicurve->get_vector(xstart, xstart + pixelcount, arg, pixelcount);
	
	return 1;
}

void CurveView::add_curvenode_view(CurveNode* node)
{
	CurveNodeView* nodeview = new CurveNodeView(m_sv, this, node, m_guicurve);
	m_nodeViews.append(nodeview);
	
	AddRemove* cmd = (AddRemove*) m_guicurve->add_node(nodeview, false);
	cmd->set_instantanious(true);
	TCommand::process_command(cmd);
	
	qSort(m_nodeViews.begin(), m_nodeViews.end(), Curve::smallerNode);
	
	update();
}

void CurveView::remove_curvenode_view(CurveNode* node)
{
	apill_foreach(CurveNodeView* nodeview, CurveNodeView, m_nodeViews) {
		if (nodeview->get_curve_node() == node) {
			m_nodeViews.removeAll(nodeview);
			if (nodeview == m_blinkingNode) {
                                m_blinkingNode = 0;
                                update_softselected_node(cpointer().scene_pos());
			}
			AddRemove* cmd = (AddRemove*) m_guicurve->remove_node(nodeview, false);
			cmd->set_instantanious(true);
			TCommand::process_command(cmd);
			
			scene()->removeItem(nodeview);
			delete nodeview;
			update();
			return;
		}
	}
}

void CurveView::calculate_bounding_rect()
{
	int y  = m_parentViewItem->get_childview_y_offset();
	m_boundingRect = QRectF(0, 0, m_parentViewItem->boundingRect().width(), m_parentViewItem->get_height());
	setPos(0, y);
	ViewItem::calculate_bounding_rect();
}

void CurveView::active_context_changed()
{
        if (has_active_context()) {
                m_blinkTimer.start(40);
        } else {
                if (ie().is_holding()) {
                        return;
                }

                m_blinkTimer.stop();
                if (m_blinkingNode) {
                        m_blinkingNode->set_color(themer()->get_color("CurveNode:default"));
                        m_blinkingNode->reset_size();
                        m_blinkingNode = 0;
                }

        }

        update(m_boundingRect);
}

	
void CurveView::mouse_hover_move_event()
{
        update_softselected_node(cpointer().scene_pos());

	if (m_blinkingNode) {
		setCursor(themer()->get_cursor("CurveNode"));
	} else {
		setCursor(themer()->get_cursor("AudioClip"));
	}
// 	printf("mouse x,y pos %d,%d\n", point.x(), point.y());
	
// 	printf("\n");
}


void CurveView::update_softselected_node(QPointF point)
{
        if (ie().is_holding()) {
		return;
	}

        QPoint pos = mapToItem(this, point).toPoint();
	
	CurveNodeView* prevNode = m_blinkingNode;
	m_blinkingNode = m_nodeViews.first();
	
	if (! m_blinkingNode)
		return;
	
	foreach(CurveNodeView* nodeView, m_nodeViews) {
		
                QPoint nodePos((int)nodeView->scenePos().x(), (int)nodeView->scenePos().y());
// 		printf("node x,y pos %d,%d\n", nodePos.x(), nodePos.y());
		
		int nodeDist = (pos - nodePos).manhattanLength();
                int blinkNodeDist = (pos - QPoint((int)m_blinkingNode->scenePos().x(), (int)m_blinkingNode->scenePos().y())).manhattanLength();
		
		if (nodeDist < blinkNodeDist) {
			m_blinkingNode = nodeView;
		}
	}

        if ((pos - QPoint(4, 4) - QPoint((int)m_blinkingNode->scenePos().x(), (int)m_blinkingNode->scenePos().y())).manhattanLength() > NODE_SOFT_SELECTION_DISTANCE) {
		m_blinkingNode = 0;
	}
	
	if (prevNode && (prevNode != m_blinkingNode) ) {
		prevNode->set_color(themer()->get_color("CurveNode:default"));
		prevNode->update();
		prevNode->reset_size();
		if (m_blinkingNode) {
			m_blinkingNode->set_selected();
		}
	}
	if (!prevNode && m_blinkingNode) {
		m_blinkingNode->set_selected();
		m_blinkDarkness = 100;
	}
}


void CurveView::update_blink_color()
{
	if (!m_blinkingNode) {
		return;
	}
	
	m_blinkDarkness += (6 * m_blinkColorDirection);
	
	if (m_blinkDarkness >= 100) {
		m_blinkColorDirection *= -1;
		m_blinkDarkness = 100;
	} else if (m_blinkDarkness <= 40) {
		m_blinkColorDirection *= -1;
		m_blinkDarkness = 40;
	}
	
	QColor blinkColor = themer()->get_color("CurveNode:blink");
	
	m_blinkingNode->set_color(blinkColor.light(m_blinkDarkness));
	
	m_blinkingNode->update();
}


TCommand* CurveView::add_node()
{
	PENTER;
	QPointF point = mapFromScene(cpointer().scene_pos());

	emit curveModified();
	
	double when = point.x() * double(m_sv->timeref_scalefactor) + m_startoffset.universal_frame();
	double value = (m_boundingRect.height() - point.y()) / m_boundingRect.height();
	
	CurveNode* node = new CurveNode(m_curve, when, value);
	
	return m_curve->add_node(node);
}


TCommand* CurveView::remove_node()
{
	PENTER;

	emit curveModified();

        update_softselected_node(cpointer().on_first_input_event_scene_pos());

	if (m_blinkingNode) {
		CurveNode* node = m_blinkingNode->get_curve_node();
		m_blinkingNode = 0;
		return m_curve->remove_node(node);
	}
	return ie().did_not_implement();
}

TCommand* CurveView::drag_node()
{
	PENTER;

        update_softselected_node(cpointer().on_first_input_event_scene_pos());
	
	if (m_blinkingNode) {
		TimeRef min(qint64(0));
		TimeRef max(qint64(-1));
		APILinkedList nodeList = m_curve->get_nodes();
		CurveNode* node = m_blinkingNode->get_curve_node();
		int index = nodeList.indexOf(node);
		
		emit curveModified();
		
		if (index > 0) {
			min = qint64(((CurveNode*)nodeList.at(index-1))->get_when() + 1);
		}
		if (nodeList.size() > (index + 1)) {
			max = qint64(((CurveNode*)nodeList.at(index+1))->get_when() - 1);
		}
                return new MoveCurveNode(m_blinkingNode->get_curve_node(), this, m_sv->timeref_scalefactor, min, max, tr("Move Curve Node"));
	}
	return ie().did_not_implement();
}


TCommand * CurveView::drag_node_vertical_only()
{
        MoveCurveNode* drag = qobject_cast<MoveCurveNode*>(drag_node());
	
	if (!drag) {
		return 0;
	}
	
	drag->set_vertical_only();
	
	return drag;
}


void CurveView::node_moved( )
{
	if (!m_blinkingNode) {
		update();
		return;
	}

	CurveNodeView* prev = 0;
	CurveNodeView* next = 0;
	
	int index = m_nodeViews.indexOf(m_blinkingNode);
	int xleft = (int) m_blinkingNode->x(), xright = (int) m_blinkingNode->x();
	int leftindex = index;
	int count = 0;
	
	if (m_blinkingNode == m_nodeViews.first()) {
		xleft = 0;
	} else {
		while ( leftindex > 0 && count < 2) {
			leftindex--;
			count++;
		}
		prev = m_nodeViews.at(leftindex);
	}	
	
	
	count = 0;
	int rightindex = index;
	
	if (m_blinkingNode == m_nodeViews.last()) {
		xright = (int) m_boundingRect.width();
	} else {
		while (rightindex < (m_nodeViews.size() - 1) && count < 2) {
			rightindex++;
			count++;
		}
		next = m_nodeViews.at(rightindex);
	}
	
	
	if (prev) xleft = (int) prev->x();
	if (next) xright = (int) next->x();
	
	
	update(xleft, 0, xright - xleft + 3, m_boundingRect.height());
}

void CurveView::set_view_mode()
{
	if (m_sv->get_sheet()->get_mode() == Sheet::EFFECTS) {
		show();
	} else {
		hide();
	}
}

void CurveView::load_theme_data()
{
	calculate_bounding_rect();
}

void CurveView::set_start_offset(TimeRef offset)
{
	m_startoffset = offset;
}

bool CurveView::has_nodes() const
{
	return m_guicurve->get_nodes().size() > 1 ? true : false;
}

float CurveView::get_default_value()
{
	return ((CurveNode*)m_guicurve->get_nodes().first())->value;
}

TCommand * CurveView::remove_all_nodes()
{
	CommandGroup* group = new CommandGroup(m_curve, tr("Clear Nodes"));

	apill_foreach(CurveNode* node, CurveNode, m_curve->get_nodes()) {
		group->add_command(m_curve->remove_node(node));
	}

	return group;
}

CurveNodeView* CurveView::get_node_view_before(TimeRef location) const
{
        TimeRef curveStartOffset = m_curve->get_start_offset();

        for (int i = m_nodeViews.size() - 1; i>=0; --i) {
                CurveNodeView* nodeview = m_nodeViews.at(i);
                TimeRef absoluteLocation = TimeRef(nodeview->get_curve_node()->get_when()) + curveStartOffset;
                if (absoluteLocation < location) {
                        return nodeview;
                }
        }

        return 0;
}

CurveNodeView* CurveView::get_node_view_after(TimeRef location) const
{
        TimeRef curveStartOffset = m_curve->get_start_offset();

        foreach(CurveNodeView* nodeview, m_nodeViews) {
                TimeRef absoluteLocation = TimeRef(nodeview->get_curve_node()->get_when()) + curveStartOffset;
                if (absoluteLocation > location) {
                        return nodeview;
                }
        }

        return 0;
}
