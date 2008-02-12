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

#include <QMouseEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QRect>
#include <QPainter>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QStyleOptionGraphicsItem>
#include <QApplication>
#include <Utils.h>
#include "InputEngine.h"
#include "Themer.h"

#include "ViewPort.h"
#include "ContextPointer.h"

#include "Import.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/**
 * \class ViewPort
 * \brief An Interface class to create Contextual, or so called 'Soft Selection' enabled Widgets.

	The ViewPort class inherits QGraphicsView, and thus is a true Canvas type of Widget.<br />
	Reimplement ViewPort to create a 'Soft Selection' enabled widget. You have to create <br />
	a QGraphicsScene object yourself, and set it as the scene the ViewPort visualizes.

	ViewPort should be used to visualize 'core' data objects. This is done by creating a <br />
	ViewItem object for each core class that has to be visualized. The naming convention <br />
	for classes that inherit ViewItem is: core class name + View.<br />
	E.g. the ViewItem class that represents an AudioClip should be named AudioClipView.

	All keyboard and mouse events by default are propagated to the InputEngine, which in <br />
	turn will parse the events. In case the event sequence was recognized by the InputEngine <br />
	it will ask a list of (pointed) ContextItem's from ContextPointer, which in turns <br />
	call's the pure virtual function get_pointed_context_items(), which you have to reimplement.<br />
	In the reimplemented function, you have to fill the supplied list with ViewItems that are <br />
	under the mouse cursor, and if needed, ViewItem's that _always_ have to be taken into account. <br />
	One can use the convenience functions of QGraphicsView for getting ViewItem's under the mouse cursor!

	Since there can be a certain delay before a key sequence has been verified, the ContextPointer <br />
	stores the position of the first event of a new key fact. This improves the pointed ViewItem <br />
	detection a lot in case the mouse is moved during a key sequence.<br />
	You should use these x/y coordinates in the get_pointed_context_items() function, see:<br />
	ContextPointer::on_first_input_event_x(), ContextPointer::on_first_input_event_y()


 *	\sa ContextPointer, InputEngine
 */



ViewPort::ViewPort(QWidget* parent)
	: QGraphicsView(parent)
	, m_mode(0)
{
	PENTERCONS;
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

ViewPort::ViewPort(QGraphicsScene* scene, QWidget* parent)
	: QGraphicsView(scene, parent)
	, m_mode(0)
{
	PENTERCONS;
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	
#if QT_VERSION >= 0x040300
#if !defined (Q_WS_WIN)
	setOptimizationFlag(DontAdjustForAntialiasing);
#endif
	setOptimizationFlag(DontSavePainterState);
	setOptimizationFlag(DontClipPainter);
#endif

	m_holdcursor = new HoldCursor(this);
	scene->addItem(m_holdcursor);
	m_holdcursor->hide();
	// m_holdCursorActive is a replacement for m_holdcursor->isVisible()
	// in mouseMoveEvents, which crashes when a hold action in one viewport
	// ends with the mouse upon a different viewport.
	// Should get a proper fix ?
	m_holdCursorActive = false;
}

ViewPort::~ViewPort()
{
	PENTERDES;
	
	cpointer().set_current_viewport((ViewPort*) 0);
}

bool ViewPort::event(QEvent * event)
{
	// We want Tab events also send to the InputEngine
	// so treat them as 'normal' key events.
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		if (ke->key() == Qt::Key_Tab) {
			keyPressEvent(ke);
			return true;
		}
	}
	if (event->type() == QEvent::KeyRelease) {
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		if (ke->key() == Qt::Key_Tab) {
			keyReleaseEvent(ke);
			return true;
		}
	}
	return QGraphicsView::event(event);
}

void ViewPort::mouseMoveEvent(QMouseEvent* event)
{
	PENTER3;
	// Qt generates mouse move events when the scrollbars move
	// since a mouse move event generates a jog() call for the 
	// active holding command, this has a number of nasty side effects :-(
	// For now, we ignore such events....
	if (event->pos() == m_oldMousePos) {
		return;
	}
	
	QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
	mouseEvent.setWidget(viewport());
// 	mouseEvent.setButtonDownScenePos(d->mousePressButton, d->mousePressScenePoint);
// 	mouseEvent.setButtonDownScreenPos(d->mousePressButton, d->mousePressScreenPoint);
	mouseEvent.setScenePos(mapToScene(event->pos()));
	mouseEvent.setScreenPos(event->globalPos());
	mouseEvent.setLastScenePos(lastMouseMoveScenePoint);
	mouseEvent.setLastScreenPos(mapFromScene(lastMouseMoveScenePoint));
	mouseEvent.setButtons(event->buttons());
	mouseEvent.setButton(event->button());
	mouseEvent.setModifiers(event->modifiers());
	lastMouseMoveScenePoint = mouseEvent.scenePos();
	mouseEvent.setAccepted(false);
	
	m_oldMousePos = event->pos();
	
	if (!ie().is_holding()) {
		QList<QGraphicsItem *> itemsUnderCursor = scene()->items(mapToScene(event->pos()));
		if (itemsUnderCursor.size()) {
			itemsUnderCursor.first()->setCursor(itemsUnderCursor.first()->cursor());
		} else {
			// If no item is below the mouse, default to default cursor
			viewport()->setCursor(themer()->get_cursor("Default"));
		}
		QApplication::sendEvent(scene(), &mouseEvent);
	} else {
		// It can happen that a cursor is set for a newly created viewitem
		// but we don't want that when the holdcursor is set!
		// So force it back to be a blankcursor.
		if (m_holdCursorActive /* was m_holdcursor->isVisible() */ && viewport()->cursor().shape() != Qt::BlankCursor) {
			viewport()->setCursor(Qt::BlankCursor);
		}
	}

// 	QGraphicsView::mouseMoveEvent(event);
	cpointer().set_point(event->x(), event->y());
	event->accept();
}

