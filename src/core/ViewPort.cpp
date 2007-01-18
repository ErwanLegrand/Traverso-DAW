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

$Id: ViewPort.cpp,v 1.4 2007/01/18 11:33:38 r_sijrier Exp $
*/

#include <QMouseEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QRect>
#include <QPainter>
#include <QPixmap>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <Utils.h>

#include "ViewPort.h"
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
        
	setAttribute(Qt::WA_OpaquePaintEvent);
}

ViewPort::ViewPort(QGraphicsScene* scene, QWidget* parent)
	: QGraphicsView(scene, parent)
{
	PENTERCONS;
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	
	m_holdcursor = new HoldCursor();
	scene->addItem(m_holdcursor);
	m_holdcursor->hide();

//         setAttribute(Qt::WA_OpaquePaintEvent);
}

ViewPort::~ViewPort()
{
	PENTERDES;
	
	cpointer().set_current_viewport((ViewPort*) 0);
}


void ViewPort::mouseMoveEvent(QMouseEvent* e)
{
	PENTER3;
// 	printf("\nViewPort::mouseMoveEvent\n");
// 	if (!ie().is_holding())
		QGraphicsView::mouseMoveEvent(e);
	cpointer().set_point(e->x(), e->y());
}

void ViewPort::resizeEvent(QResizeEvent* e)
{
	PENTER3;
	QGraphicsView::resizeEvent(e);
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


void ViewPort::mousePressEvent( QMouseEvent * e )
{
	ie().catch_mousebutton_press(e);
}

void ViewPort::mouseReleaseEvent( QMouseEvent * e )
{
	ie().catch_mousebutton_release(e);
}

void ViewPort::mouseDoubleClickEvent( QMouseEvent * e )
{
	ie().catch_mousebutton_doubleclick(e);
}

void ViewPort::wheelEvent( QWheelEvent * e )
{
	ie().catch_scroll(e);
	if ( ! e->isAccepted() ) {
		QGraphicsView::wheelEvent(e);
	}
}

void ViewPort::paintEvent( QPaintEvent* e )
{
// 	PWARN("ViewPort::paintEvent()");
	QGraphicsView::paintEvent(e);
}


void ViewPort::reset_cursor( )
{
	viewport()->unsetCursor();
	m_holdcursor->hide();
}

void ViewPort::set_hold_cursor( const QString & cursorName )
{
	viewport()->setCursor(Qt::BlankCursor);
	
	m_holdcursor->setPos(cpointer().scene_pos());
	m_holdcursor->set_type(cursorName);
	m_holdcursor->show();
}

void ViewPort::set_hold_cursor_text( const QString & text )
{
	m_holdcursor->set_text(text);
}



/**********************************************************************/
/*                      HoldCursor                                    */
/**********************************************************************/


HoldCursor::HoldCursor()
{
	setZValue(200);
}

HoldCursor::~ HoldCursor( )
{
}

void HoldCursor::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);

	printf("exposed rect is: x=%f, y=%f, w=%f, h=%f\n", option->exposedRect.x(), option->exposedRect.y(), option->exposedRect.width(), option->exposedRect.height());
	
	painter->drawPixmap(0, 0, m_pixmap);
	
	if (!m_text.isEmpty()) {
		QFontMetrics fm(QFont("Bitstream Vera Sans", 11));
		int width = fm.width(m_text) + 4;
		int height = fm.height();
		QRect textArea = QRect(m_pixmap.width() + 10, m_pixmap.height() / 4, width, height);
		painter->setFont(QFont("Bitstream Vera Sans", 11));
		painter->fillRect(textArea, QBrush(Qt::white));
		painter->drawText(textArea, m_text);
	}
}


void HoldCursor::set_text( const QString & text )
{
	m_text = text;
	update();
}

void HoldCursor::set_type( const QString & type )
{
	m_pixmap = find_pixmap(type);
}

QRectF HoldCursor::boundingRect( ) const
{
	return QRectF(0, 0, 120, 40);
}

//eof
