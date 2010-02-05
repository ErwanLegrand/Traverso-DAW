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

#include "AudioIOPage.h"

#include <AudioDevice.h>
#include <AudioBus.h>
#include <AudioChannel.h>

#include <QInputDialog>

AudioIOPage::AudioIOPage(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    setupUi(this);

    m_bus_collapsed = false;
    m_chan_collapsed = false;

    // define these strings globally. It's safer when comparing
    // translated strings.
    m_str_left = tr("Left");
    m_str_center = tr("Center");
    m_str_right = tr("Right");

    treeWidget->header()->setClickable(true);

    connect(treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(selectionChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this, SLOT(itemChanged(QTreeWidgetItem *, int)));
    connect(treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this, SLOT(itemDoubleClicked(QTreeWidgetItem *, int)));
//    connect(treeWidget->header(), SIGNAL(sectionClicked(int)),
//            this, SLOT(headerClicked(int)));
    connect(treeWidget->header(), SIGNAL(sectionDoubleClicked(int)),
            this, SLOT(headerDoubleClicked(int)));
}

void AudioIOPage::init(const QString &t, const QStringList &c_names)
{
    m_type = t;
    m_channels = c_names;

    if (audiodevice().get_driver_type() == "Jack") {
        groupBoxJack->setVisible(true);
    } else {
        groupBoxJack->setVisible(false);
    }

    QStringList headers = c_names;
    headers.prepend(tr("Bus"));

    // add all hardware channels (columns)
    treeWidget->setHeaderLabels(headers);

    const QList<BusConfig> busList = audiodevice().get_bus_configuration();

        // loop over all audio busses
        for (int j = 0; j < busList.count(); ++j) {
                BusConfig conf = busList.at(j);
                if (! (conf.type == m_type)) continue;

                QTreeWidgetItem *itm = new QTreeWidgetItem();
                itm->setText(0, conf.name);
                treeWidget->addTopLevelItem(itm);

                for (int k = 0; k < treeWidget->columnCount(); ++k) {
                    itm->setBackground(k, QApplication::palette().alternateBase());
                }

                // Add all channels to the current bus. Name them either "Mono" or "Left/Right",
                // depending on the number of channels.
                QString lbl = m_str_center;
                if (conf.channelcount > 1) {
                        lbl = m_str_left;
                }

                for (int i = 0; i < conf.channelcount; ++i) {
                        QTreeWidgetItem *bitm = new QTreeWidgetItem(itm);
                        bitm->setText(0, lbl);
                        lbl = m_str_right;

                        // set all columns to unchecked first
                        for (int k = 1; k < treeWidget->columnCount(); ++k) {
                                bitm->setCheckState(k, Qt::Unchecked);
                        }

                        // now find the one to be checked by searching its name in the
                        // list of input channels. Add +1 because column no 0 contains
                        // the names, not check boxes
                        int idx = 0;
                        if (conf.channelNames.size() > i) {
                                idx = m_channels.indexOf(conf.channelNames.at(i), 0) + 1;
                        }

                        // if a valid index was found, check it (not found returns -1, which
                        // is 0 after adding +1)
                        if (idx) {
                                bitm->setCheckState(idx, Qt::Checked);
                        }
                }

                itm->setExpanded(true);
        }
}

QList<ChannelConfig> AudioIOPage::getChannelConfig()
{
        QList<ChannelConfig> configs;
        QTreeWidgetItem *chitem = treeWidget->headerItem();

        for (int i = 1; i < treeWidget->columnCount(); ++i) {
                ChannelConfig conf;
                conf.name = chitem->text(i);
                conf.type = m_type;
                configs.append(conf);
        }

        return configs;
}


// this function uses QMaps internally instead of QLists, because
// it's easier to keep the order of the items under control with QMaps.
QList<BusConfig> AudioIOPage::getBusConfig()
{
    QMap<int, BusConfig> list;
    QTreeWidgetItem *header = treeWidget->headerItem();

    int n = 0;
    while (treeWidget->topLevelItemCount()) {
        BusConfig conf;
        conf.channelcount = 0;

        QTreeWidgetItem *parent = treeWidget->takeTopLevelItem(0);
            conf.name = parent->text(0);
            conf.type = m_type;
            conf.channelcount = parent->childCount();

            QMap<int, QString> channels;
            int k = 0;
            while (parent->childCount()) {
                QTreeWidgetItem *child = parent->takeChild(0);

                if (child->text(0) == m_str_left) {
                    k = 0;
                }
                if (child->text(0) == m_str_center) {
                    k = 1;
                }
                if (child->text(0) == m_str_right) {
                    k = 2;
                }

                for (int i = 1; i < child->columnCount(); ++i) {
                   if (child->checkState(i) == Qt::Checked) {
                        channels.insert(k, header->text(i));
                     }
                }

                conf.channelNames = channels.values();
                delete child;
        }

        delete parent;
        list.insert(n, conf);
        ++n;
    }

    return list.values();
}

void AudioIOPage::addMonoBus()
{
    QString prefix = tr("Capture");

    if (m_type == "output") {
        prefix = tr("Playback");
    }

    QString name = QString(tr("%1 %2")).arg(prefix).arg(treeWidget->topLevelItemCount() + 1);

    QTreeWidgetItem *itm = new QTreeWidgetItem();
    itm->setText(0, name);
    treeWidget->addTopLevelItem(itm);

    QString lbm = tr("Mono");

    QTreeWidgetItem *bitm = new QTreeWidgetItem(itm);
    bitm->setText(0, lbm);

    for (int j = 1; j < treeWidget->columnCount(); ++j) {
        bitm->setCheckState(j, Qt::Unchecked);
    }

    itm->setExpanded(true);
}

void AudioIOPage::addStereoBus()
{
    QString prefix = tr("Capture");

    if (m_type == "output") {
        prefix = tr("Playback");
    }

    QString name = QString("%1 %2").arg(prefix).arg(treeWidget->topLevelItemCount() + 1);

    QTreeWidgetItem *itm = new QTreeWidgetItem();
    itm->setText(0, name);
    treeWidget->addTopLevelItem(itm);

    QString lbl = tr("Left");
    QString lbr = tr("Right");

    QTreeWidgetItem *bitl = new QTreeWidgetItem(itm);
    QTreeWidgetItem *bitr = new QTreeWidgetItem(itm);
    bitl->setText(0, lbl);
    bitr->setText(0, lbr);

    for (int j = 1; j < treeWidget->columnCount(); ++j) {
        bitl->setCheckState(j, Qt::Unchecked);
        bitr->setCheckState(j, Qt::Unchecked);
    }

    itm->setExpanded(true);
}

void AudioIOPage::removeBus()
{
    int idx = treeWidget->indexOfTopLevelItem(treeWidget->currentItem());

    if (idx == -1) {
        return;
    }

    QTreeWidgetItem *itm = treeWidget->takeTopLevelItem(idx);
    delete itm;
}

void AudioIOPage::addPort()
{
    int cols = treeWidget->columnCount();
    treeWidget->setColumnCount(cols + 1);
    QTreeWidgetItem *hitem = treeWidget->headerItem();
    hitem->setText(cols, QString("%1_%2").arg(m_type).arg(cols));

    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *titem = treeWidget->topLevelItem(i);
        titem->setBackground(i, QApplication::palette().alternateBase());

        for (int j = 0; j < titem->childCount(); ++j) {
            QTreeWidgetItem *citem = titem->child(j);
            citem->setCheckState(cols, Qt::Unchecked);
        }
    }
}