void ViewPort::tabletEvent(QTabletEvent * event)
{
	PMESG("ViewPort tablet event:: x, y: %d, %d", (int)event->x(), (int)event->y());
	PMESG("ViewPort tablet event:: high resolution x, y: %d, %d", 
	      (int)event->hiResGlobalX(), (int)event->hiResGlobalY());
	cpointer().set_point((int)event->x(), (int)event->y());
	
	QGraphicsView::tabletEvent(event);
}

void ViewPort::enterEvent(QEvent* e)
{
	QGraphicsView::enterEvent(e);
	cpointer().set_current_viewport(this);
	setFocus();
}

void ViewPort::leaveEvent ( QEvent * event )
{
	QGraphicsView::leaveEvent(event);
	// There can be many reasons for a leave event, sometimes
	// this leaves the engine in a non-cleared state, e.g. modifier
	// keys still can be active!! So we reset those manually here.
	ie().clear_modifier_keys();
}

void ViewPort::keyPressEvent( QKeyEvent * e)
{
	ie().catch_key_press(e);
	e->accept();
}

void ViewPort::keyReleaseEvent( QKeyEvent * e)
{
	ie().catch_key_release(e);
	e->accept();
}

void ViewPort::mousePressEvent( QMouseEvent * e )
{
	ie().catch_mousebutton_press(e);
	e->accept();
}

void ViewPort::mouseReleaseEvent( QMouseEvent * e )
{
	ie().catch_mousebutton_release(e);
	e->accept();
}

void ViewPort::mouseDoubleClickEvent( QMouseEvent * e )
{
	ie().catch_mousebutton_doubleclick(e);
	e->accept();
}

void ViewPort::wheelEvent( QWheelEvent * e )
{
	ie().catch_scroll(e);
	e->accept();
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
	m_holdcursor->reset();
	m_holdCursorActive = false;
}

void ViewPort::set_holdcursor( const QString & cursorName )
{
	viewport()->setCursor(Qt::BlankCursor);
	
	if (!m_holdCursorActive) {
		m_holdcursor->setPos(cpointer().scene_pos());
		m_holdcursor->show();
	}
	m_holdcursor->set_type(cursorName);
	m_holdCursorActive = true;
}

void ViewPort::set_holdcursor_text( const QString & text )
{
	m_holdcursor->set_text(text);
	// TODO Find out why we have to call set_holdcursor_pos() here 
	// AGAIN when it allready has been called in for example MoveClip::jog()
	// to AVOID jitter of the hold cursor text item when the cursor is 
	// out of the viewports range
	set_holdcursor_pos(mapToScene(cpointer().pos()).toPoint());
}

void ViewPort::set_holdcursor_pos(QPoint pos)
{
	m_holdcursor->set_pos(pos);
}

void ViewPort::set_current_mode(int mode)
{
	m_mode = mode;
}



/**********************************************************************/
/*                      HoldCursor                                    */
/**********************************************************************/


HoldCursor::HoldCursor(ViewPort* vp)
	: m_vp(vp)
{
	m_textItem = new QGraphicsTextItem(this);
	m_textItem->setFont(themer()->get_font("ViewPort:fontscale:infocursor"));

	setZValue(200);
}

HoldCursor::~ HoldCursor( )
{
}

void HoldCursor::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);
	Q_UNUSED(option);

	painter->drawPixmap(0, 0, m_pixmap);
}


void HoldCursor::set_text( const QString & text )
{
	m_text = text;
	
	if (!m_text.isEmpty()) {
		QString html = "<html><body bgcolor=ghostwhite>" + m_text + "</body></html>";
		m_textItem->setHtml(html);
		m_textItem->show();
	} else {
		m_textItem->hide();
	}
}

void HoldCursor::set_type( const QString & type )
{
	m_pixmap = find_pixmap(type);
	int x = (int) pos().x();
	int y = (int) pos().y();
	setPos(x - m_pixmap.width() / 2, y - m_pixmap.height() / 2);
}

QRectF HoldCursor::boundingRect( ) const
{
	return QRectF(0, 0, 130, 40);
}

void HoldCursor::reset()
{
	m_text = "";
	m_textItem->hide();
}

void HoldCursor::set_pos(QPoint p)
{
	int x = m_vp->mapFromScene(pos()).x();
	int y = m_vp->mapFromScene(pos()).y();
	int yoffset = 0;
	
	if (y < 0) {
		yoffset = - y;
	} else if (y > m_vp->height() - m_pixmap.height()) {
		yoffset = m_vp->height() - y - m_pixmap.height();
	}
	
	int diff = m_vp->width() - (x + m_pixmap.width() + 8);
	
	if (diff < m_textItem->boundingRect().width()) {
		m_textItem->setPos(diff - m_pixmap.width(), yoffset);
	} else if (x < -m_pixmap.width()) {
		m_textItem->setPos(8 - x, yoffset);
	} else {
		m_textItem->setPos(m_pixmap.width() + 8, yoffset);
	}
	
	setPos(p);
}


