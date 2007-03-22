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
#include <QDebug>

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
	connect(lineEditTitle, SIGNAL(textEdited(const QString &)), this, SLOT(description_changed(const QString &)));
	connect(markersTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		 this, SLOT(item_changed(QTreeWidgetItem *, QTreeWidgetItem *)));
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
	markersTreeWidget->clear();

	foreach(Song* song, m_project->get_songs()) {

		TimeLine* tl = song->get_timeline();
		
		foreach(Marker* marker, tl->get_markers()) {
			QString name = marker->get_description();
			QString pos = frame_to_smpte(marker->get_when(), m_project->get_rate());

			QTreeWidgetItem* item = new QTreeWidgetItem(markersTreeWidget);
			item->setText(0, pos);
			item->setText(1, name);

			// One can use the id to easily and very robustly get the marker via this id !!
			// See for example usage SongManagerDialog::songitem_clicked();
			// One also can get the creation time of the Marker via this id, with 
			// the convenience function QTime extract_date_time(qint64 id), declared in Utils.h
			item->setData(0, Qt::UserRole, marker->get_id());
		}
	}

	markersTreeWidget->sortItems(0, Qt::AscendingOrder);
}

void MarkerDialog::item_changed(QTreeWidgetItem * current, QTreeWidgetItem * previous)
{
	m_marker = get_marker(current->data(0, Qt::UserRole).toLongLong());

	if (!m_marker) {
		return;
	}

	if (previous) {
		Marker *marker = get_marker(previous->data(0, Qt::UserRole).toLongLong());
		marker->set_description(lineEditTitle->text());
		marker->set_performer(lineEditPerformer->text());
		marker->set_composer(lineEditComposer->text());
		marker->set_songwriter(lineEditSongwriter->text());
		marker->set_arranger(lineEditArranger->text());
		marker->set_message(lineEditMessage->text());
		marker->set_isrc(lineEditIsrc->text());
		marker->set_preemphasis(checkBoxPreEmph->isChecked());
		marker->set_copyprotect(checkBoxCopy->isChecked());
	}

	lineEditTitle->setText(m_marker->get_description());
	lineEditPerformer->setText(m_marker->get_performer());
	lineEditComposer->setText(m_marker->get_composer());
	lineEditSongwriter->setText(m_marker->get_songwriter());
	lineEditArranger->setText(m_marker->get_arranger());
	lineEditMessage->setText(m_marker->get_message());
	lineEditIsrc->setText(m_marker->get_isrc());
	checkBoxPreEmph->setChecked(m_marker->get_preemphasis());
	checkBoxCopy->setChecked(m_marker->get_copyprotect());
}

// update the entry in the tree widget in real time
void MarkerDialog::description_changed(const QString &s)
{
	QTreeWidgetItem* item = markersTreeWidget->currentItem();

	if (!item) {
		return;
	}

	if (!m_marker) {
		return;
	}

	item->setText(1, s);
	m_marker->set_description(s);
}

// find the marker based on it's id. Since each song has it's own timeline,
// we need to iterate over all songs
Marker * MarkerDialog::get_marker(qint64 id)
{
	foreach(Song* song, m_project->get_songs()) {

		TimeLine* tl = song->get_timeline();
		
		foreach(Marker* marker, tl->get_markers()) {
			if (marker->get_id() == id) {
				return marker;
			}
		}
	}
	return 0;
}


//eof

