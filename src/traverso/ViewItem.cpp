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

$Id: ViewItem.cpp,v 1.4 2006/09/07 09:36:52 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include <QPainter>

#include "ViewItem.h"
#include "SongView.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
// #include "Debugger.h"

ViewItem::ViewItem(ViewPort* vp, ViewItem* parent, ContextItem* relatedContextItem)
		: ContextItem(vp), m_parent(parent), m_vp(vp)
{
	if (m_parent)
		zOrder = m_parent->get_z_order() + 1;
	else
		zOrder = 0;

	m_child = 0;
	contextMenu = 0;
	
	set_context_item(relatedContextItem);
	
	m_vp->schedule_for_repaint(this);
	m_vp->register_viewitem(this);
}


ViewItem::~ ViewItem()
{
	// 	PENTERDES2;
	if (contextMenu)
		delete contextMenu;
}

bool ViewItem::visible( )
{
	return ( (geometry.top() <= m_vp->height()) &&
		(geometry.left() <= m_vp->width()) &&
		(geometry.right() >= 0) );
}

QRect ViewItem::predraw( QPainter &  )
{
	return QRect();
}

QRect ViewItem::postdraw( QPainter &  )
{
	return QRect();
}

void ViewItem::set_geometry( int x, int y, int width, int height )
{
	geometry.setTop(y);
	geometry.setBottom(y + height);
	geometry.setLeft(x);
	geometry.setRight(x + width);
}

Command * ViewItem::show_context_menu( )
{
	if (m_child)
		contextMenu->exec(QCursor::pos());
	return 0;
}

void ViewItem::process_menu_action( QAction* qaction )
{
	QString name = (qaction->data()).toString();
	ie().broadcast_action_from_contextmenu(name);
}

void ViewItem::init_context_menu( ViewItem * item )
{
	contextMenu = new QMenu();

	m_child = item;
	connect(contextMenu, SIGNAL(triggered ( QAction* )), this, SLOT(process_menu_action( QAction* )));
	IEActionList = ie().get_contextitem_actionlist( item );
	qSort(IEActionList.begin(), IEActionList.end(), IEAction::smaller);

	QAction* action;
	foreach(IEAction* ieaction, IEActionList) {
		QString text = QString(ieaction->keySequence + "  " + ieaction->name);
		action = new QAction(this);
		action->setText(text);
		action->setData(ieaction->name);
		contextMenu->addAction(action);
	}
}


//eof
