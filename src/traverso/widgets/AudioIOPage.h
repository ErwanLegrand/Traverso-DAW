/*
    Copyright (C) 2010 Nicola Döbelin

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

#ifndef AUDIO_IO_PAGE_H
#define AUDIO_IO_PAGE_H

#include "ui_AudioIOPage.h"

#include <QWidget>
#include "defines.h"

class AudioIOPage : public QWidget, protected Ui::AudioIOPageWidget
{
    Q_OBJECT

    public:
        AudioIOPage(QWidget *parent = 0, Qt::WindowFlags f = 0);
        ~AudioIOPage() {};

        void init(const QString &, const QStringList &);
        QList<bus_config> getBusConfig();
        QStringList getChannelConfig();

    private:
        QString m_type;
        QStringList m_channels;
        bool m_bus_collapsed;
        bool m_chan_collapsed;

    private slots:
        void addMonoBus();
        void addStereoBus();
        void removeBus();
        void addPort();
        void removePort();

        void itemChanged(QTreeWidgetItem *, int);
        void itemDoubleClicked(QTreeWidgetItem *, int);
        void selectionChanged(QTreeWidgetItem *, QTreeWidgetItem *);
        void headerClicked(int);
        void headerDoubleClicked(int);
};

#endif

//eof
