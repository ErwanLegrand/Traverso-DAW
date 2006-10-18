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

$Id: ViewItem.h,v 1.5 2006/10/18 12:08:56 r_sijrier Exp $
*/

#ifndef VIEWITEM_H
#define VIEWITEM_H

#include <ContextItem.h>
#include <ContextPointer.h>
#include "ViewPort.h"

class QPainter;

class ViewItem : public ContextItem
{
	Q_OBJECT

public:

	static const int AUDIOCLIPVIEW	= 1;
	static const int TRACKVIEW 	= 2;
	static const int SONGVIEW 	= 3;
	static const int PLUGINVIEW 	= 4;
	static const int FADEVIEW 	= 5;

	ViewItem(ViewPort* vp, ViewItem* parent, ContextItem* relatedContextItem=0);
	ViewItem(){}
	~ViewItem();

	virtual QRect draw(QPainter& painter) = 0;
	virtual QRect predraw(QPainter& painter);
	virtual QRect postdraw(QPainter& painter);
	
	virtual void force_redraw();
	
	bool visible() const;

	int get_z_order() const
	{
		return zOrder;
	}
	int type() const
	{
		return m_type;
	}

	virtual bool is_pointed() const
	{
		int x = cpointer().x();
		int y = cpointer().y();
		return ( ( x > geometry.x() ) &&
			( x < (geometry.x() + geometry.width()) ) &&
			( y > geometry.top() ) &&
			( y < geometry.bottom() ) );
	}

	ViewItem* get_parent() const
	{
		return m_parent;
	}
	ViewPort* get_viewport() const
	{
		return m_vp;
	}
	
	QRect get_geometry() const;
	

protected:
	ViewItem* 		m_parent;
	ViewPort* 		m_vp;
	QRect 			geometry;
	int 			zOrder;
	int			m_type;

	void set_geometry(int x, int y, int width, int height);

};


inline bool ViewItem::visible( ) const
{
	return ( (geometry.top() <= m_vp->height()) &&
			(geometry.left() <= m_vp->width()) &&
			(geometry.right() >= 0) );
}

inline QRect ViewItem::get_geometry( ) const {return geometry;}


#endif

//eof
