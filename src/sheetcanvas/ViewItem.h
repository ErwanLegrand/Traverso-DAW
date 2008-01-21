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

*/

#ifndef VIEW_ITEM_H
#define VIEW_ITEM_H

#include <ContextItem.h>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QCursor>
#include <Utils.h>
#include <Themer.h>

class SheetView;
		
// Canvas width should be 2^31, but it doesn't work ok
// 2^30 works ok, so let's use that, still gives a lot 
// of headroom for real large recordings
#if ! defined (Q_WS_WIN)
#define MAX_CANVAS_WIDTH 1073741824
#define MAX_CANVAS_HEIGHT 1073741824
#else		
#define MAX_CANVAS_WIDTH 107374182
#define MAX_CANVAS_HEIGHT 107374182
#endif


class ViewItem : public ContextItem, public QGraphicsItem
{
	Q_OBJECT
	
public:

	ViewItem(ViewItem* parentViewItem=0, ContextItem* parentContext=0);
	~ViewItem() {};
	
	enum {Type = UserType + 1};
	
	QRectF boundingRect() const;
	virtual void calculate_bounding_rect() {
		for (int i=0; i< QGraphicsItem::children().size(); ++i) {
			QGraphicsItem* item = QGraphicsItem::children().at(i);
			if (is_viewitem(item)) {
				((ViewItem*)item)->calculate_bounding_rect();
			}
		}
	}
	void prepare_geometry_change() {prepareGeometryChange();}
	virtual int get_childview_y_offset() const {return 0;}
	virtual int type() const;
	virtual int get_height() const {return (int)m_boundingRect.height();}
	
	/**
	 *      Reimplement and call update() in the reimplementation
	 *	to make the theme change visible.
	 */
	virtual void load_theme_data() {};
	
	SheetView* get_sheetview() const {return m_sv;}
	
	static bool is_viewitem(QGraphicsItem* item) {
		return item->type() == Type;
	}
		

protected:

	SheetView* 	m_sv;
	ViewItem*	m_parentViewItem;
	QRectF		m_boundingRect;
};

inline QRectF ViewItem::boundingRect() const {return m_boundingRect;}
inline int ViewItem::type() const {return Type;}

#endif

//eof
