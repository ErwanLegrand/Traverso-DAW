/*
    Copyright (C) 2007 Remon Sijrier 
 
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

#include "MarkerDialog.h"

#include <ProjectManager.h>
#include <Project.h>
#include <Song.h>
#include <TimeLine.h>
#include <Marker.h>
#include <Utils.h>

MarkerDialog::MarkerDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
	
	m_project = pm().get_project();
	
	if (m_project) {
		setWindowTitle("Marker Editor - Project " + m_project->get_title());
		update_marker_treeview();
	}
	
	
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
}

void MarkerDialog::set_project(Project * project)
{
	m_project = project;
	
	if (! m_project) {
		// clear dialog stuff
		return;
	}
	
	// Fill dialog with marker stuff....
	update_marker_treeview();
}

void MarkerDialog::update_marker_treeview()
{
	// Just some sample function, remove/edit/delete as you like!!
	
	markersTreeWidget->clear();
	
	foreach(Song* song, m_project->get_songs()) {

		TimeLine* tl = song->get_timeline();
		
		foreach(Marker* marker, tl->get_markers()) {
			QString name = marker->get_description();
			QString pos = frame_to_smpte(marker->get_when(), m_project->get_rate());
			
			QTreeWidgetItem* item = new QTreeWidgetItem(markersTreeWidget);
			item->setTextAlignment(0, Qt::AlignHCenter);
			item->setTextAlignment(1, Qt::AlignHCenter);
			
			item->setText(0, pos);
			item->setText(1, name);
			
			// One can use the id to easily and very robustly get the marker via this id !!
			// See for example usage SongManagerDialog::songitem_clicked();
			// One also can get the creation time of the Marker via this id, with 
			// the convenience function QTime extract_date_time(qint64 id), declared in Utils.h
			item->setData(0, Qt::UserRole, marker->get_id());
		}
	}

}

//eof

