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

$Id: ClipsViewPort.cpp,v 1.12 2007/04/02 21:05:43 r_sijrier Exp $
*/

#include "ClipsViewPort.h"

#include "SongWidget.h"
#include "SongView.h"
#include "TrackView.h"
#include <libtraversocore.h>
#include <Import.h>
#include <CommandGroup.h>

#include <QScrollBar>
#include <QSet>
#include <QPaintEngine>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
		
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
	bool accepted = true;
	m_imports.clear();
	
	if (event->mimeData()->hasUrls()) {

		foreach(QUrl url, event->mimeData()->urls()) {
			QString fileName = url.toLocalFile();
			
			if (fileName.isEmpty()) {
				continue;
			}
			
			Import* import = new Import(fileName);
			m_imports.append(import);
			
			if (import->create_readsource() == -1) {
				foreach(Import* import, m_imports) {
					delete import;
				}
				m_imports.clear();
				accepted = false;
				break;
			}
		}
	}
	
	if (accepted) {
		event->acceptProposedAction();
	}
}

void ClipsViewPort::dropEvent( QDropEvent * event )
{
	if (!importTrack) {
		return;
	}

	CommandGroup* group = new CommandGroup(m_sw->get_song(), 
		       tr("Import %n audiofile(s)", "", m_imports.size()), true);
	
	foreach(Import* import, m_imports) {
		import->set_track(importTrack);
		group->add_command(import);
	}

	Command::process_command(group);
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
