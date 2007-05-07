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
#include "SongView.h"
#include "CurveNodeView.h"
#include "ClipsViewPort.h"
#include <Themer.h>
		
#include <Curve.h>
#include <CurveNode.h>
#include <ContextPointer.h>
#include <Song.h>
#include <InputEngine.h>

#include <AddRemove.h>

#include <Debugger.h>
		
class DragNode : public Command
{
	Q_OBJECT
	Q_CLASSINFO("move_up", tr("Move Up"));
	Q_CLASSINFO("move_down", tr("Move Down"));
	
public:
	DragNode(CurveNode* node, CurveView* curveview, int scalefactor, long rangeMin, long rangeMax, const QString& des);
	
	int prepare_actions();
	int do_action();
        int undo_action();
	int finish_hold();
	void cancel_action();
        int begin_hold();
        int jog();

private :
	CurveView*	m_curveView;
	CurveNode*	m_node;
	int		m_scalefactor;
	QPointF		m_origPos;
	QPointF 	m_newPos;
	long		m_rangeMin;
	long		m_rangeMax;
	QPoint		m_mousepos;

public slots:
        void move_up(bool autorepeat);
        void move_down(bool autorepeat);
};


#include "CurveView.moc"

	
DragNode::DragNode(CurveNode* node, CurveView* curveview, int scalefactor, long rangeMin, long rangeMax, const QString& des)
	: Command(curveview->get_context(), des)
{
	m_node = node;
	m_rangeMin = rangeMin;
	m_rangeMax = rangeMax;
	m_curveView = curveview;
	m_scalefactor = scalefactor;
}

int DragNode::prepare_actions()
{
	return 1;
}

int DragNode::finish_hold()
{
	return 1;
}

void DragNode::cancel_action()
{
	undo_action();
}

