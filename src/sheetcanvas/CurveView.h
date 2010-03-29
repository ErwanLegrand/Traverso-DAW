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
#include "Command.h"

#include <QTimer>

class Curve;
class CurveNode;
class CurveNodeView;
class QPoint;
class CurveView;

class DragNode : public Command
{
	Q_OBJECT
	Q_CLASSINFO("move_up", tr("Move Up"));
	Q_CLASSINFO("move_down", tr("Move Down"));
	
public:
	DragNode(CurveNode* node,
		CurveView* curveview,
   		qint64 scalefactor,
      		TimeRef rangeMin,
		TimeRef rangeMax,
  		const QString& des);

	int prepare_actions();
	int do_action();
	int undo_action();
	int finish_hold();
	void cancel_action();
	int begin_hold();
	int jog();
	void set_cursor_shape(int useX, int useY);
	void set_vertical_only();

private :
	class	Private {
		public:
			CurveView*	curveView;
			qint64		scalefactor;
			TimeRef		rangeMin;
			TimeRef		rangeMax;
			QPoint		mousepos;
			bool		verticalOnly;
	};

	Private* d;
	CurveNode* m_node;
	double	m_origWhen;
	double	m_origValue;
	double	m_newWhen;
	double 	m_newValue;
	
	int calculate_and_set_node_values();


public slots:
	void move_up(bool autorepeat);
	void move_down(bool autorepeat);
};


class CurveView : public ViewItem
{
	Q_OBJECT
	Q_CLASSINFO("add_node", tr("New node"))
	Q_CLASSINFO("remove_node", tr("Remove node"))
	Q_CLASSINFO("remove_all_nodes", tr("Remove all Nodes"))
	Q_CLASSINFO("drag_node", tr("Move node"))
	Q_CLASSINFO("drag_node_vertical_only", tr("Move node (vertical only)"))

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

	void set_start_offset(const TimeRef& offset);
	const TimeRef& get_start_offset() const {return m_startoffset;}
	

private:
	Curve*		m_curve;
	Curve*		m_guicurve;
	QTimer		m_blinkTimer;
	CurveNodeView*	m_blinkingNode;
	int		m_blinkDarkness;
	int		m_blinkColorDirection;
	QList<CurveNodeView*>	m_nodeViews;
	TimeRef		m_startoffset;
	
        void update_softselected_node(QPointF pos);

public slots:
	Command* add_node();
	Command* remove_node();
	Command* remove_all_nodes();
	Command* drag_node();
	Command* drag_node_vertical_only();
	
private slots:
	void add_curvenode_view(CurveNode* node);
	void remove_curvenode_view(CurveNode* node);
	void node_moved();
	void set_view_mode();
	void update_blink_color();
        void active_context_changed();

	
signals :
	// emit from the gui so that we can stop following the playhead only
	// when the user manually edits, not on undo/redo
	void curveModified();
};

#endif

//eof
 
