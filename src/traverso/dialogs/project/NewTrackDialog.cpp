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
#include "SubGroup.h"
#include "AudioTrack.h"

#include <CommandGroup.h>

NewTrackDialog::NewTrackDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
	
	set_project(pm().get_project());
	
        buttonBox->button(QDialogButtonBox::Apply)->setDefault(true);
        update_buses_comboboxes();
	
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
        connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(clicked(QAbstractButton*)));
        connect(isSubGroup, SIGNAL(toggled(bool)), this, SLOT(update_buses_comboboxes()));
}

void NewTrackDialog::showEvent(QShowEvent *event)
{
        update_driver_info();
}

void NewTrackDialog::create_track()
{
	if (! m_project) {
		info().information(tr("I can't create a new Track if there is no Project loaded!!"));
		return;
	}
	
	Sheet* sheet = m_project->get_current_sheet();
	if ( ! sheet ) {
		return ;
	}
	
        QString title = trackName->text();
	
	if (title.isEmpty()) {
		title = "Untitled";
	}
	
        Project* project = pm().get_project();
        Track* track;
        AudioBus* inputBus = 0;
        AudioBus* outputBus = 0;

        QString driver = audiodevice().get_driver_type();
        if (driver == "Jack") {
                for (int i=0; i<2; i++) {
                        // subgroups don't have input ports, so skip those.
                        if (isSubGroup->isChecked() && i == 1) {
                                continue;
                        }

                        QStringList channelnames;
                        BusConfig busconfig;

                        if (monoRadioButton->isChecked()) {
                                channelnames << title;
                        } else {
                                channelnames << title + "_0" << title + "_1";
                        }

                        foreach(const QString& channelname, channelnames) {
                                ChannelConfig channelconfig;
                                channelconfig.name = i == 0 ? channelname + "_out" : channelname + "_in";
                                channelconfig.type = i == 0 ? "output" : "input";
                                busconfig.channelNames << channelconfig.name;
                        }

                        busconfig.channelcount = channelnames.size();
                        busconfig.name = title;
                        busconfig.type =  i == 0 ? "output" : "input";

                        if (i==0) {
                                outputBus = project->create_software_audio_bus(busconfig);
                        }
                        if (i==1) {
                                inputBus = project->create_software_audio_bus(busconfig);
                        }
                }
        }

        if (isSubGroup->isChecked()) {
                if (driver == "Jack") {
                        track = new SubGroup(sheet, title + "-sub", 2);
                } else {
                        track = new SubGroup(sheet, title, 2);
                }

        } else {
                track = new AudioTrack(sheet, title, AudioTrack::INITIAL_HEIGHT);
        }

        if (driver == "Jack") {
                // only AudioTracks have external inputs
                if (track->get_type() == Track::AUDIOTRACK) {
                        track->add_input_bus(title);
                }
                track->add_post_send(outputBus->get_id());
        } else {
                // only AudioTracks have external inputs
                if (track->get_type() == Track::AUDIOTRACK) {
                        track->add_input_bus(inputBuses->currentText());
                }

                int index = outputBuses->currentIndex();
                qint64 outputBusId = outputBuses->itemData(index).toLongLong();
                track->add_post_send(outputBusId);
        }

        Command* command = sheet->add_track(track);
        command->setText(tr("Added %1: %2").arg(track->metaObject()->className()).arg(track->get_name()));

        Command::process_command(command);
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
        inputBuses->clear();
        outputBuses->clear();

        Sheet* sheet = m_project->get_current_sheet();

        if ( ! sheet ) {
                return ;
        }

        if (isSubGroup->isChecked()) {
                inputBusFrame->setEnabled(false);
        } else {
                inputBusFrame->setEnabled(true);
        }

        QList<SubGroup*> subs;
        subs.append(m_project->get_master_out());
        subs.append(sheet->get_master_out());
        subs.append(sheet->get_subgroups());
        foreach(SubGroup* sg, subs) {
                outputBuses->addItem(sg->get_name(), sg->get_id());
        }

        QList<AudioBus*> hardwareBuses = m_project->get_hardware_buses();

        foreach(AudioBus* bus, hardwareBuses) {
                if (bus->get_type() == ChannelIsInput) {
                        inputBuses->addItem(bus->get_name(), bus->get_id());
                } else {
                        outputBuses->addItem(bus->get_name(), bus->get_id());
                }
        }
}

void NewTrackDialog::update_driver_info()
{
        QString driver = audiodevice().get_driver_type();
        if (driver == "Jack") {
                jackTrackGroupBox->show();
                busConfigGroupBox->hide();
        } else {
                jackTrackGroupBox->hide();
                busConfigGroupBox->show();
        }
}
