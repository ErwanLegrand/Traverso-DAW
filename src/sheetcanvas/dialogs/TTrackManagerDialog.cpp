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

#include "Track.h"
#include "SubGroup.h"

TTrackManagerDialog::TTrackManagerDialog(Track *track, QWidget *parent)
        : QDialog(parent)
        , m_track(track)
{
        setupUi(this);

        insertsAndSendsGroupBox->hide();
        nameLineEdit->setText(m_track->get_name());

        resize(400, 350);

        if (m_track->get_type() == Track::SUBGROUP) {
                routingInputAddButton->setText("Add Input");
                routingOutputAddButton->setText("Add Output");
                routingInputAddNewButton->hide();
                routingOutputAddNewButton->hide();
        }
        if (m_track->get_type() == Track::AUDIOTRACK) {
                routingInputAddButton->setText("Set Input");
                routingOutputAddButton->setText("Set Output");
                routingInputRemoveButton->hide();
                routingOutputRemoveButton->hide();
        }

        MasterOutSubGroup* master = qobject_cast<MasterOutSubGroup*>(m_track);
        if (master) {
                // Master Buses are not allowed to be renamed to avoid confusion
                nameLineEdit->setEnabled(false);
        }
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
