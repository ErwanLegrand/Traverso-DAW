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

$Id: SystemInfoWidget.cpp,v 1.3 2006/05/08 20:05:27 r_sijrier Exp $
*/

#include "SystemInfoWidget.h"
#include "ui_SystemInfoWidget.h"

#include "libtraversocore.h"
#include "ColorManager.h"
#include "DiskIO.h"
#include <AudioDevice.h>

#include <QPixmap>
#include <QByteArray>

#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

SystemInfoWidget::SystemInfoWidget( QWidget * parent )
		: QWidget(parent)
{
	setupUi(this);

	QPalette palette;
	palette.setColor(QPalette::Background, cm().get("INFO_WIDGET_BACKGROUND"));
	setPalette(palette);
	setAutoFillBackground(true);

	audioCardPixmapLabel->setPixmap(QPixmap(":/audiocardsmall"));
	harddrivePixmapLabel->setPixmap(QPixmap(":/harddrivesmall"));
	driverPixmapLabel->setPixmap(QPixmap(":/driver"));
	cpuPixmapLabel->setPixmap(QPixmap(":/cpu"));


	update_driver_info();

	connect(&sytemResourcesTimer, SIGNAL(timeout()), this, SLOT(update_system_resources()));
	connect(&cpuUsageTimer, SIGNAL(timeout()), this, SLOT(update_cpu_usage()));

	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(update_driver_info()));
	connect(&audiodevice(), SIGNAL(xrun()), this, SLOT(update_xrun_info()), Qt::QueuedConnection);

	sytemResourcesTimer.start(2000);
	cpuUsageTimer.start(1000);
}

SystemInfoWidget::~ SystemInfoWidget( )
{}

void SystemInfoWidget::update_system_resources()
{
	Project* project = pm().get_project();
#if HAVE_SYS_VFS_H

	if (project) {
		struct statfs fs;
		statfs(project->get_root_dir().toAscii().data(), &fs);
		double space = floor (fs.f_bavail * (fs.f_bsize / 1048576.0));

		QString s;
		if (space > 9216) {
			s.setNum((space/1024), 'f', 2);
			s.append(" GB");
		} else {
			s.setNum(space, 'f', 0);
			s.append(" MB");
		}
		freeDiskSpaceLabel->setText(s);
	}
#endif

}

void SystemInfoWidget::update_driver_info( )
{
	xrunCount = -1;
	update_xrun_info();

	soundCardLabel->setText(audiodevice().get_device_name());
	driverTypeLabel->setText(audiodevice().get_driver_type());
	bufferSizeLabel->setText(QByteArray::number(audiodevice().get_buffer_size()).append(" frames"));
	rateLabel->setText(QByteArray::number(audiodevice().get_sample_rate()).append(" Hz"));

	QByteArray latency = QByteArray::number( ( (float)  (audiodevice().get_buffer_size() * 2) / audiodevice().get_sample_rate() ) * 1000, 'f', 2 ).append(" ms");
	latencyLabel->setText(latency);

	bitdepthLabel->setText(QByteArray::number(audiodevice().get_bit_depth()).append(" bit"));
}

void SystemInfoWidget::update_xrun_info( )
{
	++xrunCount;
	xrunLabel->setText(QByteArray::number(xrunCount).prepend("xruns "));
}

void SystemInfoWidget::update_cpu_usage( )
{
	float time = audiodevice().get_cpu_time();
	Project* project = pm().get_project();
	
	if (project)
		foreach(Song* song, project->get_song_list() ) {
			time += song->get_diskio()->get_cpu_time();
		}

	cpuLabel->setText(QByteArray::number(time, 'f', 2).append(" %"));
}


//eof



