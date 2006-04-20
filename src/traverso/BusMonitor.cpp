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
 
    $Id: BusMonitor.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include <libtraverso.h>

#include "BusMonitor.h"
#include "Interface.h"
#include "VUMeter.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


BusMonitor::BusMonitor(QWidget* parent, Interface* interface)
                : QWidget( parent)
{
        PENTERCONS;
        m_interface = interface;
        layout = new QHBoxLayout(this);
        layout->setMargin(0);

        QWidget* infoWidget= new QWidget(this);
        QHBoxLayout* infoWidgetsLayout= new QHBoxLayout(infoWidget);
        infoWidgetsLayout->setMargin(0);

        QStringList list = audiodevice().get_capture_buses_names();
        foreach(QString name, list)
        {
                VUMeter* meter = new VUMeter( infoWidget, audiodevice().get_capture_bus(name.toAscii()) );
                infoWidgetsLayout->addWidget(meter);
        }

        list = audiodevice().get_playback_buses_names();
        foreach(QString name, list)
        {
                VUMeter* meter = new VUMeter( infoWidget, audiodevice().get_playback_bus(name.toAscii()) );
                infoWidgetsLayout->addWidget(meter);
        }

        infoWidget->setLayout(infoWidgetsLayout);

        layout->addWidget(infoWidget);
        setLayout(layout);

        setAutoFillBackground(false);

        /*	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(create_vu_meters()));*/
}


BusMonitor::~BusMonitor()
{
        PENTERDES;
}

void BusMonitor::resizeEvent( QResizeEvent * )
{
        PWARN("BusMonitor Resize event");
        PENTER;
}

void BusMonitor::create_vu_meters( )
{
        PENTER;
}

//eof
