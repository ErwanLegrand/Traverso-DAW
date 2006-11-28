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

$Id: CurveView.h,v 1.1 2006/11/28 14:06:12 r_sijrier Exp $
*/

#ifndef CURVE_VIEW_H
#define CURVE_VIEW_H

#include "ViewItem.h"

class Curve;

class CurveView : public ViewItem
{
	Q_OBJECT

public:
	CurveView(SongView* sv, ViewItem* parentViewItem, Curve* curve);
	~CurveView();
	
	enum {Type = UserType + 8};
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	int type() const;
	
private:
	Curve*	m_curve;

public slots:
};


inline int CurveView::type() const {return Type;}

#endif

//eof
 
