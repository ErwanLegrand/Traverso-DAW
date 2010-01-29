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

        m_inputPage->init("capture", audiodevice().get_capture_channel_names());
        m_outputPage->init("playback", audiodevice().get_playback_channel_names());

        while (tabWidget->count()) {
            tabWidget->removeTab(0);
        }

        tabWidget->addTab(m_inputPage, tr("Input"));
        tabWidget->addTab(m_outputPage, tr("Output"));
}

void AudioIODialog::accept()
{
        QStringList c_chan_conf = m_inputPage->getChannelConfig();
        QStringList p_chan_conf = m_outputPage->getChannelConfig();

        audiodevice().set_channel_config(c_chan_conf, p_chan_conf);

        QList<bus_config> c_bus_conf = m_inputPage->getBusConfig();
        QList<bus_config> p_bus_conf = m_outputPage->getBusConfig();

        c_bus_conf.append(p_bus_conf);

        audiodevice().set_bus_config(c_bus_conf);
	
	close();
}


//eof

