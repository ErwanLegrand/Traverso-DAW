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

#include "AudioBus.h"
#include "Track.h"
#include "ProjectManager.h"
#include "Project.h"
#include "Sheet.h"
#include "SubGroup.h"
#include "Utils.h"
#include "TSend.h"

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
        update_gain_indicator();
        update_pan_indicator();

        nameLineEdit->setText(m_track->get_name());

        resize(400, 350);

        if (m_track->get_type() == Track::SUBGROUP) {
                trackLabel->setText(tr("SubGroup Bus:"));
                routingInputButton->setText("Add Input");
        }
        if (m_track->get_type() == Track::AUDIOTRACK) {
                trackLabel->setText(tr("Audio Track:"));
                routingInputButton->setText("Set Input");
//                routingInputRemoveButton->hide();
        }

        MasterOutSubGroup* master = qobject_cast<MasterOutSubGroup*>(m_track);
        if (master) {
                // Master Buses are not allowed to be renamed to avoid confusion
                nameLineEdit->setEnabled(false);
                trackLabel->setText(tr("Master Bus:"));
        }

        connect(m_track, SIGNAL(routingConfigurationChanged()), this, SLOT(update_routing_input_output_widget_view()));
        connect(m_track, SIGNAL(panChanged()), this, SLOT(update_pan_indicator()));
        connect(m_track, SIGNAL(stateChanged()), this, SLOT(update_gain_indicator()));
}

void TTrackManagerDialog::create_routing_input_menu()
{
        if (m_routingInputMenu) {
                delete m_routingInputMenu;
        }

        m_routingInputMenu = new QMenu;

        if (m_track->get_type() != Track::SUBGROUP) {
                foreach(QString busName, pm().get_project()->get_capture_buses_names()) {
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
        QAction* action;

        Sheet* sheet = m_track->get_sheet();

        SubGroup* sheetMaster = 0;
        SubGroup* projectMaster = pm().get_project()->get_master_out();

        if (sheet) {
                sheetMaster = sheet->get_master_out();
        }

        if (!(m_track == projectMaster)) {
                action = m_routingOutputMenu->addAction(projectMaster->get_name());
                action->setData(projectMaster->get_id());
        }

        if (sheetMaster && !(m_track == sheetMaster)) {
                action = m_routingOutputMenu->addAction(sheetMaster->get_name());
                action->setData(sheetMaster->get_id());
        }

        if (sheet) {
                QList<SubGroup*> subgroups = sheet->get_subgroups();
                if (!m_track->get_type() == Track::SUBGROUP && subgroups.size()) {
                        foreach(SubGroup* sub, subgroups) {
                                action = m_routingOutputMenu->addAction(sub->get_name());
                                action->setData(sub->get_id());
                        }
                }
        }

        m_routingOutputMenu->addSeparator();

        foreach(AudioBus* bus, pm().get_project()->get_playback_buses()) {
                if (bus->is_output()) {
                        action = m_routingOutputMenu->addAction(bus->get_name());
                        action->setData(bus->get_id());
                }
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
        Project* project = pm().get_project();
        if (action && project) {
                m_track->add_input_bus(action->data().toLongLong());
        }
}

void TTrackManagerDialog::routingOutputMenuActionTriggered(QAction *action)
{
        Project* project = pm().get_project();
        if (action && project) {
                m_track->add_post_send(action->data().toLongLong());
        }
}


void TTrackManagerDialog::update_routing_input_output_widget_view()
{
        routingInputListWidget->clear();

        if (m_track->get_type() == Track::SUBGROUP) {
                QList<TSend*> inputs = pm().get_project()->get_inputs_for_subgroup(qobject_cast<SubGroup*>(m_track));
                foreach(TSend* send, inputs) {
                        QListWidgetItem* item = new QListWidgetItem(routingInputListWidget);
                        item->setText(send->get_from_name());
                        item->setData(Qt::UserRole, send->get_id());
                        routingInputListWidget->addItem(item);
                }
        }

        if (m_track->get_type() == Track::AUDIOTRACK) {
                routingInputListWidget->addItem(m_track->get_bus_in_name());
        }


        routingOutputListWidget->clear();

        QList<TSend*> postSends = m_track->get_post_sends();
        foreach(TSend* send, postSends) {
                QListWidgetItem* item = new QListWidgetItem(routingOutputListWidget);
                item->setText(send->get_name());
                item->setData(Qt::UserRole, send->get_id());
                routingOutputListWidget->addItem(item);
        }
}

void TTrackManagerDialog::on_routingOutputRemoveButton_clicked()
{
        QList<QListWidgetItem*> selectedItems = routingOutputListWidget->selectedItems();
        QList<qint64> toBeRemoved;
        foreach(QListWidgetItem* item, selectedItems) {
                qint64 id = item->data(Qt::UserRole).toLongLong();
                toBeRemoved.append(id);
        }

        m_track->remove_post_sends(toBeRemoved);
}

void TTrackManagerDialog::update_gain_indicator()
{
        gainLabel->setText(coefficient_to_dbstring(m_track->get_gain()));

}

void TTrackManagerDialog::update_pan_indicator()
{
        panLabel->setText(QByteArray::number(m_track->get_pan(), 'f', 2));
}
