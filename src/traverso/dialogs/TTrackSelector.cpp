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

#include "TTrackSelector.h"

#include "TSession.h"
#include "Track.h"

TTrackSelector::TTrackSelector(QWidget* parent, TSession* parentSession, TSession* childSession)
        : QDialog(parent)
        , m_parentSession(parentSession)
        , m_childSession(childSession)
{
        setupUi(this);

        QList<Track*> tracks = parentSession->get_tracks();
        foreach(Track* track, tracks) {
                QListWidgetItem* item = new QListWidgetItem(tracksListWidget);
                item->setText(track->get_name());
                item->setData(Qt::UserRole, track->get_id());
                tracksListWidget->addItem(item);
        }

        if (childSession->get_name().isEmpty()) {
                nameLineEdit->setText(QString("SubView %1").arg(parentSession->get_child_sessions().size() + 1));
                nameLineEdit->selectAll();
        } else {
                nameLineEdit->setText(childSession->get_name());
                tracksListWidget->setFocus(Qt::MouseFocusReason);
        }

        filterLineEdit->hide();
        filterNameLabel->hide();
}

void TTrackSelector::accept()
{
        QList<QListWidgetItem*> selected = tracksListWidget->selectedItems();
        foreach(QListWidgetItem* item, selected) {
                Track* track = m_parentSession->get_track(item->data(Qt::UserRole).toLongLong());
                Q_ASSERT(track);
                m_childSession->add_track(track);
        }

        m_childSession->set_name(nameLineEdit->text());

        done(QDialog::Accepted);
}

void TTrackSelector::reject()
{
        done(QDialog::Rejected);
}
