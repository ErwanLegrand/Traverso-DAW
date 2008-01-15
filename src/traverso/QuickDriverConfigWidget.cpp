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

$Id: QuickDriverConfigWidget.cpp,v 1.12 2008/01/15 19:51:49 r_sijrier Exp $
*/

#include "QuickDriverConfigWidget.h"
#include "ui_QuickDriverConfigWidget.h"

#include <AudioDevice.h>
#include <Config.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

QuickDriverConfigWidget::QuickDriverConfigWidget( QWidget * parent )
	: QWidget(parent, Qt::Popup)
{
	setupUi(this);
	
	periodBufferSizesList << 16 << 32 << 64 << 128 << 256 << 512 << 1024 << 2048 << 4096;
	
	connect(driverComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(driver_combobox_index_changed(QString)));
	connect(rateComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(rate_combobox_index_changed(QString)));
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(driver_params_changed()));
	
	okButton->setDefault(true);
	
	QStringList drivers = audiodevice().get_available_drivers();
	foreach(QString name, drivers) {
		driverComboBox->addItem(name);
	}
		
	update_driver_info();
}

QuickDriverConfigWidget::~ QuickDriverConfigWidget( )
{}


void QuickDriverConfigWidget::on_cancelButton_clicked( )
{
	hide();
	update_driver_info();
}

void QuickDriverConfigWidget::on_okButton_clicked( )
{
	QString driver = driverComboBox->currentText();
	int rate = rateComboBox->currentText().toInt();
	int bufSize = periodBufferSizesList.at(latencyComboBox->currentIndex());
	bool capture = config().get_property("Hardware", "capture", 1).toInt();
	bool playback = config().get_property("Hardware", "playback", 1).toInt();
	QString cardDevice = "";
	QString ditherShape = config().get_property("Hardware", "DitherShape", "None").toString();

#if defined (ALSA_SUPPORT)
	if (driver == "ALSA") {
		cardDevice = config().get_property("Hardware", "carddevice", "hw:0").toString();
	}
#endif
	
#if defined (PORTAUDIO_SUPPORT)
	if (driver == "PortAudio") {
#if defined (Q_WS_X11)
		cardDevice = config().get_property("Hardware", "pahostapi", "alsa").toString();
#elif defined (Q_WS_MAC)
		cardDevice = config().get_property("Hardware", "pahostapi", "coreaudio").toString();
#elif defined (Q_WS_WIN)
		cardDevice = config().get_property("Hardware", "pahostapi", "wmme").toString();
#endif
	}
#endif // end PORTAUDIO_SUPPORT
	
	
	audiodevice().set_parameters(rate, bufSize, driver, capture, playback, cardDevice, ditherShape);

	config().set_property("Hardware", "samplerate", rateComboBox->currentText().toInt());
	config().set_property("Hardware", "buffersize", periodBufferSizesList.at(latencyComboBox->currentIndex()));
	config().set_property("Hardware", "drivertype", driverComboBox->currentText());

	hide();
}

void QuickDriverConfigWidget::driver_combobox_index_changed( QString )
{
}

void QuickDriverConfigWidget::rate_combobox_index_changed( QString )
{
	update_latency_combobox();
}

void QuickDriverConfigWidget::update_latency_combobox( )
{
	latencyComboBox->clear();
	int rate = rateComboBox->currentText().toInt();
	int bufferSize = audiodevice().get_buffer_size();
	
	for (int i=0; i<periodBufferSizesList.size(); ++i) {
		QString latency = QString::number( ((float)(periodBufferSizesList.at(i)) / rate) * 1000 * 2, 'f', 2);
		latencyComboBox->addItem(latency);
	}
	
	int index = periodBufferSizesList.indexOf(bufferSize);
	latencyComboBox->setCurrentIndex(index);
}

void QuickDriverConfigWidget::update_driver_info( )
{
	int driverIndex = driverComboBox->findText(audiodevice().get_driver_type());
	if (driverIndex >= 0) {
		driverComboBox->setCurrentIndex(driverIndex);
	} else {
		PERROR("AudioDevice returned a driver type which is not found in the driverComboBox????");
	}
	
	int rateIndex = rateComboBox->findText(QString::number(audiodevice().get_sample_rate()));
	if (rateIndex >= 0) {
		rateComboBox->setCurrentIndex(rateIndex);
	} else {
		PERROR("AudioDevice returned a samplerate which is not found in the rateComboBox????");
	}
		
	
	update_latency_combobox();
}

void QuickDriverConfigWidget::driver_params_changed( )
{
	if (isHidden()) {
		update_driver_info();
	}
}


//eof
 
