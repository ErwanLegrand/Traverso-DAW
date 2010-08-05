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

#ifndef TTRACK_MANAGER_DIALOG_H
#define TTRACK_MANAGER_DIALOG_H

#include "ui_TTrackManagerDialog.h"

#include <QDialog>

class Track;
class TSend;
class QMenu;

class TTrackManagerDialog : public QDialog, protected Ui::TTrackManagerDialog
{
        Q_OBJECT

public:
        TTrackManagerDialog(Track* track, QWidget* parent);
        ~TTrackManagerDialog() {}

private:
        Track*  m_track;
        QMenu*  m_routingInputMenu;
        QMenu*  m_routingOutputMenu;
        QMenu*  m_preSendsMenu;
        TSend*  m_selectedPreSend;
        TSend*  m_selectedPostSend;

        void create_routing_input_menu();
        void create_routing_output_menu();
        void create_pre_sends_menu();
        QMenu* create_sends_menu();


private slots:
        void update_routing_input_output_widget_view();
        void update_gain_indicator();
        void update_pan_indicator();
        void accept();
        void reject();

        void routingInputMenuActionTriggered(QAction* action);
        void routingOutputMenuActionTriggered(QAction* action);
        void preSendsMenuActionTriggered(QAction* action);

        void on_routingOutputRemoveButton_clicked();
        void on_preSendsRemoveButton_clicked();
        void on_routingInputRemoveButton_clicked();

        void on_muteButton_clicked();
        void on_soloButton_clicked();
        void on_recordButton_clicked();
        void on_monitorButton_clicked();

        void pre_sends_selection_changed();
        void post_sends_selection_changed();
        void track_gain_value_changed(int value);
        void track_pan_value_changed(int value);
        void pre_sends_gain_value_changed(int value);
        void pre_sends_pan_value_changed(int value);
        void post_sends_gain_value_changed(int value);
        void post_sends_pan_value_changed(int value);

        void update_track_status_buttons(bool);

};

#endif
