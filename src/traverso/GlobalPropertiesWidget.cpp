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

$Id: GlobalPropertiesWidget.cpp,v 1.14 2007/01/15 23:51:47 r_sijrier Exp $
*/

#include "GlobalPropertiesWidget.h"
#include "ui_GlobalPropertiesWidget.h"

#include "libtraversocore.h"

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
	config().set_property("Hardware", "samplerate", defaultSampleRateComboBox->currentText());
	config().set_property("Hardware", "bufferSize", bufferSizeComboBox->currentText());
	config().set_property("Project", "loadLastUsed", (loadLastProjectCheckBox->isChecked() ? 1 : 0));
	config().set_property("Hardware", "drivertype", audioDriverBackendComboBox->currentText());
	config().set_property("AudioClip", "WaveFormRectified", (waveFormRectifiedCheckBox->isChecked() ? 1 : 0));
	config().set_property("AudioClip", "WaveFormMerged", (waveFormMergedCheckBox->isChecked() ? 1 : 0));
	config().set_property("CCE", "holdTimeout", holdTimeoutSpinBox->text());
	config().set_property("CCE", "doublefactTimeout", doubleFactTimeoutSpinBox->text());
	config().set_property("Song", "trackCreationCount", numberOfTrackSpinBox->text());
	QString zoomLevel = defaultHZoomLevelComboBox->currentText().mid(2);
	config().set_property("Song", "hzoomLevel", zoomLevel);
	int playback=1, capture=1;
	if(duplexComboBox->currentIndex() == 1) {
		capture = 0;
	}
	if(duplexComboBox->currentIndex() == 2) {
		playback = 0;
	}
	config().set_property("Hardware", "playback", playback);
	config().set_property("Hardware", "capture", capture);
	config().save();
}


void GlobalPropertiesWidget::load_properties( )
{
	int doubleFactTimeout = config().get_property("CCE", "doublefactTimeout", 200).toInt();
	int holdTimeout = config().get_property("CCE", "holdTimeout", 200).toInt();
	int defaultNumTracks = config().get_property("Song", "trackCreationCount", 6).toInt();
	int hzoomLevel = config().get_property("Song", "hzoomLevel", 8).toInt();
	int waveform = config().get_property("AudioClip", "WaveFormRectified", 0).toInt();
	int waveformmerged = config().get_property("AudioClip", "WaveFormMerged", 0).toInt();
	int loadLastUsedProject = config().get_property("Project", "loadLastUsed", 1).toInt();
	int defaultSampleRate = config().get_property("Hardware", "samplerate", 44100).toInt();
	int bufferSize = config().get_property("Hardware", "bufferSize", 1024).toInt();
	QString driverType = config().get_property("Hardware", "drivertype", "ALSA").toString();
	bool capture = config().get_property("Hardware", "capture", 1).toInt();
	bool playback = config().get_property("Hardware", "playback", 1).toInt();

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
	QString cardDevice = config().get_property("Hardware", "carddevice", "hw:0").toString();
	
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

