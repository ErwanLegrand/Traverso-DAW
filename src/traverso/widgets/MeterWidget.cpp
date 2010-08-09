/*
    Copyright (C) 2008 Remon Sijrier

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


#include "MeterWidget.h"

#include "Command.h"
#include "ContextPointer.h"
#include "PluginChain.h"
#include "ProjectManager.h"
#include "Project.h"
#include "Sheet.h"
#include "TBusTrack.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const int STOP_DELAY = 6000; // in ms

MeterWidget::MeterWidget(QWidget* parent, MeterView* item)
	: ViewPort(parent)
	, m_item(item)
{
	PENTERCONS;
	setMinimumWidth(40);
	setMinimumHeight(10);

	QGraphicsScene* scene = new QGraphicsScene(this);
	setScene(scene);
	
	if (m_item) {
		scene->addItem(m_item);
		m_item->setPos(0,0);
	}

	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

MeterWidget::~MeterWidget()
{
}

void MeterWidget::resizeEvent( QResizeEvent *  )
{
	if (m_item) {
		m_item->resize();
	}
}

void MeterWidget::hideEvent(QHideEvent * event)
{
	QWidget::hideEvent(event);
	if (m_item) {
		m_item->hide_event();
	}
}


void MeterWidget::showEvent(QShowEvent * event)
{
	QWidget::showEvent(event);
	if (m_item) {
		m_item->show_event();
	}
}

QSize MeterWidget::minimumSizeHint() const
{
	return QSize(150, 50);
}

QSize MeterWidget::sizeHint() const
{
	return QSize(220, 50);
}



MeterView::MeterView(MeterWidget* widget)
	: ViewItem(0, 0)
	, m_widget(widget)
	, m_meter(0)
        , m_session(0)
{
        m_project = 0;

	// Nicola: Not sure if we need to initialize here, perhaps a 
	// call to resize would suffice ?
	m_boundingRect = QRectF();

	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
        connect(&timer, SIGNAL(timeout()), this, SLOT(update_data()));
	m_delayTimer.setSingleShot(true);
	connect(&m_delayTimer, SIGNAL(timeout()), this, SLOT(delay_timeout()));
}

MeterView::~MeterView()
{
	if (m_meter) {
                delete m_meter;
	}
}

void MeterView::resize()
{
	PENTER;
	
	prepareGeometryChange();
	// Nicola: Make this as large as the MeterWidget
	// by setting the boundingrect.
	m_boundingRect = QRectF(0, 0, m_widget->width(), m_widget->height());
}

void MeterView::set_project(Project *project)
{
	if (project) {
                m_project = project;
                m_project->add_meter(m_meter);
                connect(m_project, SIGNAL(transportStarted()), this, SLOT(transport_started()));
                connect(m_project, SIGNAL(transportStopped()), this, SLOT(transport_stopped()));
        } else {
		m_project = 0;
		timer.stop();
	}
}

void MeterView::hide_event()
{
        transport_stopped();
}

void MeterView::show_event()
{
        if (m_project && m_project->is_transport_rolling()) {
                transport_started();
        }
}

void MeterView::transport_started()
{
	timer.start(40);
	m_delayTimer.stop();
}

void MeterView::transport_stopped()
{
	m_delayTimer.start(STOP_DELAY);
}

void MeterView::delay_timeout()
{
	timer.stop();
}


//eof
 
