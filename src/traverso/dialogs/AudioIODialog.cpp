/*
    Copyright (C) 2009 Nicola Döbelin
 
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

#include "AudioIODialog.h"

#include <AudioDevice.h>
#include <AudioBus.h>
#include <AudioChannel.h>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QHash>
#include <QString>
#include <QPushButton>
#include <QStringList>
#include <QDebug>

AudioIODialog::AudioIODialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

	QString str = audiodevice().get_device_longname();
	labelDeviceName->setText(str);

        if (audiodevice().get_driver_type() == "Jack") {
                groupBoxJackInput->setVisible(true);
                groupBoxJackOutput->setVisible(true);
        } else {
                groupBoxJackInput->setVisible(false);
                groupBoxJackOutput->setVisible(false);
        }

	initInput();
	initOutput();

        connect(inputTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
                this, SLOT(inputSelectionChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
        connect(outputTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
                this, SLOT(outputSelectionChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
        connect(inputTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
                this, SLOT(itemChanged(QTreeWidgetItem *, int)));
        connect(outputTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
                this, SLOT(itemChanged(QTreeWidgetItem *, int)));
        connect(inputTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
                this, SLOT(itemDoubleClicked(QTreeWidgetItem *, int)));
        connect(outputTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
                this, SLOT(itemDoubleClicked(QTreeWidgetItem *, int)));
}

void AudioIODialog::initInput()
{
	m_inputChannelList = audiodevice().get_capture_channel_names();
        QStringList headers = m_inputChannelList;
        headers.prepend(tr("Bus"));

        // add all hardware channels (columns)
        inputTreeWidget->setHeaderLabels(headers);

        const QList<bus_config> busList = audiodevice().get_capture_bus_configuration();

        // loop over all audio busses
        for (int j = 0; j < busList.count(); ++j) {
                bus_config conf = busList.at(j);

                QTreeWidgetItem *itm = new QTreeWidgetItem();
                itm->setText(0, conf.name);
                inputTreeWidget->addTopLevelItem(itm);

                // Add all channels to the current bus. Name them either "Mono" or "Left/Right",
                // depending on the number of channels.
                QString lbl = tr("Mono");
                if (conf.channels.count() > 1) {
                        lbl = tr("Left");
                }

                for (int i = 0; i < conf.channels.count(); ++i) {
                        QTreeWidgetItem *bitm = new QTreeWidgetItem(itm);
                        bitm->setText(0, lbl);
                        lbl = tr("Right");

                        // set all columns to unchecked first
                        for (int k = 1; k < inputTreeWidget->columnCount(); ++k) {
                                bitm->setCheckState(k, Qt::Unchecked);
                        }

                        // now find the one to be checked by searching its name in the
                        // list of input channels. Add +1 because column no 0 contains
                        // the names, not check boxes
                        int idx = m_inputChannelList.indexOf(conf.channels.at(i), 0) + 1;

                        // if a valid index was found, check it (not found returns -1, which
                        // is 0 after adding +1)
                        if (idx) {
                                bitm->setCheckState(idx, Qt::Checked);
                        }
                }

                itm->setExpanded(true);
	}
}

void AudioIODialog::initOutput()
{
        m_outputChannelList = audiodevice().get_playback_channel_names();
        QStringList headers = m_outputChannelList;
        headers.prepend(tr("Bus"));

        // add all hardware channels (columns)
        outputTreeWidget->setHeaderLabels(headers);

        const QList<bus_config> busList = audiodevice().get_playback_bus_configuration();

        // loop over all audio busses
        for (int j = 0; j < busList.count(); ++j) {
                bus_config conf = busList.at(j);

                QTreeWidgetItem *itm = new QTreeWidgetItem();
                itm->setText(0, conf.name);
                outputTreeWidget->addTopLevelItem(itm);

                // Add all channels to the current bus. Name them either "Mono" or "Left/Right",
                // depending on the number of channels.
                QString lbl = tr("Mono");
                if (conf.channels.count() > 1) {
                        lbl = tr("Left");
                }

                for (int i = 0; i < conf.channels.count(); ++i) {
                        QTreeWidgetItem *bitm = new QTreeWidgetItem(itm);
                        bitm->setText(0, lbl);
                        lbl = tr("Right");

                        // set all columns to unchecked first
                        for (int k = 1; k < outputTreeWidget->columnCount(); ++k) {
                                bitm->setCheckState(k, Qt::Unchecked);
                        }

                        // now find the one to be checked by searching its name in the
                        // list of output channels. Add +1 because column no 0 contains
                        // the names, not check boxes
                        int idx = m_outputChannelList.indexOf(conf.channels.at(i), 0) + 1;

                        // if a valid index was found, check it (not found returns -1, which
                        // is 0 after adding +1)
                        if (idx) {
                                bitm->setCheckState(idx, Qt::Checked);
                        }
                }

                itm->setExpanded(true);
        }
}

void AudioIODialog::accept()
{
        QList<bus_config> iconf = inputBusConfig();
        QList<bus_config> oconf = outputBusConfig();

        audiodevice().set_bus_config(iconf, oconf);
	
	close();
}


void AudioIODialog::addMonoInput()
{
        QString name = QString(tr("Capture %1")).arg(inputTreeWidget->topLevelItemCount() + 1);

        QTreeWidgetItem *itm = new QTreeWidgetItem();
        itm->setText(0, name);
        inputTreeWidget->addTopLevelItem(itm);

        QString lbm = tr("Mono");

        QTreeWidgetItem *bitm = new QTreeWidgetItem(itm);
        bitm->setText(0, lbm);

        for (int j = 1; j < inputTreeWidget->columnCount(); ++j) {
                bitm->setCheckState(j, Qt::Unchecked);
        }

        itm->setExpanded(true);
}


void AudioIODialog::addMonoOutput()
{
        QString name = QString(tr("Playback %1")).arg(inputTreeWidget->topLevelItemCount() + 1);

        QTreeWidgetItem *itm = new QTreeWidgetItem();
        itm->setText(0, name);
        outputTreeWidget->addTopLevelItem(itm);

        QString lbm = tr("Mono");

        QTreeWidgetItem *bitm = new QTreeWidgetItem(itm);
        bitm->setText(0, lbm);

        for (int j = 1; j < outputTreeWidget->columnCount(); ++j) {
                bitm->setCheckState(j, Qt::Unchecked);
        }

        itm->setExpanded(true);
}

void AudioIODialog::addStereoInput()
{
        QString name = QString(tr("Capture %1")).arg(inputTreeWidget->topLevelItemCount() + 1);

        QTreeWidgetItem *itm = new QTreeWidgetItem();
        itm->setText(0, name);
        inputTreeWidget->addTopLevelItem(itm);

        QString lbl = tr("Left");
        QString lbr = tr("Right");

        QTreeWidgetItem *bitl = new QTreeWidgetItem(itm);
        QTreeWidgetItem *bitr = new QTreeWidgetItem(itm);
        bitl->setText(0, lbl);
        bitr->setText(0, lbr);

        for (int j = 1; j < inputTreeWidget->columnCount(); ++j) {
                bitl->setCheckState(j, Qt::Unchecked);
                bitr->setCheckState(j, Qt::Unchecked);
        }

        itm->setExpanded(true);
}

void AudioIODialog::addStereoOutput()
{
        QString name = QString(tr("Playback %1")).arg(inputTreeWidget->topLevelItemCount() + 1);

        QTreeWidgetItem *itm = new QTreeWidgetItem();
        itm->setText(0, name);
        outputTreeWidget->addTopLevelItem(itm);

        QString lbl = tr("Left");
        QString lbr = tr("Right");

        QTreeWidgetItem *bitl = new QTreeWidgetItem(itm);
        QTreeWidgetItem *bitr = new QTreeWidgetItem(itm);
        bitl->setText(0, lbl);
        bitr->setText(0, lbr);

        for (int j = 1; j < outputTreeWidget->columnCount(); ++j) {
                bitl->setCheckState(j, Qt::Unchecked);
                bitr->setCheckState(j, Qt::Unchecked);
        }

        itm->setExpanded(true);
}

void AudioIODialog::removeInput()
{
        int idx = inputTreeWidget->indexOfTopLevelItem(inputTreeWidget->currentItem());

        if (idx == -1) {
                return;
        }

        QTreeWidgetItem *itm = inputTreeWidget->takeTopLevelItem(idx);
        delete itm;
}

void AudioIODialog::removeOutput()
{
        int idx = outputTreeWidget->indexOfTopLevelItem(outputTreeWidget->currentItem());

        if (idx == -1) {
                return;
        }

        QTreeWidgetItem *itm = outputTreeWidget->takeTopLevelItem(idx);
        delete itm;
}

void AudioIODialog::inputSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
        if (current) {
                buttonRemoveInput->setEnabled(true);
        } else {
                buttonRemoveInput->setEnabled(false);
        }
}

void AudioIODialog::outputSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
        if (current) {
                buttonRemoveOutput->setEnabled(true);
        } else {
                buttonRemoveOutput->setEnabled(false);
        }
}

// we will uncheck all checkboxes but the one that was changed. We don't care if the
// changed one was checked or unchecked, as having none checked is valid, but having
// more than one checked is not.
void AudioIODialog::itemChanged(QTreeWidgetItem *itm, int col)
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
void AudioIODialog::itemDoubleClicked(QTreeWidgetItem *itm, int)
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

QList<bus_config> AudioIODialog::outputBusConfig()
{
        QList<bus_config> list;
        QTreeWidgetItem *header = outputTreeWidget->headerItem();

        while (outputTreeWidget->topLevelItemCount()) {
                bus_config conf;

                QTreeWidgetItem *parent = outputTreeWidget->takeTopLevelItem(0);
                conf.name = parent->text(0);

                while (parent->childCount()) {
                        QTreeWidgetItem *child = parent->takeChild(0);

                        for (int i = 1; i < child->columnCount(); ++i) {
                            if (child->checkState(i) == Qt::Checked) {
                                conf.channels.append(header->text(i));
                            }
                        }

                        delete child;
                }

                delete parent;
                list.append(conf);
        }

        return list;
}

QList<bus_config> AudioIODialog::inputBusConfig()
{
        QList<bus_config> list;
        QTreeWidgetItem *header = inputTreeWidget->headerItem();

        while (inputTreeWidget->topLevelItemCount()) {
                bus_config conf;

                QTreeWidgetItem *parent = inputTreeWidget->takeTopLevelItem(0);
                conf.name = parent->text(0);

                while (parent->childCount()) {
                        QTreeWidgetItem *child = parent->takeChild(0);

                        for (int i = 1; i < child->columnCount(); ++i) {
                            if (child->checkState(i) == Qt::Checked) {
                                conf.channels.append(header->text(i));
                            }
                        }

                        delete child;
                }

                delete parent;
                list.append(conf);
        }

        return list;
}


void AudioIODialog::addJackInput()
{
    int cols = inputTreeWidget->columnCount();
    inputTreeWidget->setColumnCount(cols + 1);
    QTreeWidgetItem *hitem = inputTreeWidget->headerItem();
    hitem->setText(cols, QString(tr("capture_%1")).arg(cols));

    for (int i = 0; i < inputTreeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *titem = inputTreeWidget->topLevelItem(i);

        for (int j = 0; j < titem->childCount(); ++j) {
            QTreeWidgetItem *citem = titem->child(j);
            citem->setCheckState(cols, Qt::Unchecked);
        }
    }
}

void AudioIODialog::addJackOutput()
{
    int cols = outputTreeWidget->columnCount();
    outputTreeWidget->setColumnCount(cols + 1);
    QTreeWidgetItem *hitem = outputTreeWidget->headerItem();
    hitem->setText(cols, QString(tr("playback_%1")).arg(cols));

    for (int i = 0; i < outputTreeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *titem = outputTreeWidget->topLevelItem(i);

        for (int j = 0; j < titem->childCount(); ++j) {
            QTreeWidgetItem *citem = titem->child(j);
            citem->setCheckState(cols, Qt::Unchecked);
        }
    }
}

void AudioIODialog::removeJackInput()
{
    inputTreeWidget->setColumnCount(inputTreeWidget->columnCount() - 1);
}

void AudioIODialog::removeJackOutput()
{
    outputTreeWidget->setColumnCount(outputTreeWidget->columnCount() - 1);
}



//eof

