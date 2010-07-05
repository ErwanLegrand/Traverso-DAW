/*
Copyright (C) 2005-2008 Remon Sijrier

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

#include "BusMonitor.h"

#include "VUMeter.h"

#include <ProjectManager.h>
#include <Project.h>
#include <Sheet.h>
#include <SubGroup.h>
#include <Themer.h>
#include <AudioDevice.h>
#include <AudioBus.h>
#include <QHBoxLayout>
#include <QMenu>
#include <QKeyEvent>
#include <QMouseEvent>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


BusMonitor::BusMonitor(QWidget* parent)
	: QWidget( parent)
{
	PENTERCONS;
	
	setAutoFillBackground(false);
        m_masterOutMeter = m_projectMaster = 0;
        m_sheet = 0;
        m_layout = 0;
	
	m_menu = 0;
	
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

        if (!pm().get_project()) {
                return;
        }

        if (m_layout) delete m_layout;

        m_layout = new QHBoxLayout(this);
        m_layout->setMargin(0);
        setLayout(m_layout);

	while( ! inMeters.isEmpty() ) {
		VUMeter* meter = inMeters.takeFirst();
                m_layout->removeWidget( meter );
		delete meter;
	}

	while ( ! outMeters.isEmpty() ) {
		VUMeter* meter = outMeters.takeFirst();
                m_layout->removeWidget( meter );
		delete meter;
	}
	
        m_layout->addStretch(1);


        //FIXME: restarting the Driver crashes here!!!!!

        QStringList list = pm().get_project()->get_capture_buses_names();
//        foreach(QString name, list) {
//                AudioBus* bus = pm().get_project()->get_capture_bus(name);
//                if (!bus) continue;
//		VUMeter* meter = new VUMeter( this, bus );
//		connect(bus, SIGNAL(monitoringPeaksStarted()), meter, SLOT(peak_monitoring_started()));
//		connect(bus, SIGNAL(monitoringPeaksStopped()), meter, SLOT(peak_monitoring_stopped()));
//                m_layout->addWidget(meter);
//		inMeters.append(meter);
//		meter->hide();
//	}

        // TODO: show playback VU's on demand or ??
        list = pm().get_project()->get_playback_buses_names();
        if (list.size()) {
//                VUMeter* meter = new VUMeter( this, audiodevice().get_playback_bus(list.at(0)));
//		layout->addWidget(meter);
//		outMeters.append(meter);
	}
	
        m_layout->addSpacing(4);

        if (m_masterOutMeter) {
                m_layout->removeWidget(m_masterOutMeter);
                m_layout->removeWidget(m_projectMaster);
                delete m_masterOutMeter;
                delete m_projectMaster;
                m_masterOutMeter = 0;
                m_projectMaster = 0;
        }

        if (m_sheet) {
                AudioBus* bus = m_sheet->get_master_out()->get_process_bus();
                m_masterOutMeter = new VUMeter(this, bus);
                m_layout->addWidget(m_masterOutMeter);
                bus = m_sheet->get_project()->get_master_out()->get_process_bus();
                m_projectMaster = new VUMeter(this, bus);
                m_layout->addWidget(m_projectMaster);
        }

}

void BusMonitor::set_project(Project * project)
{
	Q_UNUSED(project);
	
        if (project) {
                connect(project, SIGNAL(currentSessionChanged(TSession*)), this, SLOT(set_session(TSession*)));
        } else {
                m_sheet = 0;
        }
}

void BusMonitor::set_session(TSession* session)
{
        Project* project = qobject_cast<Project*>(session);
        if (project) {
                return;
        }
        m_sheet = qobject_cast<Sheet*>(session);
        create_vu_meters();
}

void BusMonitor::keyPressEvent(QKeyEvent * event)
{
	if (event->isAutoRepeat()) {
		return;
	} else if (event->key() == Qt::Key_R) {
		reset_vu_meters();
	} else {
		QWidget::keyPressEvent(event);
	}
}

void BusMonitor::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::RightButton) {
		show_menu();
	}
}

void BusMonitor::reset_vu_meters()
{
	foreach(VUMeter* meter, inMeters) {
		meter->reset();
	}
	foreach(VUMeter* meter, outMeters) {
		meter->reset();
	}
        if (m_masterOutMeter) {
                m_masterOutMeter->reset();
                m_projectMaster->reset();
        }
}

void BusMonitor::show_menu()
{
	if (!m_menu) {
		m_menu = new QMenu(this);
		QAction* action = m_menu->addAction("Bus Monitor");
		QFont font(themer()->get_font("ContextMenu:fontscale:actions"));
		font.setBold(true);
		action->setFont(font);
		action->setEnabled(false);
		m_menu->addSeparator();
		action = m_menu->addAction("Reset VU's  < R >");
		connect(action, SIGNAL(triggered(bool)), this, SLOT(reset_vu_meters()));
	}

	m_menu->exec(QCursor::pos());
}

void BusMonitor::enterEvent(QEvent *)
{
        setFocus();
}

