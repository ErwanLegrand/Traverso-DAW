/*
Copyright (C) 2005-2006 Remon Sijrier 

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

$Id: ViewItem.h,v 1.1 2006/11/08 14:45:22 r_sijrier Exp $
*/

#ifndef VIEW_ITEM_H
#define VIEW_ITEM_H

#include <ContextItem.h>
#include <QtGui>

class SongView;
		
class ViewItem : public ContextItem, public QGraphicsItem
{
	Q_OBJECT
			
public:

	ViewItem(ViewItem* parent=0, ContextItem* parentContext=0) : ContextItem(parent), QGraphicsItem(parent)
	{
		set_context_item(parentContext);
	}
	
	~ViewItem() {};
	
	enum {Type = UserType + 1};
	
	QRectF boundingRect() const;
	virtual void calculate_bounding_rect() {};
	void prepare_geometry_change() {prepareGeometryChange();}
	int type() const;

protected:

	SongView* 	m_sv;
	QRectF		m_boundingRectangle;
};

inline QRectF ViewItem::boundingRect() const {return m_boundingRectangle;}
inline int ViewItem::type() const {return Type;}

#endif

//eof
