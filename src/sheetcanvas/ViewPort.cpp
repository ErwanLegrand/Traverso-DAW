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
#include "TInputEventDispatcher.h"
#include "Themer.h"

#include "SheetView.h"
#include "ViewPort.h"
#include "ViewItem.h"
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


 *	\sa ContextPointer, InputEventDispatcher
 */

ViewPort::ViewPort(QGraphicsScene* scene, QWidget* parent)
	: QGraphicsView(scene, parent)
        , AbstractViewPort()
        , m_sv(0)
        , m_mode(0)
{
	PENTERCONS;
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	
	setOptimizationFlag(DontAdjustForAntialiasing);
        setOptimizationFlag(DontSavePainterState);
	setMouseTracking(true);
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
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		if (ke->key() == Qt::Key_Tab)
		{
			keyPressEvent(ke);
			return true;
		}
	}

	if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		if (ke->key() == Qt::Key_Tab)
		{
			keyReleaseEvent(ke);
			return true;
		}
        }

	return QGraphicsView::event(event);
}

void ViewPort::mouseMoveEvent(QMouseEvent* event)
{
        PENTER4;

        // tells the context pointer where we are, so command object can 'get' the
        // scene position in their jog function from cpointer, or view items that
        // accept mouse hover move 'events'
	cpointer().store_mouse_cursor_position(event->x(), event->y());

        if (cpointer().keyboard_only_input()) {
                event->accept();
                return;
        }

	// Qt generates mouse move events when the scrollbars move
	// since a mouse move event generates a jog() call for the 
	// active holding command, this has a number of nasty side effects :-(
	// For now, we ignore such events....
	if (event->pos() == m_oldMousePos) {
		return;
        }

        m_oldMousePos = event->pos();

	updateContext(event->pos());

	event->accept();
}

void ViewPort::updateContext(const QPoint &pos)
{
	QList<ViewItem*> mouseTrackingItems;

	if (!ied().is_holding())
	{
		QList<QGraphicsItem *> itemsUnderCursor = scene()->items(mapToScene(pos));
		QList<ContextItem*> activeContextItems;

		if (itemsUnderCursor.size())
		{
			foreach(QGraphicsItem* item, itemsUnderCursor)
			{
				if (ViewItem::is_viewitem(item))
				{
					ViewItem* viewItem = (ViewItem*)item;
					if (!viewItem->ignore_context())
					{
						activeContextItems.append(viewItem);
						if (viewItem->has_mouse_tracking())
						{
							mouseTrackingItems.append(viewItem);
						}
					}
				}
			}
		}
		else
		{
			// If no item is below the mouse, default to default cursor
			if (m_sv)
			{
				m_sv->set_cursor_shape(":/cursorFloat");
			}
		}

		// since sheetview has no bounding rect, and should always have 'active context'
		// add it if it's available
		if (m_sv) {
			activeContextItems.append(m_sv);
		}

		cpointer().set_active_context_items_by_mouse_movement(activeContextItems);

		if (m_sv)
		{
			m_sv->set_canvas_cursor_pos(mapToScene(pos));
		}
	}

	foreach(ViewItem* item, mouseTrackingItems) {
		item->mouse_hover_move_event();
	}
}


void ViewPort::tabletEvent(QTabletEvent * event)
{
	PMESG("ViewPort tablet event:: x, y: %d, %d", (int)event->x(), (int)event->y());
	PMESG("ViewPort tablet event:: high resolution x, y: %f, %f",
	      event->hiResGlobalX(), event->hiResGlobalY());
	cpointer().store_mouse_cursor_position((int)event->x(), (int)event->y());
	
	QGraphicsView::tabletEvent(event);
}

void ViewPort::enterEvent(QEvent* e)
{
	QGraphicsView::enterEvent(e);

	if (m_sv)
	{
		viewport()->setCursor(Qt::BlankCursor);
	}
	else
	{
		viewport()->setCursor(themer()->get_cursor("Default"));
	}

	cpointer().set_current_viewport(this);
	setFocus();
}

void ViewPort::leaveEvent(QEvent *)
{
	cpointer().set_current_viewport(0);
	// Force the next mouse move event to do something
        // even if the mouse didn't move, so switching viewports
        // does update the current context!
        m_oldMousePos = QPoint();
}

void ViewPort::keyPressEvent( QKeyEvent * e)
{
	ied().catch_key_press(e);
	e->accept();
}

void ViewPort::keyReleaseEvent( QKeyEvent * e)
{
	ied().catch_key_release(e);
	e->accept();
}

void ViewPort::mousePressEvent( QMouseEvent * e )
{
	ied().catch_mousebutton_press(e);
	e->accept();
}

void ViewPort::mouseReleaseEvent( QMouseEvent * e )
{
	ied().catch_mousebutton_release(e);
	e->accept();
}

void ViewPort::mouseDoubleClickEvent( QMouseEvent * e )
{
	ied().catch_mousebutton_press(e);
	e->accept();
}

void ViewPort::wheelEvent( QWheelEvent * e )
{
	ied().catch_scroll(e);
	e->accept();
}

void ViewPort::paintEvent( QPaintEvent* e )
{
// 	PWARN("ViewPort::paintEvent()");
	QGraphicsView::paintEvent(e);
}

void ViewPort::setCanvasCursor(const QString &cursor)
{
	viewport()->setCursor(Qt::BlankCursor);

	m_sv->set_cursor_shape(cursor);
}

void ViewPort::setCursorText( const QString & text )
{
	m_sv->set_edit_cursor_text(text);
}

void ViewPort::set_holdcursor_pos(QPointF pos)
{
	m_sv->set_canvas_cursor_pos(pos);
}

void ViewPort::set_current_mode(int mode)
{
	m_mode = mode;
}

void ViewPort::grab_mouse()
{
        viewport()->grabMouse();
}

void ViewPort::release_mouse()
{
        viewport()->releaseMouse();
	updateContext(m_oldMousePos);
}
