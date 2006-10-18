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

$Id: SystemInfoWidget.cpp,v 1.5 2006/10/18 12:08:56 r_sijrier Exp $
*/

#include "SystemInfoWidget.h"

#include "libtraversocore.h"
#include "ColorManager.h"
#include "DiskIO.h"
#include <AudioDevice.h>
#include <Utils.h>

#include <QPixmap>
#include <QByteArray>

#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ResourcesInfoWidget::ResourcesInfoWidget( QWidget * parent )
	: QPushButton(parent)
{
	setIcon(find_pixmap(":/memorysmall"));
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update_resources_status()));
	update_resources_status();
	updateTimer.start(1200);
}

void ResourcesInfoWidget::update_resources_status( )
{
	Project* project = pm().get_project();
	
	if (!project) {
		return;
	}
	
	float time = audiodevice().get_cpu_time();
	int bufReadStatus = 100;
	int bufWriteStatus = 100;
	
	
	foreach(Song* song, project->get_song_list() ) {
		bufReadStatus = std::min(song->get_diskio()->get_read_buffers_fill_status(), bufReadStatus);
		bufWriteStatus = std::min(song->get_diskio()->get_write_buffers_fill_status(), bufWriteStatus);
		time += song->get_diskio()->get_cpu_time();
	}

	QByteArray cputime = QByteArray::number(time, 'f', 2).append(" %");
	if (time < 10.0) {
		cputime.prepend("  ");
	}
	
	QByteArray readstatus = QByteArray::number(bufReadStatus).append("%");
	if (bufReadStatus < 100) {
		readstatus.prepend("  ");
	}
	
	QByteArray writestatus = QByteArray::number(bufWriteStatus).append("%");
	if (bufWriteStatus < 100) {
		writestatus.prepend("  ");
	}
	
	setText(" R " + 
		readstatus + 
		"  W " +
		 writestatus + 
		"  CPU " + cputime);
}




DriverInfoWidget::DriverInfoWidget( QWidget * parent )
	: QPushButton(parent)
{
	setIcon(find_pixmap(":/driver"));
	setToolTip("Click to configure audiodevice");
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(update_driver_info()));
	connect(&audiodevice(), SIGNAL(bufferUnderRun()), this, SLOT(update_xrun_info()));
	update_driver_info();
}

void DriverInfoWidget::update_driver_info( )
{
	xrunCount = 0;
	draw_information();
}

void DriverInfoWidget::draw_information( )
{
	QByteArray latency = QByteArray::number( ( (float)  (audiodevice().get_buffer_size() * 2) / audiodevice().get_sample_rate() ) * 1000, 'f', 2 ).append(" ms");
	
	QByteArray xruns = "";
	if (xrunCount) {
		xruns = QByteArray::number(xrunCount).prepend(" xruns ");
	}
	
	setText(audiodevice().get_driver_type() + 
			"  " +
			QByteArray::number(audiodevice().get_sample_rate()) + 
			"/" + 
			QByteArray::number(audiodevice().get_bit_depth()) + 
			" @ " +
			latency +
			xruns);
}

void DriverInfoWidget::update_xrun_info( )
{
	xrunCount++;
	draw_information();
}



HDDSpaceInfoWidget::HDDSpaceInfoWidget( QWidget * parent )
	: QPushButton(parent)
{
	setIcon(find_pixmap(":/harddrivesmall"));
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update_harddisk_space_info()));
	update_harddisk_space_info();
	updateTimer.start(5000);
}

void HDDSpaceInfoWidget::update_harddisk_space_info( )
{
#if HAVE_SYS_VFS_H
	Project* project = pm().get_project();

	if (!project) {
		setText("No Info");
		return;
	}
	
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
	
	setText(s);

#endif
}

//eof