int DragNode::begin_hold()
{
	m_origPos.setX(m_node->get_when());
	m_origPos.setY(m_node->get_value());
	m_newPos = m_origPos;
	
	m_mousepos = QPoint(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
	return 1;
}


int DragNode::do_action()
{
	m_node->set_when_and_value(m_newPos.x(), m_newPos.y());
	return 1;
}

int DragNode::undo_action()
{
	m_node->set_when_and_value(m_origPos.x(), m_origPos.y());
	return 1;
}

void DragNode::move_up(bool )
{
	m_newPos.setY(m_newPos.y() + ( 1 / m_curveView->boundingRect().height()) );
	do_action();
}

void DragNode::move_down(bool )
{
	m_newPos.setY(m_newPos.y() - ( 1 / m_curveView->boundingRect().height()) );
	do_action();
}

int DragNode::jog()
{
	QPoint mousepos = cpointer().pos();
	
	int dx, dy;
	dx = mousepos.x() - m_mousepos.x();
	dy = mousepos.y() - m_mousepos.y();
	
	m_mousepos = mousepos;
	
	m_newPos.setX(m_newPos.x() + dx * m_scalefactor);
	m_newPos.setY(m_newPos.y() - ( dy / m_curveView->boundingRect().height()) );
	
	if ( ((int)(m_newPos.x() - m_curveView->get_start_offset())/ m_scalefactor) > m_curveView->boundingRect().width()) {
		m_newPos.setX(m_curveView->boundingRect().width() * m_scalefactor + m_curveView->get_start_offset());
	}
	if ( m_newPos.x() - m_curveView->get_start_offset() < 0) {
		m_newPos.setX(m_curveView->get_start_offset());
	}
	
	if (m_newPos.y() < 0.0) {
		m_newPos.setY(0.0);
	}
	if (m_newPos.x() < 0.0) {
		m_newPos.setX(0.0);
	}
	if (m_newPos.y() > 1.0) {
		m_newPos.setY(1.0);
	}
	
	if (m_newPos.x() < m_rangeMin) {
		m_newPos.setX(m_rangeMin);
	} else if (m_rangeMax != -1 && m_newPos.x() > m_rangeMax) {
		m_newPos.setX(m_rangeMax);
	}

	return do_action();
}

		
CurveView::CurveView(SongView* sv, ViewItem* parentViewItem, Curve* curve)
	: ViewItem(parentViewItem, curve)
	, m_curve(curve)
{
	setZValue(parentViewItem->zValue() + 1);
	
	m_sv = sv;
#if QT_VERSION < 0x040300
	m_sv->scene()->addItem(this);
#endif
	load_theme_data();
	
	m_blinkColorDirection = 1;
	m_blinkingNode = 0;
	m_startoffset = 0;
	m_guicurve = new Curve(0, m_sv->get_song());
	
	QList<CurveNode* >* nodes = m_curve->get_nodes();
	for (int i=0; i < nodes->size(); i++) {
		add_curvenode_view(nodes->at(i));
	}
	
	connect(&m_blinkTimer, SIGNAL(timeout()), this, SLOT(update_blink_color()));
	connect(m_curve, SIGNAL(nodeAdded(CurveNode*)), this, SLOT(add_curvenode_view(CurveNode*)));
	connect(m_curve, SIGNAL(nodeRemoved(CurveNode*)), this, SLOT(remove_curvenode_view(CurveNode*)));
	connect(m_curve, SIGNAL(nodePositionChanged()), this, SLOT(node_moved()));
	connect(m_sv->get_song(), SIGNAL(modeChanged()), this, SLOT(set_view_mode()));
	
	setAcceptsHoverEvents(true);
	
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
	
	painter->save();
	
	painter->setClipRect(m_boundingRect);
	painter->setRenderHint(QPainter::Antialiasing);
	
	QPen pen;
	
	if (m_sv->get_song()->get_mode() == Song::EFFECTS) {
		pen.setColor(themer()->get_color("Curve:active"));
	} else {
		pen.setColor(themer()->get_color("Curve:inactive"));
	}
	
	painter->setPen(pen);
	
	QPolygonF polygon;
	int xstart = (int) option->exposedRect.x();
	int pixelcount = (int) option->exposedRect.width();
	
	// Path's need an additional pixel righ/left to be painted correctly.
	// FadeView get_curve adjusts for this, if changing these 
	// values, also change the adjustment in FadeView::get_curve() !!!
	pixelcount += 2;
	xstart -= 1;
	if (xstart < 0) {
		xstart = 0;
	}
	float height = m_boundingRect.height();
	float vector[pixelcount];
	
// 	printf("range: %d\n", (int)m_nodeViews.last()->pos().x());
	int offset = m_startoffset / m_sv->scalefactor;
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
	foreach(CurveNodeView* view, m_nodeViews) {
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
	
	if (xstart <= 80 && pixelcount >= 100 ) {
		painter->setFont(themer()->get_font("CurveView:fontscale:label"));
		painter->drawText(10, m_boundingRect.height() - 14, "Gain Curve");
	}
	
	painter->restore();
}

int CurveView::get_vector(int xstart, int pixelcount, float* arg)
{
	m_guicurve->get_vector(xstart, xstart + pixelcount, arg, pixelcount);
	return 1;
}

void CurveView::add_curvenode_view(CurveNode* node)
{
	CurveNodeView* nodeview = new CurveNodeView(m_sv, this, node, m_guicurve);
#if QT_VERSION < 0x040300
	m_sv->scene()->addItem(nodeview);
#endif
	m_nodeViews.append(nodeview);
	
	AddRemove* cmd = (AddRemove*) m_guicurve->add_node(nodeview, false);
	cmd->set_instantanious(true);
	Command::process_command(cmd);
	
	qSort(m_nodeViews.begin(), m_nodeViews.end(), Curve::smallerNode);
	
	update_softselected_node(cpointer().pos());
	update();
}

void CurveView::remove_curvenode_view(CurveNode* node)
{
	foreach(CurveNodeView* nodeview, m_nodeViews) {
		if (nodeview->get_curve_node() == node) {
			m_nodeViews.removeAll(nodeview);
			if (nodeview == m_blinkingNode) {
				update_softselected_node(cpointer().pos());
			}
			AddRemove* cmd = (AddRemove*) m_guicurve->remove_node(nodeview, false);
			cmd->set_instantanious(true);
			Command::process_command(cmd);
			
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

void CurveView::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
{
	Q_UNUSED(event);
	
	m_blinkColor = themer()->get_color("CurveNode:blink");
	m_blinkTimer.start(40);
}

void CurveView::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
{
	Q_UNUSED(event);
	
	if (ie().is_holding()) {
		event->ignore();
		return;
	}
	
	m_blinkTimer.stop();
	if (m_blinkingNode) {
		m_blinkingNode->set_color(themer()->get_color("CurveNode:default"));
		m_blinkingNode->reset_size();
		m_blinkingNode = 0;
	}
}
		
		
void CurveView::hoverMoveEvent ( QGraphicsSceneHoverEvent * event )
{
	QPoint point((int)event->pos().x(), (int)event->pos().y());
	
	update_softselected_node(point);

	if (m_blinkingNode) {
		setCursor(themer()->get_cursor("CurveNode"));
	} else {
		setCursor(themer()->get_cursor("AudioClip"));
	}
// 	printf("mouse x,y pos %d,%d\n", point.x(), point.y());
	
// 	printf("\n");
}


void CurveView::update_softselected_node( QPoint pos , bool force)
{
	if (ie().is_holding() && !force) {
		return;
	}
	
	CurveNodeView* prevNode = m_blinkingNode;
	m_blinkingNode = m_nodeViews.first();
	
	if (! m_blinkingNode)
		return;
	
	foreach(CurveNodeView* nodeView, m_nodeViews) {
		
		QPoint nodePos((int)nodeView->pos().x(), (int)nodeView->pos().y());
// 		printf("node x,y pos %d,%d\n", nodePos.x(), nodePos.y());
		
		int nodeDist = (pos - nodePos).manhattanLength();
		int blinkNodeDist = (pos - QPoint((int)m_blinkingNode->x(), (int)m_blinkingNode->y())).manhattanLength();
		
		if (nodeDist < blinkNodeDist) {
			m_blinkingNode = nodeView;
			m_blinkColor = themer()->get_color("CurveNode:blink");
		}
	}

	if ((pos - QPoint(4, 4) - QPoint((int)m_blinkingNode->x(), (int)m_blinkingNode->y())).manhattanLength() > 40) {
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
	}
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
	
	if (red > 255 || blue > 255) {
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
	
	CurveNode* node = new CurveNode(m_curve, point.x() * m_sv->scalefactor + m_startoffset,
					 (m_boundingRect.height() - point.y()) / m_boundingRect.height());
	return m_curve->add_node(node);
}


Command* CurveView::remove_node()
{
	PENTER;

	QPointF origPos(mapFromScene(QPoint(cpointer().on_first_input_event_scene_x(), cpointer().on_first_input_event_scene_y())));

	update_softselected_node(QPoint((int)origPos.x(), (int)origPos.y()), true);

	if (m_blinkingNode) {
		CurveNode* node = m_blinkingNode->get_curve_node();
		m_blinkingNode = 0;
		return m_curve->remove_node(node);
	}
	return ie().did_not_implement();
}

Command* CurveView::drag_node()
{
	PENTER;

	QPointF origPos(mapFromScene(QPoint(cpointer().on_first_input_event_scene_x(), cpointer().on_first_input_event_scene_y())));

	update_softselected_node(QPoint((int)origPos.x(), (int)origPos.y()), true);
	
	if (m_blinkingNode) {
		long min = 0;
		long max = -1;
		QList<CurveNode* >* nodeList = m_curve->get_nodes();
		CurveNode* node = m_blinkingNode->get_curve_node();
		int index = nodeList->indexOf(node);

		if (index > 0) {
			min = nodeList->at(index-1)->get_when() + 1;
		}
		if (nodeList->size() > index + 1) {
			max = nodeList->at(index+1)->get_when() - 1;
		}
		return new DragNode(m_blinkingNode->get_curve_node(), this, m_sv->scalefactor, min, max, tr("Drag Node"));
	}
	return ie().did_not_implement();
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
	if (m_sv->get_song()->get_mode() == Song::EFFECTS) {
		show();
	} else {
		hide();
	}
}

void CurveView::load_theme_data()
{
	calculate_bounding_rect();
}

void CurveView::set_start_offset(nframes_t offset)
{
	m_startoffset = offset;
}

//eof

