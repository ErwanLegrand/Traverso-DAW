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

$Id: BusMonitor.cpp,v 1.14 2007/10/29 09:00:11 r_sijrier Exp $
*/

#include <libtraverso.h>

#include "BusMonitor.h"
#include "VUMeter.h"
#include "ProjectManager.h"
#include "Project.h"
#include <QHBoxLayout>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


BusMonitor::BusMonitor(QWidget* parent)
	: QWidget( parent)
{
	PENTERCONS;
	
	setAutoFillBackground(false);
	
	create_vu_meters();

	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(create_vu_meters()));
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
}


BusMonitor::~BusMonitor()
{
	PENTERDES;
}

QSize BusMonitor::sizeHint() const
{
	int width = 0;
	foreach(QWidget* widget, outMeters) {
		if (! widget->isHidden()) {
			width += widget->width();
		}
	}
	return QSize(width, 140);
}

QSize BusMonitor::minimumSizeHint() const
{
	return QSize(50, 50);
}

void BusMonitor::create_vu_meters( )
{
	PENTER;

	QLayout* lay = layout();
	
	if (lay) delete lay;

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setMargin(0);
	setLayout(layout);
	
	while( ! inMeters.isEmpty() ) {
		VUMeter* meter = inMeters.takeFirst();
		layout->removeWidget( meter );
		delete meter;
	}

	while ( ! outMeters.isEmpty() ) {
		VUMeter* meter = outMeters.takeFirst();
		layout->removeWidget( meter );
		delete meter;
	}
	
	layout->addStretch(1);

	QStringList list = audiodevice().get_capture_buses_names();
	foreach(QString name, list)
	{
		AudioBus* bus = audiodevice().get_capture_bus(name.toAscii());
		VUMeter* meter = new VUMeter( this, bus );
		connect(bus, SIGNAL(monitoringPeaksStarted()), meter, SLOT(peak_monitoring_started()));
		connect(bus, SIGNAL(monitoringPeaksStopped()), meter, SLOT(peak_monitoring_stopped()));
		layout->addWidget(meter);
		inMeters.append(meter);
		meter->hide();
	}

	list = audiodevice().get_playback_buses_names();
	if (list.size())
	{
		VUMeter* meter = new VUMeter( this, audiodevice().get_playback_bus(list.at(0).toAscii()) );
		layout->addWidget(meter);
		outMeters.append(meter);
	}
	
	layout->addSpacing(4);
}

void BusMonitor::set_project(Project * project)
{
	Q_UNUSED(project);
	
	QStringList list = audiodevice().get_capture_buses_names();
	foreach(QString name, list)
	{
		AudioBus* bus = audiodevice().get_capture_bus(name.toAscii());
		bus->reset_monitor_peaks();
	}
}

//eof

