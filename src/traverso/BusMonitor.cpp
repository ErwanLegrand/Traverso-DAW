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

$Id: BusMonitor.cpp,v 1.4 2006/08/25 11:16:18 r_sijrier Exp $
*/

#include <libtraverso.h>

#include "BusMonitor.h"
#include "Interface.h"
#include "VUMeter.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


BusMonitor::BusMonitor(QWidget* parent, Interface* iface)
		: QWidget( parent)
{
	PENTERCONS;
	m_interface = iface;
	layout = new QHBoxLayout(this);
	layout->setMargin(0);

	vumeterLayoutWidget = new QWidget(this);
	vumeterLayout = new QHBoxLayout(vumeterLayoutWidget);

	vumeterLayout->setMargin(0);

	vumeterLayoutWidget->setLayout(vumeterLayout);

	layout->addWidget(vumeterLayoutWidget);
	setLayout(layout);

	setAutoFillBackground(false);

	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(create_vu_meters()));
}


BusMonitor::~BusMonitor()
{
	PENTERDES;
}

void BusMonitor::resizeEvent( QResizeEvent * )
{
	PENTER2;
}

void BusMonitor::create_vu_meters( )
{
	PENTER;

	vumeterLayout->setEnabled(false);

	while( ! inMeters.isEmpty() ) {
		VUMeter* meter = inMeters.takeFirst();
		vumeterLayout->removeWidget( meter );
		delete meter;
	}

	while ( ! outMeters.isEmpty() ) {
		VUMeter* meter = outMeters.takeFirst();
		vumeterLayout->removeWidget( meter );
		delete meter;
	}


	QStringList list = audiodevice().get_capture_buses_names();
	foreach(QString name, list)
	{
		AudioBus* bus = audiodevice().get_capture_bus(name.toAscii());
// 		bus->set_monitor_peaks(true);
		VUMeter* meter = new VUMeter( vumeterLayoutWidget, bus );
		vumeterLayout->addWidget(meter);
		inMeters.append(meter);
	}

	list = audiodevice().get_playback_buses_names();
	foreach(QString name, list)
	{
		VUMeter* meter = new VUMeter( vumeterLayoutWidget, audiodevice().get_playback_bus(name.toAscii()) );
		vumeterLayout->addWidget(meter);
		outMeters.append(meter);
	}

/*	AudioBus* bus = audiodevice().get_capture_bus("Capture 1");
	if (bus)
		bus->set_monitor_peaks(true);*/

	vumeterLayout->setEnabled(true);
}

//eof
