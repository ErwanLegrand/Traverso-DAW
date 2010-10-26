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


#include "NewTrackDialog.h"
#include <QPushButton>
#include <QRadioButton>
#include <QComboBox>

#include "AudioDevice.h"
#include "AudioBus.h"
#include "Information.h"
#include "Project.h"
#include "ProjectManager.h"
#include "Sheet.h"
#include "TBusTrack.h"
#include "AudioTrack.h"

#include <CommandGroup.h>

NewTrackDialog::NewTrackDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);

        resize(300, 300);

	set_project(pm().get_project());
	
        buttonBox->button(QDialogButtonBox::Apply)->setDefault(true);

        update_buses_comboboxes();
        reset_information_label();
        m_timer.setSingleShot(true);
	
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
        connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(clicked(QAbstractButton*)));
        connect(isBusTrack, SIGNAL(toggled(bool)), this, SLOT(update_buses_comboboxes()));
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(reset_information_label()));

        resize(300, 300);
}

void NewTrackDialog::showEvent(QShowEvent *event)
{
        update_driver_info();
        if (m_project->get_current_session() == m_project) {
                isAudioTrack->hide();
                isBusTrack->setChecked(true);
        } else {
                isAudioTrack->show();
                isAudioTrack->setChecked(true);
        }
}

void NewTrackDialog::create_track()
{
	if (! m_project) {
		info().information(tr("I can't create a new Track if there is no Project loaded!!"));
		return;
	}
	
        TSession* session = m_project->get_current_session();
        if ( ! session ) {
		return ;
	}
	
        QString title = trackName->text();
	
	if (title.isEmpty()) {
		title = "Untitled";
	}
	
        QString driver = audiodevice().get_driver_type();
        Sheet* sheet = qobject_cast<Sheet*>(session);
        Track* track;

        if (isBusTrack->isChecked()) {
                track = new TBusTrack(session, title, 2);

        } else {
                track = new AudioTrack(sheet, title, AudioTrack::INITIAL_HEIGHT);
        }

        track->set_channel_count(channelCountSpinBox->value());

        if (driver == "Jack") {
                track->connect_to_jack(true, true);
        } else {
                // only AudioTracks have external inputs
                if (track->get_type() == Track::AUDIOTRACK) {
                        track->add_input_bus(inputsHW->currentText());
                }

                int index = outputsHW->currentIndex();
                qint64 outputBusId = outputsHW->itemData(index).toLongLong();
                if (outputBusId) {
                        track->add_post_send(outputBusId);
                }
                outputBusId = outputsBus->itemData(index).toLongLong();
                if (outputBusId) {
                        track->add_post_send(outputBusId);
                }
        }

        TCommand* command = session->add_track(track);
        command->setText(tr("Added %1: %2").arg(track->metaObject()->className()).arg(track->get_name()));

        TCommand::process_command(command);

        QString styleSheet = "color: darkGreen; background: lightGreen; padding: 5px; border: solid 1px;";
        informationLabel->setStyleSheet(styleSheet);
        informationLabel->setText(tr("Created new Track '%1'' in Sheet '%2'").arg(track->get_name(), session->get_name()));

        m_timer.start(2000);
}

void NewTrackDialog::clicked(QAbstractButton *button)
{
        QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);

        if (role == QDialogButtonBox::RejectRole) {
                hide();
        }

        if (role == QDialogButtonBox::ApplyRole) {
                create_track();
        }

}

void NewTrackDialog::set_project(Project * project)
{
	m_project = project;
}

void NewTrackDialog::update_buses_comboboxes()
{
        outputsBus->clear();
        inputsHW->clear();
        outputsHW->clear();

        outputsHW->addItem("Select...");

        TSession* session = m_project->get_current_session();

        if ( ! session ) {
                return ;
        }

        if (isBusTrack->isChecked()) {
                inputBusFrame->setEnabled(false);
                jackInPortsCheckBox->setChecked(false);
        } else {
                inputBusFrame->setEnabled(true);
                jackInPortsCheckBox->setChecked(true);
        }

        QList<TBusTrack*> subs;
        subs.append(m_project->get_master_out());
        subs.append(session->get_master_out());
        subs.append(session->get_bus_tracks());
        foreach(TBusTrack* sg, subs) {
                outputsBus->addItem(sg->get_name(), sg->get_id());
        }

        QList<AudioBus*> hardwareBuses = m_project->get_hardware_buses();

        foreach(AudioBus* bus, hardwareBuses) {
                if (bus->get_type() == ChannelIsInput) {
                        inputsHW->addItem(bus->get_name(), bus->get_id());
                } else {
                        outputsHW->addItem(bus->get_name(), bus->get_id());
                }
        }
}

void NewTrackDialog::update_driver_info()
{
        QString driver = audiodevice().get_driver_type();
        if (driver == "Jack") {
                jackTrackFrame->show();
                busesFrame->hide();
        } else {
                jackTrackFrame->hide();
                busesFrame->show();
        }
}

void NewTrackDialog::reset_information_label()
{
        QString styleSheet = "color: black;";
        informationLabel->setStyleSheet(styleSheet);
        informationLabel->setText(tr("Fill in Track name, and hit enter to add new Track"));
}
