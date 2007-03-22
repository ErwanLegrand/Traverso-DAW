/*
Copyright (C) 2006-2007 Remon Sijrier 

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

$Id: ClipsViewPort.cpp,v 1.10 2007/03/22 01:06:54 r_sijrier Exp $
*/

#include "ClipsViewPort.h"

#include "SongWidget.h"
#include "SongView.h"
#include "TrackView.h"
#include <libtraversocore.h>
#include <Import.h>

#include <QScrollBar>
#include <QSet>
#include <QPaintEngine>
		
#include <Debugger.h>
		
ClipsViewPort::ClipsViewPort(QGraphicsScene* scene, SongWidget* sw)
	: ViewPort(scene, sw)
{

	m_sw = sw;
	viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
	
// 	setViewportUpdateMode(SmartViewportUpdate);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void ClipsViewPort::get_pointed_context_items(QList<ContextItem* > &list)
{
	printf("ClipsViewPort::get_pointed_view_items\n");
	QList<QGraphicsItem *> itemlist = items(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
	foreach(QGraphicsItem* item, itemlist) {
		list.append((ViewItem*)item);
	}
	list.append(m_sw->get_songview());
}

void ClipsViewPort::resizeEvent( QResizeEvent * e )
{
	ViewPort::resizeEvent(e);
	m_sw->get_songview()->update_scrollbars();
}


void ClipsViewPort::paintEvent(QPaintEvent * e)
{
// 	printf("ClipsViewPort::paintEvent\n");
	QGraphicsView::paintEvent(e);
}


void ClipsViewPort::dragEnterEvent( QDragEnterEvent * event )
{
	QString fileName = event->mimeData()->text();
	// TODO Use QUrl instead
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

	ReadSource* source = resources_manager()->get_readsource(dir + name);
	if (source) {
		event->acceptProposedAction();
		delete source;
	}

}

void ClipsViewPort::dropEvent( QDropEvent * event )
{
	if (!importTrack) {
		return;
	}

	import = new Import(importTrack, importFileName);

	ie().process_command(import);
	import = 0;
}

void ClipsViewPort::dragMoveEvent( QDragMoveEvent * event )
{
	Project* project = pm().get_project();
	if (!project) {
		return;
	}
	
	Song* song = project->get_current_song();
	
	if (!song) {
		return;
	}
	
	importTrack = 0;
	
	// hmmm, code below is candidate for improvements...?
	
	// no mouse move events during D&D move events...
	// So we need to calculate the scene pos ourselves.
	QPointF mouseposTosScene = mapFromGlobal(QCursor::pos());
	
	QList<QGraphicsItem *> itemlist = items((int)mouseposTosScene.x(), (int)mouseposTosScene.y());
	foreach(QGraphicsItem* obj, itemlist) {
		TrackView* tv = dynamic_cast<TrackView*>(obj);
		if (tv) {
			importTrack = tv->get_track();
			return;
		}
	}
}

//eof
