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

*/

#include "ClipsViewPort.h"

#include "SheetWidget.h"
#include "SheetView.h"
#include "AudioTrackView.h"
#include "ViewItem.h"
#include <libtraversocore.h>
#include <Import.h>
#include <CommandGroup.h>
#include "RemoveClip.h"

#include "AudioDevice.h"

#include <QScrollBar>
#include <QSet>
#include <QPaintEngine>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
		
#include <Debugger.h>
		
ClipsViewPort::ClipsViewPort(QGraphicsScene* scene, SheetWidget* sw)
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
}

void ClipsViewPort::resizeEvent( QResizeEvent * e )
{
	ViewPort::resizeEvent(e);
	m_sw->get_sheetview()->clipviewport_resize_event();
}


void ClipsViewPort::paintEvent(QPaintEvent * e)
{
	QGraphicsView::paintEvent(e);
}


void ClipsViewPort::dragEnterEvent( QDragEnterEvent * event )
{
	m_imports.clear();
	m_resourcesImport.clear();
	
	// let's see if the D&D was from the resources bin.
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
		QByteArray encodedData = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
		QDataStream stream(&encodedData, QIODevice::ReadOnly);
		int r, c;
		QMap<int, QVariant> v;
		
		while (!stream.atEnd()) {
			stream >> r >> c >> v;
			qint64 id = v.value(Qt::UserRole).toLongLong();
			if (!id) {
				continue;
			}
			m_resourcesImport.append(id);
		}
	}
	
	// and who knows, it could have been a D&D drop from a filemanager...
	if (event->mimeData()->hasUrls()) {

		foreach(QUrl url, event->mimeData()->urls()) {
			QString fileName = url.toLocalFile();
			
			if (fileName.isEmpty()) {
				continue;
			}
			
			Import* import = new Import(fileName);
			m_imports.append(import);
			
			// If a readsource fails to init, the D&D should be
			// marked as failed, cleanup allready created imports,
			// and clear the import list.
			if (import->create_readsource() == -1) {
				foreach(Import* import, m_imports) {
					delete import;
				}
				m_imports.clear();
				break;
			}
		}
	}
	
	if (m_imports.size() || m_resourcesImport.size()) {
		event->acceptProposedAction();
	}
}

void ClipsViewPort::dropEvent(QDropEvent* event )
{
	PENTER;
	Q_UNUSED(event)
	
	if (!importTrack) {
		return;
	}

	CommandGroup* group = new CommandGroup(m_sw->get_sheet(), 
		       tr("Import %n audiofile(s)", "", m_imports.size() + m_resourcesImport.size()), true);
	
	TimeRef startpos = TimeRef(mapFromGlobal(QCursor::pos()).x() * m_sw->get_sheetview()->timeref_scalefactor);
	
	foreach(qint64 id, m_resourcesImport) {
		AudioClip* clip = resources_manager()->get_clip(id);
		if (clip) {
			bool hadSheet = clip->has_sheet();
			clip->set_sheet(m_sw->get_sheet());
			clip->set_track(importTrack);
			if (!hadSheet) {
				clip->set_state(clip->get_dom_node());
			}
			clip->set_track_start_location(startpos);
			startpos = clip->get_track_end_location();
			AddRemoveClip* arc = new AddRemoveClip(clip, AddRemoveClip::ADD);
			group->add_command(arc);
			continue;
		}
		ReadSource* source = resources_manager()->get_readsource(id);
		if (source) {
			clip = resources_manager()->new_audio_clip(source->get_short_name());
			resources_manager()->set_source_for_clip(clip, source);
			clip->set_sheet(importTrack->get_sheet());
			clip->set_track(importTrack);
			clip->set_track_start_location(startpos);
			startpos = clip->get_track_end_location();
			AddRemoveClip* arc = new AddRemoveClip(clip, AddRemoveClip::ADD);
			group->add_command(arc);
		}
	}
	
	bool firstItem = true;
	foreach(Import* import, m_imports) {
		import->set_track(importTrack);
		if (firstItem) {
			// Place first item at cursor, others at end of track.
			import->set_position(startpos);
			firstItem = false;
		}
		group->add_command(import);
	}

	Command::process_command(group);
}

void ClipsViewPort::dragMoveEvent( QDragMoveEvent * event )
{
	Q_UNUSED(event)
			
	Project* project = pm().get_project();
	if (!project) {
		return;
	}
	
	Sheet* sheet = project->get_current_sheet();
	
	if (!sheet) {
		return;
	}
	
	importTrack = 0;
	
	// hmmm, code below is candidate for improvements...?
	
	// no mouse move events during D&D move events...
	// So we need to calculate the scene pos ourselves.
	QPointF mouseposTosScene = mapFromGlobal(QCursor::pos());
	
	QList<QGraphicsItem *> itemlist = items((int)mouseposTosScene.x(), (int)mouseposTosScene.y());
	foreach(QGraphicsItem* obj, itemlist) {
		AudioTrackView* tv = dynamic_cast<AudioTrackView*>(obj);
		if (tv) {
			importTrack = tv->get_track();
			return;
		}
	}
}

