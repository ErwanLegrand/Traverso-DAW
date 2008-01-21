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

#ifndef POSITION_INDICATOR_H
#define POSITION_INDICATOR_H

#include "ViewItem.h"
#include <QPixmap>

class PositionIndicator : public ViewItem
{
	Q_OBJECT
	
public:
	PositionIndicator(ViewItem* parent);
	~PositionIndicator() {}
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void calculate_bounding_rect();
	void set_position(int x, int y);
	
	void set_value(const QString& value);
	
private:
	QString m_value;
	QPixmap m_background;
};

#endif

//eof
