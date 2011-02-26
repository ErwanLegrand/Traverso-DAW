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
	CurveView(SheetView* sv, ViewItem* parentViewItem, Curve* curve);
	~CurveView();
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	int get_vector(int xstart, int pixelcount, float *arg);
	bool has_nodes() const;
	float get_default_value();
	void calculate_bounding_rect();
	void load_theme_data();
        void mouse_hover_move_event();
	QString get_name() const;

        void set_start_offset(TimeRef offset);
        TimeRef get_start_offset() const {return m_startoffset;}
        CurveNodeView* get_node_view_after(TimeRef location) const;
        CurveNodeView* get_node_view_before(TimeRef location) const;
        Curve* get_curve() const {return m_curve;}
	
        void update_softselected_node(QPointF pos);

private:
	Curve*		m_curve;
	Curve*		m_guicurve;
	QTimer		m_blinkTimer;
	CurveNodeView*	m_blinkingNode;
	int		m_blinkDarkness;
	int		m_blinkColorDirection;
	QList<CurveNodeView*>	m_nodeViews;
	TimeRef		m_startoffset;
	
	QList<CurveNodeView*>	get_selected_nodes();

public slots:
	TCommand* add_node();
	TCommand* remove_node();
	TCommand* remove_all_nodes();
	TCommand* drag_node();
	TCommand* select_lazy_selected_node();
	TCommand* toggle_select_all_nodes();
	
private slots:
	void add_curvenode_view(CurveNode* node);
	void remove_curvenode_view(CurveNode* node);
	void node_moved();
	void update_blink_color();
        void active_context_changed();

	
signals :
	// emit from the gui so that we can stop following the playhead only
	// when the user manually edits, not on undo/redo
	void curveModified();
};

#endif

//eof
 
