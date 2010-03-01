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
	
	
        Track* track;

        if (isSubGroup->isChecked()) {
                track = new SubGroup(sheet, title, 2);
        } else {
                track = new AudioTrack(sheet, "Unnamed", AudioTrack::INITIAL_HEIGHT);
                track->set_input_bus(inputBuses->currentText());
        }

        track->set_name(title);
        track->set_output_bus(outputBuses->currentText());

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
        outputBuses->clear();

        Sheet* sheet = m_project->get_current_sheet();

        if ( ! sheet ) {
                return ;
        }

        QStringList busNames;

        if (isSubGroup->isChecked()) {
                inputBusFrame->setEnabled(false);
        } else {
                inputBusFrame->setEnabled(true);
        }

        busNames.append(sheet->get_master_out()->get_name());
        busNames.append(audiodevice().get_playback_buses_names());

        foreach(QString busName, busNames) {
                outputBuses->addItem(busName);
        }


        inputBuses->clear();

        busNames.clear();
        busNames.append(audiodevice().get_capture_buses_names());

        foreach(QString busName, busNames) {
                inputBuses->addItem(busName);
        }
}


