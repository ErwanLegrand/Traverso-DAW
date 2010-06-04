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

#include <QString>
#include <QTabWidget>

AudioIODialog::AudioIODialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

	QString str = audiodevice().get_device_longname();
	labelDeviceName->setText(str);

        m_inputPage = new AudioIOPage(this);
        m_outputPage = new AudioIOPage(this);

        m_inputPage->init("input", audiodevice().get_capture_channel_names());
        m_outputPage->init("output", audiodevice().get_playback_channel_names());

        while (tabWidget->count()) {
            tabWidget->removeTab(0);
        }

        tabWidget->addTab(m_inputPage, tr("Input"));
        tabWidget->addTab(m_outputPage, tr("Output"));
}

void AudioIODialog::accept()
{
        QList<ChannelConfig> channelConfigs  = m_inputPage->getChannelConfig();
        channelConfigs.append(m_outputPage->getChannelConfig());

//        audiodevice().set_channel_config(channelConfigs);

        QList<BusConfig> busConfigs = m_inputPage->getBusConfig();
        busConfigs.append(m_outputPage->getBusConfig());

        AudioDeviceSetup ads = audiodevice().get_device_setup();
        ads.channelConfigs.clear();
        ads.busConfigs.clear();

        ads.channelConfigs = channelConfigs;
        ads.busConfigs = busConfigs;

        audiodevice().set_parameters(ads);
	
	close();
}


//eof

