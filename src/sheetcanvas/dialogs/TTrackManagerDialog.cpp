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
#include "AudioTrack.h"
#include "Track.h"
#include "ProjectManager.h"
#include "Project.h"
#include "Sheet.h"
#include "TBusTrack.h"
#include "Utils.h"
#include "TSend.h"
#include "Themer.h"
#include "TMainWindow.h"

#include <QMenu>

#include "Debugger.h"

TTrackManagerDialog::TTrackManagerDialog(Track *track, QWidget *parent)
        : QDialog(parent)
        , m_track(track)
{
        setupUi(this);

        preSendsGainPanGroupBox->setEnabled(false);
        postSendsGainPanGroupBox->setEnabled(false);

        m_routingInputMenu = m_routingOutputMenu = m_preSendsMenu = 0;
        m_selectedPostSend = m_selectedPreSend = 0;
        create_routing_input_menu();
        create_routing_output_menu();
        create_pre_sends_menu();

        update_routing_input_output_widget_view();
        trackPanSlider->setValue(m_track->get_pan() * 64);
        trackGainSlider->setValue(coefficient_to_dB(m_track->get_gain()) * 10);

        update_gain_indicator();
        update_pan_indicator();
        update_track_status_buttons(true);

        nameLineEdit->setText(m_track->get_name());

        if (m_track->get_type() == Track::AUDIOTRACK) {
                trackLabel->setText(tr("Audio Track:"));
                routingInputButton->setText("Set Input");
        }
        if (m_track->get_type() == Track::BUS) {
                trackLabel->setText(tr("Bus Track:"));
                routingInputButton->setText("Add Input");
        }

        prePluginsUpButton->setIcon(QIcon(":/up"));
        prePluginsDownButton->setIcon(QIcon(":/down"));
        postPluginsUpButton->setIcon(QIcon(":/up"));
        postPluginsDownButton->setIcon(QIcon(":/down"));

        MasterOutSubGroup* master = qobject_cast<MasterOutSubGroup*>(m_track);
        if (master) {
                // Master Buses are not allowed to be renamed to avoid confusion
                nameLineEdit->setEnabled(false);
                trackLabel->setText(tr("Master Bus:"));
        }

        connect(m_track, SIGNAL(panChanged()), this, SLOT(update_pan_indicator()));
        connect(m_track, SIGNAL(stateChanged()), this, SLOT(update_gain_indicator()));
        connect(m_track, SIGNAL(soloChanged(bool)), this, SLOT(update_track_status_buttons(bool)));
        connect(m_track, SIGNAL(muteChanged(bool)), this, SLOT(update_track_status_buttons(bool)));
        AudioTrack* audiotrack = qobject_cast<AudioTrack*>(m_track);
        if (audiotrack) {
                connect(audiotrack, SIGNAL(armedChanged(bool)), this, SLOT(update_track_status_buttons(bool)));
        } else {
                recordButton->hide();
        }
        connect(pm().get_project(), SIGNAL(trackRoutingChanged()), this, SLOT(update_routing_input_output_widget_view()));

        connect(preSendsListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(pre_sends_selection_changed()));
        connect(routingOutputListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(post_sends_selection_changed()));
        connect(trackGainSlider, SIGNAL(valueChanged(int)), this, SLOT(track_gain_value_changed(int)));
        connect(trackPanSlider, SIGNAL(valueChanged(int)), this, SLOT(track_pan_value_changed(int)));
        connect(postSendsGainSlider, SIGNAL(valueChanged(int)), this, SLOT(post_sends_gain_value_changed(int)));
        connect(postSendsPanSlider, SIGNAL(valueChanged(int)), this, SLOT(post_sends_pan_value_changed(int)));
        connect(preSendsGainSlider, SIGNAL(valueChanged(int)), this, SLOT(pre_sends_gain_value_changed(int)));
        connect(preSendsPanSlider, SIGNAL(valueChanged(int)), this, SLOT(pre_sends_pan_value_changed(int)));
}

