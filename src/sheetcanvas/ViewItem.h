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
#include <Utils.h>

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
#if QT_VERSION >= 0x040600
        Q_INTERFACES(QGraphicsItem)
#endif
	
public:

        ViewItem(ViewItem* parentViewItem=0, ContextItem* parentContext=0) :
        ContextItem(parentViewItem)
        , QGraphicsItem(parentViewItem)
        {
                set_context_item(parentContext);
                m_parentViewItem = parentViewItem;
                m_hasMouseTracking = false;
		m_ignoreContext = false;
        }

        virtual ~ViewItem() {}
	
	enum {Type = UserType + 1};
	
	QRectF boundingRect() const;
	virtual void calculate_bounding_rect() {
		for (int i=0; i< QGraphicsItem::children().size(); ++i) {
			QGraphicsItem* item = QGraphicsItem::children().at(i);
			if (is_viewitem(item)) {
                                (qgraphicsitem_cast<ViewItem*>(item))->calculate_bounding_rect();
			}
		}
	}

	virtual int type() const;
	bool ignore_context() const {return m_ignoreContext;}
	virtual int get_height() const {return (int)m_boundingRect.height();}
	virtual QString get_name() const {return m_parentViewItem ? m_parentViewItem->get_name() : "";}
	
	/**
	 *      Reimplement and call update() in the reimplementation
	 *	to make the theme change visible.
	 */
        virtual void load_theme_data() {}
        virtual void mouse_hover_move_event() {}
	
	SheetView* get_sheetview() const {return m_sv;}
	
	static bool is_viewitem(QGraphicsItem* item) {
		return item->type() == Type;
	}

        bool has_mouse_tracking() const {return m_hasMouseTracking;}
		

protected:

	SheetView* 	m_sv;
	ViewItem*	m_parentViewItem;
	QRectF		m_boundingRect;
        bool            m_hasMouseTracking;
	bool		m_ignoreContext;
};

inline QRectF ViewItem::boundingRect() const {return m_boundingRect;}
inline int ViewItem::type() const {return Type;}

#endif

//eof