void AudioIOPage::removePort()
{
    treeWidget->setColumnCount(treeWidget->columnCount() - 1);
}

// we will uncheck all checkboxes but the one that was changed. We don't care if the
// changed one was checked or unchecked, as having none checked is valid, but having
// more than one checked is not.
void AudioIOPage::itemChanged(QTreeWidgetItem *itm, int col)
{
    // this is necessary because unchecking also emits the signal, so do
    // nothing if the state changed from checked to unchecked
    if (itm->checkState(col) == Qt::Unchecked) {
        return;
    }

    // now uncheck all boxes but the one checked by the user
    for (int i = 1; i < itm->columnCount(); ++i) {
        if (i != col) {
            itm->setCheckState(i, Qt::Unchecked);
        }
    }
}

// double click on a toplevel item opens a rename dialog
void AudioIOPage::itemDoubleClicked(QTreeWidgetItem *itm, int)
{
    // if it has a parent, it's not a toplevel item, so do nothing
    if (itm->parent()) {
        return;
    }

    // first find out in which TreeWidget the item lives
    QTreeWidget *tw = itm->treeWidget();
    QStringList names;

    // then load all toplevel item labels but the current one into the names stringlist
    for (int i = 0; i < tw->topLevelItemCount(); ++i) {
        if (tw->topLevelItem(i) != itm) {
            names.append(tw->topLevelItem(i)->text(0));
        }
    }

    QString oldstr = itm->text(0);
    QString newstr = itm->text(0);

    bool ok = false;

    // open a dialog and store the new string
    newstr = QInputDialog::getText(this, tr("Change Bus Name"),
                                           tr("Bus Name"), QLineEdit::Normal, oldstr, &ok);


    if (!ok) {  // dialog aborted
        return;
    }

    // if the new string already exists, open the dialog again until either a non-existing
    // name was entered, or the dialog is aborted
    while (names.contains(newstr, Qt::CaseSensitive)) {
        newstr = QInputDialog::getText(this, tr("Change Bus Name"),
                                               tr("Bus Name"), QLineEdit::Normal, newstr, &ok);

        if (!ok) {  // dialog aborted
            return;
        }
    }

    // if we made it down hear, we can accept the new string
    itm->setText(0, newstr);
}

void AudioIOPage::selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    if (current) {
        buttonRemoveBus->setEnabled(true);
    } else {
        buttonRemoveBus->setEnabled(false);
    }
}

void AudioIOPage::headerClicked(int col)
{
    if (!col) {
        return;
    }

    QHeaderView *header = treeWidget->header();
    if (header->sectionSize(col) == header->minimumSectionSize()) {
        header->resizeSection(col, header->defaultSectionSize());
    } else {
        header->resizeSection(col, header->minimumSectionSize());
    }
}

void AudioIOPage::headerDoubleClicked(int col)
{
    // double click on first column expands/collapses all busses
    if (!col) {
        for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
            treeWidget->topLevelItem(i)->setExpanded(m_bus_collapsed);
        }
        m_bus_collapsed = !m_bus_collapsed;
        return;
    }

    // double click on any other column expands/collapses all channels
    QHeaderView *header = treeWidget->header();
    int size;

    if (m_chan_collapsed) {
        size = header->defaultSectionSize();
    } else {
        size = header->minimumSectionSize();
    }

    m_chan_collapsed = !m_chan_collapsed;

    for (int i = 1; i < header->count(); ++i) {
        header->resizeSection(i, size);
    }
}

//eof
