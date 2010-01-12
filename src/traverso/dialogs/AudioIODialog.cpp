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

#include <QTableWidget>
#include <QTableWidgetItem>
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
                groupBoxJackInput->setEnabled(true);
                groupBoxJackOutput->setEnabled(true);
        } else {
                groupBoxJackInput->setEnabled(false);
                groupBoxJackOutput->setEnabled(false);
        }

	initInput();
	initOutput();
}

void AudioIODialog::initInput()
{
	m_inputChannelList = audiodevice().get_capture_channel_names();
	const QHash<QString, QStringList> busList = audiodevice().get_capture_bus_configuration();
	
	tableWidgetInput->setColumnCount(m_inputChannelList.count() + 2);
	tableWidgetInput->setRowCount(1);
	
	for (int i = 0; i < m_inputChannelList.count(); ++i) {
		tableWidgetInput->setItem(0, i + 2, new QTableWidgetItem(m_inputChannelList.at(i)));
	}

	QHash<QString, QStringList>::const_iterator it = busList.constBegin();

	while (it != busList.constEnd()) {

		int span_start = tableWidgetInput->rowCount();
		int span_end = 0;
		QString side = "L";
		QStringList busChannels = it.value();
		for (int k = 0; k < busChannels.count(); ++k) {
			if (busChannels.count() <= 1) {
				side = "M";
			}
		  
			int j = tableWidgetInput->rowCount();
			tableWidgetInput->setRowCount(j + 1);
			tableWidgetInput->setItem(j, 0, new QTableWidgetItem(it.key()));
			
			for (int i = 0; i < m_inputChannelList.count(); ++i) {
				tableWidgetInput->setItem(j, 1, new QTableWidgetItem(side));
				
				QTableWidgetItem *itm = new QTableWidgetItem();
				itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

				if (busChannels.at(k) ==  m_inputChannelList.at(i)) {
					itm->setCheckState(Qt::Checked);
				} else {
					itm->setCheckState(Qt::Unchecked);
				}
			
				tableWidgetInput->setItem(j, i+2, itm);
			}
			side = "R";
			++span_end;
		}
		tableWidgetInput->setSpan(span_start, 0, span_end, 1);
		++it;
	}
}

void AudioIODialog::initOutput()
{
	m_outputChannelList = audiodevice().get_playback_channel_names();
	const QHash<QString, QStringList> busList = audiodevice().get_playback_bus_configuration();
	
	tableWidgetOutput->setColumnCount(m_outputChannelList.count() + 2);
	tableWidgetOutput->setRowCount(1);
	
	for (int i = 0; i < m_outputChannelList.count(); ++i) {
		tableWidgetOutput->setItem(0, i + 2, new QTableWidgetItem(m_outputChannelList.at(i)));
	}

	QHash<QString, QStringList>::const_iterator it = busList.constBegin();

	while (it != busList.constEnd()) {

		int span_start = tableWidgetOutput->rowCount();
		int span_end = 0;
		QString side = "L";
		QStringList busChannels = it.value();
		for (int k = 0; k < busChannels.count(); ++k) {
			if (busChannels.count() <= 1) {
				side = "M";
			}
		  
			int j = tableWidgetOutput->rowCount();
			tableWidgetOutput->setRowCount(j + 1);
			tableWidgetOutput->setItem(j, 0, new QTableWidgetItem(it.key()));
			
			for (int i = 0; i < m_outputChannelList.count(); ++i) {
				tableWidgetOutput->setItem(j, 1, new QTableWidgetItem(side));
				
				QTableWidgetItem *itm = new QTableWidgetItem();
				itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

				if (busChannels.at(k) ==  m_outputChannelList.at(i)) {
					itm->setCheckState(Qt::Checked);
				} else {
					itm->setCheckState(Qt::Unchecked);
				}
			
				tableWidgetOutput->setItem(j, i+2, itm);
			}
			side = "R";
			++span_end;
		}
		tableWidgetOutput->setSpan(span_start, 0, span_end, 1);
		++it;
	}
}

