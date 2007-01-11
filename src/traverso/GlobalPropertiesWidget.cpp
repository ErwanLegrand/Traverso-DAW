/*
Copyright (C) 2005-2006 Remon Sijrier 

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

$Id: GlobalPropertiesWidget.cpp,v 1.13 2007/01/11 11:53:30 r_sijrier Exp $
*/

#include "GlobalPropertiesWidget.h"
#include "ui_GlobalPropertiesWidget.h"

#include "libtraversocore.h"

#include <QSettings>
#include <Config.h>
#include "Traverso.h"
#include <AudioDevice.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

GlobalPropertiesWidget::GlobalPropertiesWidget( QWidget * parent )
		: QWidget(parent)
{
	setupUi(this);

	audiocardLabel->setPixmap(QPixmap(":/audiocard"));
	keyboardLabel->setPixmap(QPixmap(":/keyboard"));
	
	holdTimeoutSpinBox->setMinimum(120);
	doubleFactTimeoutSpinBox->setMinimum(120);

	QStringList drivers = audiodevice().get_available_drivers();
	foreach(QString name, drivers)
	audioDriverBackendComboBox->addItem(name);

	connect(bufferSizeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT (update_latency_label(int)));
	connect(defaultSampleRateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT (update_latency_label(int)));
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(update_driver_info()));

	update_driver_info();
	load_properties();
}

GlobalPropertiesWidget::~ GlobalPropertiesWidget( )
{}

void GlobalPropertiesWidget::save_properties( )
{
	config().set_hardware_property("samplerate", defaultSampleRateComboBox->currentText());
	config().set_hardware_property("bufferSize", bufferSizeComboBox->currentText());
	config().set_project_property("loadLastUsed", (loadLastProjectCheckBox->isChecked() ? 1 : 0));
	config().set_hardware_property("drivertype", audioDriverBackendComboBox->currentText());
	
	bool playback=true, capture=true;
	if(duplexComboBox->currentIndex() == 1) {
		capture = false;
	}
	if(duplexComboBox->currentIndex() == 2) {
		playback = false;
	}
	config().set_hardware_property("playback", playback);
	config().set_hardware_property("capture", capture);
	config().save();
	
	QSettings settings;
	settings.setValue("CCE/holdTimeout", holdTimeoutSpinBox->text());
	settings.setValue("CCE/doublefactTimeout", doubleFactTimeoutSpinBox->text());
	settings.setValue("trackCreationCount", numberOfTrackSpinBox->text());
	QString zoomLevel = defaultHZoomLevelComboBox->currentText().mid(2);
	settings.setValue("hzoomLevel", zoomLevel);
	settings.setValue("WaveFormRectified", (waveFormRectifiedCheckBox->isChecked() ? 1 : 0));
	settings.setValue("WaveFormMerged", (waveFormMergedCheckBox->isChecked() ? 1 : 0));
}


