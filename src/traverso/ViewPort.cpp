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

$Id: ViewPort.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
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
#include "SongView.h"
#include "ViewItem.h"
#include "ContextPointer.h"

#include "Import.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ViewPort::ViewPort(QWidget* widget)
		: QWidget(widget)
{
	PENTERCONS;
	pixmap = QPixmap();
	import = 0;
	importTrack = 0;

	setMouseTracking(true);
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAcceptDrops(true);

	setMinimumSize(100, 50);
	
	connect(pm().get_project(), SIGNAL(destroyed(QObject* )), this, SLOT (destroy(QObject* ) ));
}

ViewPort::~ViewPort()
{
	PENTERDES;
/*	foreach(ViewItem* item, predrawItemList)
		delete item;*/
		
/*	foreach(ViewItem* item, viewItemList)
		delete item;*/
		
	cpointer().set_current_viewport((ViewPort*) 0);
}


void ViewPort::mouseMoveEvent(QMouseEvent* e)
{
	PENTER3;
	cpointer().set_point(e->x(), e->y());
	emit pointChanged();
}

void ViewPort::resizeEvent(QResizeEvent* )
{
	PENTER;
	if (pixmap.size() != size())
		pixmap = QPixmap(size());
	emit resized();
}

void ViewPort::enterEvent(QEvent* )
{
	cpointer().set_current_viewport(this);
}


void ViewPort::leaveEvent(QEvent *)
{}

void ViewPort::paintEvent( QPaintEvent *  )
{
	QPainter p(&pixmap);
	QPainter pixmapPainter(this);

	// Sort all the ViewItems in zOrder!
	qSort(repaintViewItemList.begin(), repaintViewItemList.end(), ViewItem::smaller);


	if (predrawItemList.size() > 0) {
		for (int i=0; i < predrawItemList.size(); ++i) {
			ViewItem* view = predrawItemList.at(i);
			view->predraw(p);
		}
	}

	if (repaintViewItemList.size() > 0) {
		for (int i=0; i < repaintViewItemList.size(); ++i) {
			ViewItem* view = repaintViewItemList.at(i);
			view->draw(p);
		}
		clear_repaintviewitemlist();
	}


	if (postdrawItemList.size() > 0) {
		for (int i=0; i < postdrawItemList.size(); ++i) {
			ViewItem* view = postdrawItemList.at(i);
			view->postdraw(p);
		}
	}


	pixmapPainter.drawPixmap(0, 0, pixmap);
}


void ViewPort::clear_repaintviewitemlist( )
{
	repaintViewItemList.clear();
}


void ViewPort::schedule_for_repaint( ViewItem * view )
{
	if (!repaintViewItemList.contains(view))
		repaintViewItemList.append(view);
	else {
		// 		PWARN("ViewItem %s allready in ViewPort updatelist!", view->metaObject()->className());
	}

	/*	PWARN("View classname is: %s", view->metaObject()->className());
		if (view->get_context())
			{
			ContextItem* con = view->get_context();
			QMetaObject::invokeMethod(con,"test", Qt::DirectConnection);
			PWARN("Context classname is: %s", con->metaObject()->className());
			if(con->get_context())
				PWARN("Related Context classname is: %s", con->get_context()->metaObject()->className());
			}*/

	update();
}

void ViewPort::register_predraw_item( ViewItem * item )
{
	predrawItemList.append(item);
}

void ViewPort::register_postdraw_item( ViewItem * item )
{
	postdrawItemList.append(item);
}

void ViewPort::get_pointed_view_items( QList< ViewItem * > & list )
{
	PENTER3;
	for (int i=0; i < viewItemList.size(); ++i) {
		ViewItem* item = viewItemList.at(i);
		if (item->is_pointed())
			list.append(item);
	}
	qSort(list.begin(), list.end(), ViewItem::greater);
}

void ViewPort::unregister_viewitem( ViewItem * item )
{
	int index = viewItemList.indexOf(item);
	viewItemList.takeAt(index);

	// Also remove it from repaintViewItemList if it's in there.
	index = repaintViewItemList.indexOf(item);
	if (index > 0)
		repaintViewItemList.takeAt(index);
}

void ViewPort::dragEnterEvent( QDragEnterEvent * event )
{
	QString fileName = event->mimeData()->text();
	int begin = fileName.indexOf("file:///");
	if(begin != 0) {
		return;
	}

	begin = 7;

	int length = fileName.length();
	importFileName = fileName.mid(begin, length);

	begin = importFileName.lastIndexOf("/") + 1;

	length = importFileName.length();
	QString dir = importFileName.left(begin);
	QString name = importFileName.right(length - begin);

	ReadSource* source = new ReadSource(0, dir, name);
	if (source->init() > 0) {
		event->acceptProposedAction();
	}

	delete source;
}

void ViewPort::dropEvent( QDropEvent * event )
{
	if (!importTrack)
		return;

	import = new Import(importTrack, importFileName);

	if (import) {
		if (import->prepare_actions()) {
			import->do_action();
			import->push_to_history_stack();
			event->acceptProposedAction();
		} else {
			delete import;
			import = 0;
		}
	}
}

void ViewPort::dragMoveEvent( QDragMoveEvent * event )
{
	Project* project = pm().get_project();
	if (!project)
		return;
	Song* song = project->get_current_song();
	if (!song)
		return;
	importTrack = song->get_track_under_y(event->answerRect().y());
	if (!importTrack)
		return;
}

void ViewPort::register_viewitem( ViewItem * item )
{
	viewItemList.append(item);
}

void ViewPort::destroy( QObject * )
{
//	FIXME ViewPort is NOT deleted at the correct time, however, if
//			I use this method of deletion, the new ViewPorts don't show up anymore!
// 	PWARN("deleting myself");
// 	delete this;
}
//eof