void AudioIODialog::accept()
{
	QHash<QString, QStringList> ohash = outputBusConfig();
	QHash<QString, QStringList> ihash = inputBusConfig();

	audiodevice().set_bus_config(ihash, ohash);
	
	close();
}


void AudioIODialog::addMonoInput()
{
	int rc = tableWidgetInput->rowCount();
	tableWidgetInput->setRowCount(rc + 1);
	
	tableWidgetInput->setItem(rc, 0, new QTableWidgetItem(QString(tr("Input %1")).arg(rc)));
	tableWidgetInput->setItem(rc, 1, new QTableWidgetItem("M"));
	
	for (int i = 2; i < tableWidgetInput->columnCount(); ++i) {
		QTableWidgetItem *itm = new QTableWidgetItem();
		itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		itm->setCheckState(Qt::Unchecked);
		tableWidgetInput->setItem(rc, i, itm);		  
	}
}


void AudioIODialog::addMonoOutput()
{
	int rc = tableWidgetOutput->rowCount();
	tableWidgetOutput->setRowCount(rc + 1);
	
	tableWidgetOutput->setItem(rc, 0, new QTableWidgetItem(QString(tr("Output %1")).arg(rc)));
	tableWidgetOutput->setItem(rc, 1, new QTableWidgetItem("M"));
	
	for (int i = 2; i < tableWidgetOutput->columnCount(); ++i) {
		QTableWidgetItem *itm = new QTableWidgetItem();
		itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		itm->setCheckState(Qt::Unchecked);
		tableWidgetOutput->setItem(rc, i, itm);		  
	}
}

void AudioIODialog::addStereoInput()
{
	int rc = tableWidgetInput->rowCount();
	tableWidgetInput->setRowCount(rc + 2);
	
	tableWidgetInput->setItem(rc, 0, new QTableWidgetItem(QString(tr("Input %1")).arg(rc)));
	tableWidgetInput->setSpan(rc, 0, 2, 1);
	tableWidgetInput->setItem(rc, 1, new QTableWidgetItem("L"));
	tableWidgetInput->setItem(rc+1, 1, new QTableWidgetItem("R"));
	
	for (int i = 2; i < tableWidgetInput->columnCount(); ++i) {
		QTableWidgetItem *litm = new QTableWidgetItem();
		QTableWidgetItem *ritm = new QTableWidgetItem();
		litm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		ritm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		litm->setCheckState(Qt::Unchecked);
		ritm->setCheckState(Qt::Unchecked);
		tableWidgetInput->setItem(rc, i, litm);
		tableWidgetInput->setItem(rc+1, i, ritm);
	}
}

void AudioIODialog::addStereoOutput()
{
	int rc = tableWidgetOutput->rowCount();
	tableWidgetOutput->setRowCount(rc + 2);
	
	tableWidgetOutput->setItem(rc, 0, new QTableWidgetItem(QString(tr("Output %1")).arg(rc)));
	tableWidgetOutput->setSpan(rc, 0, 2, 1);
	tableWidgetOutput->setItem(rc, 1, new QTableWidgetItem("L"));
	tableWidgetOutput->setItem(rc+1, 1, new QTableWidgetItem("R"));
	
	for (int i = 2; i < tableWidgetOutput->columnCount(); ++i) {
		QTableWidgetItem *litm = new QTableWidgetItem();
		QTableWidgetItem *ritm = new QTableWidgetItem();
		litm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		ritm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		litm->setCheckState(Qt::Unchecked);
		ritm->setCheckState(Qt::Unchecked);
		tableWidgetOutput->setItem(rc, i, litm);
		tableWidgetOutput->setItem(rc+1, i, ritm);
	}
}

