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

$Id: CurveView.h,v 1.3 2006/12/04 19:24:54 r_sijrier Exp $
*/

#ifndef CURVE_VIEW_H
#define CURVE_VIEW_H

#include "ViewItem.h"

#include <QTimer>

class Curve;
class CurveNode;
class CurveNodeView;

class CurveView : public ViewItem
{
	Q_OBJECT

public:
	CurveView(SongView* sv, ViewItem* parentViewItem, Curve* curve);
	~CurveView();
	
	enum {Type = UserType + 8};
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	int type() const;
	void calculate_bounding_rect();
	
protected:
	void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
	void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
	void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );

private:
	Curve*		m_curve;
	QTimer		m_blinkTimer;
	CurveNodeView*	m_blinkingNode;
	QColor		m_blinkColor;
	int		m_blinkColorDirection;
	QList<CurveNodeView*>	m_nodeViews;

public slots:
	
	Command* add_node();
	Command* remove_item();
	Command* drag();
	
private slots:
	void add_curvenode_view(CurveNode* node);
	void remove_curvenode_view(CurveNode* node);
	void curve_changed();
	void set_view_mode();
	
	void update_blink_color();
};


inline int CurveView::type() const {return Type;}

#endif

//eof
 
