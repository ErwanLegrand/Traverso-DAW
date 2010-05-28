/*
Copyright (C) 2010 Remon Sijrier

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


#include "TTrackManagerDialog.h"

#include "AudioDevice.h"
#include "Track.h"
#include "ProjectManager.h"
#include "Project.h"
#include "Sheet.h"
#include "SubGroup.h"

#include <QMenu>

TTrackManagerDialog::TTrackManagerDialog(Track *track, QWidget *parent)
        : QDialog(parent)
        , m_track(track)
{
        setupUi(this);

        m_routingInputMenu = m_routingOutputMenu = 0;
        create_routing_input_menu();
        create_routing_output_menu();

        update_routing_input_output_widget_view();

        insertsAndSendsGroupBox->hide();
        nameLineEdit->setText(m_track->get_name());

        resize(400, 350);

        if (m_track->get_type() == Track::SUBGROUP) {
                trackLabel->setText(tr("SubGroup Bus:"));
                routingInputButton->setText("Add Input");
                routingInputAddNewButton->hide();
                routingOutputAddNewButton->hide();
        }
        if (m_track->get_type() == Track::AUDIOTRACK) {
                trackLabel->setText(tr("Audio Track:"));
                routingInputButton->setText("Set Input");
                routingInputRemoveButton->hide();
                routingOutputRemoveButton->hide();
        }

        MasterOutSubGroup* master = qobject_cast<MasterOutSubGroup*>(m_track);
        if (master) {
                // Master Buses are not allowed to be renamed to avoid confusion
                nameLineEdit->setEnabled(false);
                trackLabel->setText(tr("Master Bus:"));
        }

        connect(m_track, SIGNAL(busConfigurationChanged()), this, SLOT(update_routing_input_output_widget_view()));
}

void TTrackManagerDialog::create_routing_input_menu()
{
        if (m_routingInputMenu) {
                delete m_routingInputMenu;
        }

        m_routingInputMenu = new QMenu;

        if (m_track->get_type() != Track::SUBGROUP) {
                foreach(QString busName, audiodevice().get_capture_buses_names()) {
                        m_routingInputMenu->addAction(busName);
                }
        }

        routingInputButton->setMenu(m_routingInputMenu);

        connect(m_routingInputMenu, SIGNAL(triggered(QAction*)), this, SLOT(routingInputMenuActionTriggered(QAction*)));

}

void TTrackManagerDialog::create_routing_output_menu()
{
        if (m_routingOutputMenu) {
                delete m_routingOutputMenu;
        }

        m_routingOutputMenu = new QMenu;

        Sheet* sheet = m_track->get_sheet();

        SubGroup* sheetMaster = 0;
        SubGroup* projectMaster = pm().get_project()->get_master_out();

        if (sheet) {
                sheetMaster = sheet->get_master_out();
        }

        if (sheetMaster && !(m_track == sheetMaster)) {
                m_routingOutputMenu->addAction(sheetMaster->get_name());
        }

        if (!(m_track == projectMaster)) {
                m_routingOutputMenu->addAction(projectMaster->get_name());
        }

        if (sheet) {
                QList<SubGroup*> subgroups = sheet->get_subgroups();
                if (!m_track->get_type() == Track::SUBGROUP && subgroups.size()) {
                        foreach(SubGroup* sub, subgroups) {
                                m_routingOutputMenu->addAction(sub->get_name());
                        }
                }
        }

        m_routingOutputMenu->addSeparator();

        foreach(QString busName, audiodevice().get_playback_buses_names()) {
                m_routingOutputMenu->addAction(busName);
        }

        routingOutputButton->setMenu(m_routingOutputMenu);

        connect(m_routingOutputMenu, SIGNAL(triggered(QAction*)), this, SLOT(routingOutputMenuActionTriggered(QAction*)));
}

void TTrackManagerDialog::accept()
{
        QDialog::accept();

        m_track->set_name(nameLineEdit->text());
}

void TTrackManagerDialog::reject()
{
        QDialog::reject();
}

void TTrackManagerDialog::routingInputMenuActionTriggered(QAction *action)
{
        if (action) {
                m_track->set_input_bus(action->text());
        }
}

void TTrackManagerDialog::routingOutputMenuActionTriggered(QAction *action)
{
        if (action) {
                m_track->set_output_bus(action->text());
        }
}


void TTrackManagerDialog::update_routing_input_output_widget_view()
{
        routingInputListWidget->clear();
        routingInputListWidget->addItem(m_track->get_bus_in_name());

        routingOutputListWidget->clear();
        routingOutputListWidget->addItem(m_track->get_bus_out_name());
}