void AudioIODialog::removeInput()
{
	int row = tableWidgetInput->currentItem()->row();
	int col = tableWidgetInput->currentItem()->column();
	int rowspan = tableWidgetInput->rowSpan(row, col) - 1;

	// this is a workaround. Shifting spans should be fixed in Qt 4.6
	if (rowspan) {
		tableWidgetInput->setSpan(row, col, 1, 1);
	}
 
	// remove all rows spanned by the first cell
	while (rowspan >= 0) {
		if (row + rowspan < tableWidgetInput->rowCount()) {
			tableWidgetInput->removeRow(row + rowspan);
		}		
		--rowspan;
	}
}

void AudioIODialog::removeOutput()
{
	int row = tableWidgetOutput->currentItem()->row();
	int col = tableWidgetOutput->currentItem()->column();
	int rowspan = tableWidgetOutput->rowSpan(row, col) - 1;

	// this is a workaround. Shifting spans should be fixed in Qt 4.6
	if (rowspan) {
		tableWidgetOutput->setSpan(row, col, 1, 1);
	}
 
	// remove all rows spanned by the first cell
	while (rowspan >= 0) {
		if (row + rowspan < tableWidgetOutput->rowCount()) {
			tableWidgetOutput->removeRow(row + rowspan);
		}		
		--rowspan;
	}
}

void AudioIODialog::inputSelectionChanged(int x, int y)
{
	if (y > 0 || x == 0) {
		buttonRemoveInput->setEnabled(false);
	} else {
		buttonRemoveInput->setEnabled(true);
	}
}

void AudioIODialog::outputSelectionChanged(int x, int y)
{
	if (y > 0 || x == 0) {
		buttonRemoveOutput->setEnabled(false);
	} else {
		buttonRemoveOutput->setEnabled(true);
	}
}

QHash<QString, QStringList> AudioIODialog::outputBusConfig()
{
	QHash<QString, QStringList> hash;
	QString bus;
	QStringList channels;
	QTableWidgetItem *b_item;
	QTableWidgetItem *c_item;
	
	if (tableWidgetOutput->rowCount() <= 1) {
		return hash;
	}
	
	int i = 1;
	while (i < tableWidgetOutput->rowCount()) {
		channels.clear();
		b_item = tableWidgetOutput->item(i, 0);
		
		if (!b_item) {
			break;
		}

		bus = b_item->text();
		
		for (int j = 0; j < tableWidgetOutput->rowSpan(b_item->row(), 0); ++j) {
			if (j) {
				++i;
			}

			for (int k = 2; k < tableWidgetOutput->columnCount(); ++k) {
				c_item = tableWidgetOutput->item(i, k);
				if (c_item->checkState() == Qt::Checked) {
					channels.append(tableWidgetOutput->item(0, k)->text());
				}
			}
		}
		hash.insert(bus, channels);
		++i;
	}
	
	return hash;
}

QHash<QString, QStringList> AudioIODialog::inputBusConfig()
{
	QHash<QString, QStringList> hash;
	QString bus;
	QStringList channels;
	QTableWidgetItem *b_item;
	QTableWidgetItem *c_item;
	
	if (tableWidgetInput->rowCount() <= 1) {
		return hash;
	}
	
	int i = 1;
	while (i < tableWidgetInput->rowCount()) {
		channels.clear();
		b_item = tableWidgetInput->item(i, 0);

		if (!b_item) {
			break;
		}
		
		bus = b_item->text();
		
		for (int j = 0; j < tableWidgetInput->rowSpan(b_item->row(), 0); ++j) {
			if (j) {
				++i;
			}

			for (int k = 2; k < tableWidgetInput->columnCount(); ++k) {
				c_item = tableWidgetInput->item(i, k);
				if (c_item->checkState() == Qt::Checked) {
					channels.append(tableWidgetInput->item(0, k)->text());
				}
			}
		}
		hash.insert(bus, channels);
		++i;
	}

	return hash;
}

//eof

