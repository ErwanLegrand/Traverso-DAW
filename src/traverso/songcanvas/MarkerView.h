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
class SongView;

class MarkerView : public ViewItem
{
	Q_OBJECT
	
public:
	MarkerView(Marker* marker, SongView* sv, ViewItem* parent);
	~MarkerView() {}
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void calculate_bounding_rect();
	void load_theme_data();
	
	Marker* get_marker() const {return m_marker;}
	
private:
	Marker* m_marker;
	
private slots:
	void update_position();
	
};

#endif

//eof
