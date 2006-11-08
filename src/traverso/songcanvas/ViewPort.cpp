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

$Id: ViewPort.cpp,v 1.1 2006/11/08 14:45:22 r_sijrier Exp $
*/

#include <libtraversocore.h>
#include <ReadSource.h>

#include <QMouseEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QRect>
#include <QtAlgorithms>
#include <QPainter>
#include <QPixmap>
#include <QWidget>
#include <QFile>

#include "ViewPort.h"
#include "ViewItem.h"
#include "ContextPointer.h"

#include "Import.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ViewPort::ViewPort(QWidget* parent)
	: QGraphicsView(parent)
{
	PENTERCONS;
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

ViewPort::ViewPort(QGraphicsScene* scene, QWidget* parent)
	: QGraphicsView(scene, parent)
{
	PENTERCONS;
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

ViewPort::~ViewPort()
{
	PENTERDES;
	
	cpointer().set_current_viewport((ViewPort*) 0);
}


void ViewPort::mouseMoveEvent(QMouseEvent* e)
{
	PENTER3;
	if (!ie().is_holding())
		QGraphicsView::mouseMoveEvent(e);
	cpointer().set_point(e->x(), e->y());
}

void ViewPort::resizeEvent(QResizeEvent* e)
{
	PENTER3;
	QGraphicsView::resizeEvent(e);
	QApplication::syncX ();
// 	QCoreApplication::flush ();
}

void ViewPort::enterEvent(QEvent* e)
{
	QGraphicsView::enterEvent(e);
	cpointer().set_current_viewport(this);
}


void ViewPort::leaveEvent(QEvent* e)
{
	QGraphicsView::leaveEvent(e);
}


void ViewPort::paintEvent( QPaintEvent* e )
{
// 	PWARN("ViewPort::paintEvent()");
	QGraphicsView::paintEvent(e);
}


void ViewPort::get_pointed_view_items( QList< ViewItem * > & list )
{
	printf("ViewPort::get_pointed_view_items\n");
	QList<QGraphicsItem *> itemlist = items(cpointer().x(), cpointer().y());
	printf("itemlist size is %d\n", itemlist.size());
	foreach(QGraphicsItem* item, itemlist) {
		ViewItem* view = qgraphicsitem_cast<ViewItem*>(item);
		if (view) {
			list.append(view);
		}
	}
	PENTER3;
}

void ViewPort::reset_context( )
{
/*	if (m_holdCursor) {
		QRect holdCursorGeo = m_holdCursor->get_geometry();
		for (int i=0; i<viewItemList.size(); ++i) {
			ViewItem* view = viewItemList.at(i);
			if (holdCursorGeo.intersects(view->get_geometry())) {
				view->force_redraw();
			}
		}
		delete m_holdCursor;
		m_holdCursor = 0;
	}*/
	
	viewport()->unsetCursor();
}

void ViewPort::set_hold_cursor( const QString & cursorName )
{
/*	if (m_holdCursor) {
		PERROR("Setting hold cursor, but it allready exist!!");
		return;
	}*/
	
	viewport()->setCursor(Qt::BlankCursor);
/*	m_holdCursor = new HoldCursor(this, QPoint(cpointer().x(), cpointer().y()), cursorName);
	
	update();*/
}

void ViewPort::set_hold_cursor_text( const QString & text )
{
/*	if (!m_holdCursor) {
		PERROR("Cannot set text for hold cursor, since it DOESN'T EXIST!!!");
		return;
	}
	*/
// 	m_holdCursor->set_text(text);
}

//eof