void TTrackManagerDialog::create_routing_input_menu()
{
        if (m_routingInputMenu) {
                delete m_routingInputMenu;
        }

        m_routingInputMenu = new QMenu;

        if (m_track->get_type() == Track::AUDIOTRACK) {
                foreach(AudioBus* bus, pm().get_project()->get_hardware_buses()) {
                        if (bus->is_input()) {
                                QAction* action = m_routingInputMenu->addAction(bus->get_name());
                                action->setData(bus->get_id());
                        }
                }
        }

        Project* project = pm().get_project();
        Sheet* sheet = qobject_cast<Sheet*>(m_track->get_session());
        bool isProjectBus = false;

        foreach(TBusTrack* track, project->get_bus_tracks()) {
                if (track == m_track) {
                        isProjectBus = true;
                        break;
                }
        }

        if (m_track->get_type() == Track::BUS) {
                QList<Track*> tracks;
                if (m_track == project->get_master_out()) {
                        tracks = project->get_tracks();
                        tracks.append(project->get_sheet_tracks());
                } else if (isProjectBus) {
                        foreach(Sheet* sheet, project->get_sheets()) {
                                foreach(AudioTrack* track, sheet->get_audio_tracks()) {
                                        tracks.append(track);
                                }
                                foreach(TBusTrack* track, sheet->get_bus_tracks()) {
                                        tracks.append(track);
                                }
                                tracks.append(sheet->get_master_out());

                        }
                } else if (sheet){
                        foreach(AudioTrack* at, sheet->get_audio_tracks()) {
                                tracks.append(at);
                        }
                        if (m_track == sheet->get_master_out()) {
                                foreach(TBusTrack* sg, sheet->get_bus_tracks()) {
                                        tracks.append(sg);
                                }
                        }
                }
                foreach(Track* track, tracks) {
                        QAction* action = m_routingInputMenu->addAction(track->get_name());
                        action->setData(track->get_id());
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

        m_routingOutputMenu = create_sends_menu();

        routingOutputButton->setMenu(m_routingOutputMenu);

        connect(m_routingOutputMenu, SIGNAL(triggered(QAction*)), this, SLOT(routingOutputMenuActionTriggered(QAction*)));
}

void TTrackManagerDialog::create_pre_sends_menu()
{
        if (m_preSendsMenu) {
                delete m_preSendsMenu;
        }

        m_preSendsMenu = create_sends_menu();

        preSendsButton->setMenu(m_preSendsMenu);

        connect(m_preSendsMenu, SIGNAL(triggered(QAction*)), this, SLOT(preSendsMenuActionTriggered(QAction*)));
}

QMenu* TTrackManagerDialog::create_sends_menu()
{
        QMenu* menu = new QMenu;
        QAction* action;

        Sheet* sheet = qobject_cast<Sheet*>(m_track->get_session());
        Project* project = pm().get_project();

        TBusTrack* sheetMaster = 0;
        TBusTrack* projectMaster = project->get_master_out();

        if (sheet) {
                sheetMaster = sheet->get_master_out();
        }

        if (!(m_track == projectMaster)) {
                action = menu->addAction(projectMaster->get_name());
                action->setData(projectMaster->get_id());
        }

        if (sheetMaster && !(m_track == sheetMaster)) {
                action = menu->addAction(sheetMaster->get_name());
                action->setData(sheetMaster->get_id());
        }

        if (sheet) {
                QList<TBusTrack*> busTracks = sheet->get_bus_tracks();
                if (!m_track->get_type() == Track::BUS && busTracks.size()) {
                        foreach(TBusTrack* busTrack, busTracks) {
                                action = menu->addAction(busTrack->get_name());
                                action->setData(busTrack->get_id());
                        }
                }
                menu->addSeparator();
        }

        if (project) {
                QList<TBusTrack*> busTracks = project->get_bus_tracks();
                foreach(TBusTrack* busTrack, busTracks) {
                        action = menu->addAction(busTrack->get_name());
                        action->setData(busTrack->get_id());
                }
                menu->addSeparator();
        }



        foreach(AudioBus* bus, pm().get_project()->get_hardware_buses()) {
                if (bus->is_output()) {
                        action = menu->addAction(bus->get_name());
                        action->setData(bus->get_id());
                }
        }

        return menu;
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
        PENTER2;

        Project* project = pm().get_project();
        if (!project) {
                return;
        }

        if (!action) {
                return;
        }

        if (m_track->get_type() == Track::AUDIOTRACK) {
                m_track->add_input_bus(action->text());
        }

        if (m_track->get_type() == Track::BUS) {
                qint64 senderId = action->data().toLongLong();
                Track* sender = project->get_track(senderId);
                if (sender) {
                        sender->add_post_send(m_track->get_id());
                }
        }
}

void TTrackManagerDialog::routingOutputMenuActionTriggered(QAction *action)
{
        Project* project = pm().get_project();
        if (action && project) {
                m_track->add_post_send(action->data().toLongLong());
        }
}

void TTrackManagerDialog::preSendsMenuActionTriggered(QAction *action)
{
        Project* project = pm().get_project();
        if (action && project) {
                m_track->add_pre_send(action->data().toLongLong());
        }
}


void TTrackManagerDialog::update_routing_input_output_widget_view()
{
        routingInputListWidget->clear();

        if (m_track->get_type() == Track::BUS) {
                QList<TSend*> inputs = pm().get_project()->get_inputs_for_bus_track(qobject_cast<TBusTrack*>(m_track));
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

        preSendsListWidget->clear();
        QList<TSend*> preSends = m_track->get_pre_sends();
        foreach(TSend* send, preSends) {
                QListWidgetItem* item = new QListWidgetItem(preSendsListWidget);
                item->setText(send->get_name());
                item->setData(Qt::UserRole, send->get_id());
                preSendsListWidget->addItem(item);
        }
}

void TTrackManagerDialog::on_routingInputRemoveButton_clicked()
{
        QList<QListWidgetItem*> selectedItems = routingInputListWidget->selectedItems();
        foreach(QListWidgetItem* item, selectedItems) {
                qint64 id = item->data(Qt::UserRole).toLongLong();
                QList<qint64> toBeRemoved;
                toBeRemoved.append(id);
                QList<Track*> tracks = pm().get_project()->get_sheet_tracks();
                tracks.append(pm().get_project()->get_tracks());
                foreach(Track* track, tracks) {
                        QList<TSend*> preSends = track->get_pre_sends();
                        foreach(TSend* send, preSends) {
                                if (send->get_id() == id) {
                                        track->remove_pre_sends(toBeRemoved);
                                }
                        }
                        QList<TSend*> postSends = track->get_post_sends();
                        foreach(TSend* send, postSends) {
                                if (send->get_id() == id) {
                                        track->remove_post_sends(toBeRemoved);
                                }
                        }
                }
        }
}

void TTrackManagerDialog::on_preSendsRemoveButton_clicked()
{
        QList<QListWidgetItem*> selectedItems = preSendsListWidget->selectedItems();
        QList<qint64> toBeRemoved;
        foreach(QListWidgetItem* item, selectedItems) {
                qint64 id = item->data(Qt::UserRole).toLongLong();
                toBeRemoved.append(id);
        }

        m_track->remove_pre_sends(toBeRemoved);
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

void TTrackManagerDialog::pre_sends_selection_changed()
{
        QList<QListWidgetItem*> selectedItems = preSendsListWidget->selectedItems();
        if (selectedItems.size()) {
                preSendsGainPanGroupBox->setEnabled(true);
                qint64 sendId = selectedItems.first()->data(Qt::UserRole).toLongLong();
                m_selectedPreSend = m_track->get_send(sendId);
                if (m_selectedPreSend) {
                        preSendsGainSlider->setValue(coefficient_to_dB(m_selectedPreSend->get_gain()) * 10);
                        postSendsPanSlider->setValue(m_selectedPreSend->get_pan() * 64);
                }
        } else {
                preSendsGainPanGroupBox->setEnabled(false);
                m_selectedPreSend = 0;
        }
}

void TTrackManagerDialog::post_sends_selection_changed()
{
        QList<QListWidgetItem*> selectedItems = routingOutputListWidget->selectedItems();
        if (selectedItems.size()) {
                postSendsGainPanGroupBox->setEnabled(true);
                qint64 sendId = selectedItems.first()->data(Qt::UserRole).toLongLong();
                m_selectedPostSend = m_track->get_send(sendId);
                if (m_selectedPostSend) {
                        postSendsGainSlider->setValue(coefficient_to_dB(m_selectedPostSend->get_gain()) * 10);
                        postSendsPanSlider->setValue(m_selectedPostSend->get_pan() * 64);
                }
        } else {
                postSendsGainPanGroupBox->setEnabled(false);
                m_selectedPreSend = 0;
        }
}

void TTrackManagerDialog::track_gain_value_changed(int value)
{
        float v = float(value) / 10;
        float gain = dB_to_scale_factor(v);
        m_track->set_gain(gain);
}

void TTrackManagerDialog::track_pan_value_changed(int value)
{
        float pan = float(value) / 64;
        m_track->set_pan(pan);
}

void TTrackManagerDialog::pre_sends_gain_value_changed(int value)
{
        if (!m_selectedPreSend) {
                return;
        }

        float v = float(value) / 10;
        float gain = dB_to_scale_factor(v);
        QByteArray gainString = QByteArray::number(v, 'f', 1) + " dB";
        preSendGainLabel->setText(gainString);
        m_selectedPreSend->set_gain(gain);
}

void TTrackManagerDialog::pre_sends_pan_value_changed(int value)
{
        if (!m_selectedPreSend) {
                return;
        }

        float pan = float(value) / 64;
        QByteArray panString = QByteArray::number(pan, 'f', 2);
        preSendPanLabel->setText(panString);
        m_selectedPreSend->set_pan(pan);
}

void TTrackManagerDialog::post_sends_gain_value_changed(int value)
{
        if (!m_selectedPostSend) {
                return;
        }
        float v = float(value) / 10;
        float gain = dB_to_scale_factor(v);
        QByteArray gainString = QByteArray::number(v, 'f', 1) + " dB";
        postSendGainLabel->setText(gainString);
        m_selectedPostSend->set_gain(gain);
}

void TTrackManagerDialog::post_sends_pan_value_changed(int value)
{
        if (!m_selectedPostSend) {
                return;
        }

        float pan = float(value) / 64;
        QByteArray panString = QByteArray::number(pan, 'f', 2);
        postSendPanLabel->setText(panString);
        m_selectedPostSend->set_pan(pan);
}

void TTrackManagerDialog::on_muteButton_clicked()
{
        m_track->mute();
}

void TTrackManagerDialog::on_soloButton_clicked()
{
        m_track->solo();
}

void TTrackManagerDialog::on_recordButton_clicked()
{
        AudioTrack* audiotrack = qobject_cast<AudioTrack*>(m_track);
        if (audiotrack) {
                audiotrack->toggle_arm();
        }
}

void TTrackManagerDialog::on_monitorButton_clicked()
{
//        m_track->monitor();
}

void TTrackManagerDialog::update_track_status_buttons(bool)
{
        QPalette defaultPalette = TMainWindow::instance()->palette();
        QPalette highlightedPalette = TMainWindow::instance()->palette();

        if (m_track->is_muted()) {
                highlightedPalette.setColor(QPalette::Button, themer()->get_color("TrackPanel:muteled"));
                muteButton->setPalette(highlightedPalette);
        } else {
                muteButton->setPalette(defaultPalette);
        }

        if (m_track->is_solo()) {
                highlightedPalette.setColor(QPalette::Button, themer()->get_color("TrackPanel:sololed"));
                soloButton->setPalette(highlightedPalette);
        } else {
                soloButton->setPalette(defaultPalette);
        }

        AudioTrack* audiotrack = qobject_cast<AudioTrack*>(m_track);
        if (audiotrack) {
                if (audiotrack->armed()) {
                        highlightedPalette.setColor(QPalette::Button, themer()->get_color("TrackPanel:recled"));
                        recordButton->setPalette(highlightedPalette);
                } else {
                        recordButton->setPalette(defaultPalette);
                }
        }
}
