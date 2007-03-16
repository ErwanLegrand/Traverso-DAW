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

$Id: BusMonitor.cpp,v 1.8 2007/03/16 00:10:26 r_sijrier Exp $
*/

#include <libtraverso.h>

#include "BusMonitor.h"
#include "VUMeter.h"
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
}


BusMonitor::~BusMonitor()
{
	PENTERDES;
}

void BusMonitor::resizeEvent( QResizeEvent* e)
{
	QWidget::resizeEvent(e);
	PENTER2;
}

QSize BusMonitor::sizeHint() const
{
	return QSize( (inMeters.size() + outMeters.size()) * 50, 140);
}

QSize BusMonitor::minimumSizeHint() const
{
	return QSize((inMeters.size() + outMeters.size()) * 20, 50);
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
		layout->addWidget(meter);
		inMeters.append(meter);
	}

	list = audiodevice().get_playback_buses_names();
	foreach(QString name, list)
	{
		VUMeter* meter = new VUMeter( this, audiodevice().get_playback_bus(name.toAscii()) );
		layout->addWidget(meter);
		outMeters.append(meter);
	}
	
	layout->addSpacing(4);
}


//eof

