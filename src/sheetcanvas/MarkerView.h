/*
    Copyright (C) 2007 Remon Sijrier 
 
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

#ifndef MARKER_VIEW_H
#define MARKER_VIEW_H

#include "ViewItem.h"

class Marker;
class SheetView;
class LineView;
class QColor;
class PositionIndicator;

class MarkerView : public ViewItem
{
	Q_OBJECT
	
public:
	MarkerView(Marker* marker, SheetView* sv, ViewItem* parent);
	~MarkerView() {}
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void calculate_bounding_rect();
	void load_theme_data();
	void set_active(bool);
	int position();
	void set_position(int);
	void set_dragging(bool dragging);
	
	Marker* get_marker() const {return m_marker;}
	
private:
	Marker* m_marker;
	LineView* m_line;
	QColor	m_fillColor;
	bool	m_active;
	bool	m_dragging;
	PositionIndicator* m_posIndicator;
	int	m_ascent;
	int	m_width;
	
private slots:
	void update_position();
	void update_drawing();
};

#endif

//eof
