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
#include <Sheet.h>
#include <Track.h>
#include <AudioTrack.h>
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
        captureBusesListWidget->clear();
        playbackBusesListWidget->clear();

        QStringList c_names = audiodevice().get_capture_buses_names();
        QStringList p_names = audiodevice().get_playback_buses_names();
        p_names.prepend("Master Out");

        foreach(QString name, c_names) {
                QListWidgetItem* item = new QListWidgetItem(captureBusesListWidget);
		item->setText(name);
	}
	
        foreach(QString name, p_names) {
                QListWidgetItem* item = new QListWidgetItem(playbackBusesListWidget);
		item->setText(name);
	}
}

void BusSelectorDialog::current_track_changed(int index)
{
	if (index == -1) {
		return;
	}

        QList<QListWidgetItem *>  c_selectedlist = captureBusesListWidget->selectedItems();
        QList<QListWidgetItem *>  p_selectedlist = playbackBusesListWidget->selectedItems();

        // first unselect selected items
        for (int i = 0; i < c_selectedlist.size(); ++i) {
                c_selectedlist.at(i)->setSelected(false);
	}

        for (int i = 0; i < p_selectedlist.size(); ++i) {
                p_selectedlist.at(i)->setSelected(false);
        }

	qint64 id = trackComboBox->itemData(index).toLongLong();
	Sheet* sheet = pm().get_project()->get_current_sheet();
	m_currentTrack = sheet->get_track(id);
	
        QList<QListWidgetItem *> c_list = captureBusesListWidget->findItems(m_currentTrack->get_bus_in_name(), Qt::MatchExactly);
        QList<QListWidgetItem *> p_list = playbackBusesListWidget->findItems(m_currentTrack->get_bus_out_name(), Qt::MatchExactly);

        if (c_list.size()) {
                QListWidgetItem* item = c_list.at(0);
		item->setSelected(true);
	}

        if (p_list.size()) {
                QListWidgetItem* item = p_list.at(0);
                item->setSelected(true);
        }
}



void BusSelectorDialog::accept()
{
	Q_ASSERT(m_currentTrack);
	
        QList<QListWidgetItem *>  c_list = captureBusesListWidget->selectedItems();
        QList<QListWidgetItem *>  p_list = playbackBusesListWidget->selectedItems();

        if (c_list.size()) {
                m_currentTrack->set_input_bus(c_list.at(0)->text());
        }

        if (p_list.size()) {
                m_currentTrack->set_output_bus(p_list.at(0)->text());
	}
	
	hide();
}

void BusSelectorDialog::reject()
{
	hide();
}

void BusSelectorDialog::set_current_track(Track * t)
{
	trackComboBox->clear();
	
	Sheet* sheet = pm().get_project()->get_current_sheet();
	
        foreach(Track* track, sheet->get_tracks()) {
		QString fulltitle = QString::number(track->get_sort_index() + 1) + " " + track->get_name();
		trackComboBox->addItem(fulltitle, track->get_id());
	}
	
	
        int index = trackComboBox->findData(t->get_id());
	
	trackComboBox->setCurrentIndex(index);
}


//eof

