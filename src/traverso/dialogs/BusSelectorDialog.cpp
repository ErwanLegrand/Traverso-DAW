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

#include "BusSelectorDialog.h"

#include <AudioDevice.h>
#include <ProjectManager.h>
#include <Song.h>
#include <Track.h>
#include <Project.h>
#include <QPushButton>


BusSelectorDialog::BusSelectorDialog(QWidget* parent)
	: QDialog(parent)
	, m_currentTrack(0)
{
	setupUi(this);
	
	update_buses_list_widget();
	
	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(trackComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(current_track_changed(int)));
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(update_buses_list_widget()));
}

void BusSelectorDialog::update_buses_list_widget()
{
	busesListWidget->clear();
	
	QStringList names = audiodevice().get_capture_buses_names();
	
	foreach(QString name, names) {
		QListWidgetItem* item = new QListWidgetItem(busesListWidget);
		item->setText(name);
	}
	
	busesListWidgetPlayback->clear();
	names.clear();
	names << audiodevice().get_playback_buses_names();
	
	foreach(QString name, names) {
		QListWidgetItem* item = new QListWidgetItem(busesListWidgetPlayback);
		item->setText(name);
	}
}

void BusSelectorDialog::current_track_changed(int index)
{
	if (index == -1) {
		return;
	}
	
	QList<QListWidgetItem *>  selectedlist = busesListWidget->selectedItems();
	
	if (selectedlist.size()) {
		selectedlist.at(0)->setSelected(false);
	}
	
	qint64 id = trackComboBox->itemData(index).toLongLong();
	Song* song = pm().get_project()->get_current_song();
	m_currentTrack = song->get_track(id);
	
	QList<QListWidgetItem *> list = busesListWidget->findItems(m_currentTrack->get_bus_in(), Qt::MatchExactly);
	
	if (list.size()) {
		QListWidgetItem* item = list.at(0);
		item->setSelected(true);
		
		if (m_currentTrack->capture_left_channel() && m_currentTrack->capture_right_channel()) {
			radioBoth->setChecked(true);
		} else if (m_currentTrack->capture_left_channel()) {
			radioLeftOnly->setChecked(true);
		} else {
			radioRightOnly->setChecked(true);
		}	
	}


	selectedlist = busesListWidgetPlayback->selectedItems();
	
	if (selectedlist.size()) {
		selectedlist.at(0)->setSelected(false);
	}
	
	list = busesListWidgetPlayback->findItems(m_currentTrack->get_bus_out(), Qt::MatchExactly);
	
	if (list.size()) {
		QListWidgetItem* item = list.at(0);
		item->setSelected(true);
		if (m_currentTrack->playback_left_channel() && m_currentTrack->playback_right_channel()) {
			radioBothPlayback->setChecked(true);
		} else if (m_currentTrack->playback_left_channel()) {
			radioLeftOnlyPlayback->setChecked(true);
		} else {
			radioRightOnlyPlayback->setChecked(true);
		}	
	}
}



void BusSelectorDialog::accept()
{
	Q_ASSERT(m_currentTrack);
	
	QList<QListWidgetItem *>  list = busesListWidget->selectedItems();
	
	if (list.size()) {
		m_currentTrack->set_bus_in(list.at(0)->text().toAscii());
		
		if (radioBoth->isChecked()) {
			m_currentTrack->set_capture_left_channel(true);
			m_currentTrack->set_capture_right_channel(true);
		} else if (radioLeftOnly->isChecked()) {
			m_currentTrack->set_capture_left_channel(true);
			m_currentTrack->set_capture_right_channel(false);
		} else {
			m_currentTrack->set_capture_left_channel(false);
			m_currentTrack->set_capture_right_channel(true);
		}
	}

	list = busesListWidgetPlayback->selectedItems();
	
	if (list.size()) {
		m_currentTrack->set_bus_out(list.at(0)->text().toAscii());
		
		if (radioBothPlayback->isChecked()) {
			m_currentTrack->set_playback_left_channel(true);
			m_currentTrack->set_playback_right_channel(true);
		} else if (radioLeftOnlyPlayback->isChecked()) {
			m_currentTrack->set_playback_left_channel(true);
			m_currentTrack->set_playback_right_channel(false);
		} else {
			m_currentTrack->set_playback_left_channel(false);
			m_currentTrack->set_playback_right_channel(true);
		}
	}
	
	hide();
}

void BusSelectorDialog::reject()
{
	hide();
}

void BusSelectorDialog::set_current_track(Track * track)
{
	trackComboBox->clear();
	
	Song* song = pm().get_project()->get_current_song();
	
	foreach(Track* track, song->get_tracks()) {
		QString fulltitle = QString::number(track->get_sort_index() + 1) + " " + track->get_name();
		trackComboBox->addItem(fulltitle, track->get_id());
	}
	
	
	int index = trackComboBox->findData(track->get_id());
	
	trackComboBox->setCurrentIndex(index);
}

//eof