void GlobalPropertiesWidget::load_properties( )
{
	QSettings settings;
	int doubleFactTimeout = settings.value("CCE/doublefactTimeout").toInt();
	int holdTimeout = settings.value("CCE/holdTimeout").toInt();
	int defaultNumTracks = settings.value("trackCreationCount").toInt();
	int hzoomLevel = settings.value("hzoomLevel").toInt();
	int waveform = settings.value("WaveFormRectified").toInt();
	int waveformmerged = settings.value("WaveFormMerged").toInt();
	int loadLastUsedProject = settings.value("Project/loadLastUsed").toInt();
	int defaultSampleRate = settings.value("Hardware/samplerate").toInt();
	int bufferSize = settings.value("Hardware/bufferSize").toInt();
	QString driverType = settings.value("Hardware/drivertype").toString();
	bool capture = config().get_hardware_int_property("capture");
	bool playback = config().get_hardware_int_property("playback");



	int defaultSampleRateIndex = 0;
	int bufferSizeIndex = 0;

	switch(defaultSampleRate) {
	case 22050 :
		defaultSampleRateIndex = 0;
		break;
	case 32000 :
		defaultSampleRateIndex = 1;
		break;
	case 44100 :
		defaultSampleRateIndex = 2;
		break;
	case 48000 :
		defaultSampleRateIndex = 3;
		break;
	case 88200 :
		defaultSampleRateIndex = 4;
		break;
	case 96000 :
		defaultSampleRateIndex = 5;
		break;
	default :
		break;
	}

	switch(bufferSize) {
	case 16 :
		bufferSizeIndex = 0;
		break;
	case 32 :
		bufferSizeIndex = 1;
		break;
	case 64 :
		bufferSizeIndex = 2;
		break;
	case 128 :
		bufferSizeIndex = 3;
		break;
	case 256 :
		bufferSizeIndex = 4;
		break;
	case 512 :
		bufferSizeIndex = 5;
		break;
	case 1024 :
		bufferSizeIndex = 6;
		break;
	case 2048 :
		bufferSizeIndex = 7;
		break;
	case 4096 :
		bufferSizeIndex = 8;
		break;
	default :
		break;
	}

	int driverTypeIndex = audioDriverBackendComboBox->findText(driverType);
	if (driverTypeIndex >= 0)
		audioDriverBackendComboBox->setCurrentIndex(driverTypeIndex);

	int hzoomIndex = defaultHZoomLevelComboBox->findText(QByteArray::number(hzoomLevel).prepend("1:"));
	if (hzoomIndex >= 0)
		defaultHZoomLevelComboBox->setCurrentIndex(hzoomIndex);

	loadLastProjectCheckBox->setChecked(loadLastUsedProject);
	numberOfTrackSpinBox->setValue(defaultNumTracks);
	defaultSampleRateComboBox->setCurrentIndex(defaultSampleRateIndex);
	bufferSizeComboBox->setCurrentIndex(bufferSizeIndex);
	doubleFactTimeoutSpinBox->setValue(doubleFactTimeout);
	holdTimeoutSpinBox->setValue(holdTimeout);
	waveFormRectifiedCheckBox->setChecked(waveform);
	waveFormMergedCheckBox->setChecked(waveformmerged);
	
	if (capture && playback) {
		duplexComboBox->setCurrentIndex(0);
	} else if (playback) {
		duplexComboBox->setCurrentIndex(1);
	} else {
		duplexComboBox->setCurrentIndex(2);
	}
}

void GlobalPropertiesWidget::on_saveButton_clicked( )
{
	// Simulate a click on the apply button for the audiodevice settings!
	/*	on_deviceApplyButton_clicked();*/
	save_properties();
	info().information(tr("Settings Saved!"));
}

void GlobalPropertiesWidget::on_discardButton_clicked( )
{
	load_properties();
}

void GlobalPropertiesWidget::on_defaultsButton_clicked( )
{
	config().reset_settings();
	load_properties();
}

void GlobalPropertiesWidget::on_deviceApplyButton_clicked( )
{
	uint rate = defaultSampleRateComboBox->currentText().toUInt();
	uint bufferSize = bufferSizeComboBox->currentText().toUInt();
	QString driverType  = audioDriverBackendComboBox->currentText();
	bool capture = false, playback = false;
	QString cardDevice = config().get_hardware_string_property("carddevice");
	
	if (duplexComboBox->currentIndex() == 0) {
		capture = playback = true;
	} else if (duplexComboBox->currentIndex() == 1) {
		playback = true;
	} else {
		capture = true;
	}

	audiodevice().set_parameters(rate, bufferSize, driverType, capture, playback, cardDevice);
}

void GlobalPropertiesWidget::update_latency_label( int index)
{
	QByteArray latency = QByteArray::number( ( (float)  (bufferSizeComboBox->currentText().toUInt() * 2) / defaultSampleRateComboBox->currentText().toUInt() ) * 1000, 'f', 2 ).append(" ms");
	latencyLabel->setText(latency);
}

void GlobalPropertiesWidget::update_driver_info( )
{
	cardNameLabel->setText(audiodevice().get_device_name());
}

//eof

