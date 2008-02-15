/*
    Copyright (C) 2005-2006 Nicola Doebelin

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
#include "InputEngine.h"
#include "PluginChain.h"
#include "ProjectManager.h"
#include "Project.h"
#include "Sheet.h"
#include "Themer.h"

#include <QDebug>

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

void MeterWidget::get_pointed_context_items(QList<ContextItem* > &list)
{
	printf("MeterWidget::get_pointed_view_items\n");
	QList<QGraphicsItem *> itemlist = items(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
	foreach(QGraphicsItem* item, itemlist) {
		if (ViewItem::is_viewitem(item)) {
			list.append((ViewItem*)item);
		}
	}
	
	printf("itemlist size is %d\n", itemlist.size());
}


MeterView::MeterView(MeterWidget* widget)
	: ViewItem(0, 0)
	, m_widget(widget)
	, m_meter(0)
	, m_sheet(0)
{
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
		// FIXME!
// 		delete m_meter;
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
		connect(project, SIGNAL(currentSheetChanged(Sheet *)), this, SLOT(set_sheet(Sheet*)));
		m_project = project;
	} else {
		m_project = 0;
		set_sheet(0);
		timer.stop();
	}
}

void MeterView::set_sheet(Sheet *sheet)
{
	if (m_widget->parentWidget()->isHidden()) {
		m_sheet = sheet;
		return;
	}
	

	if (m_sheet) {
		if (m_meter) {
			// FIXME The removed plugin still needs to be deleted!!!!!!
			Command::process_command(m_sheet->get_plugin_chain()->remove_plugin(m_meter, false));
			timer.stop();
			disconnect(m_sheet, SIGNAL(transferStopped()), this, SLOT(transport_stopped()));
			disconnect(m_sheet, SIGNAL(transferStarted()), this, SLOT(transport_started()));
		}
	}
	
	m_sheet = sheet;
	
	if ( ! m_sheet ) {
		return;
	}

	connect(m_sheet, SIGNAL(transferStopped()), this, SLOT(transport_stopped()));
	connect(m_sheet, SIGNAL(transferStarted()), this, SLOT(transport_started()));
}

void MeterView::hide_event()
{
	if (m_sheet) {
		if (m_meter) {
			Command::process_command(m_sheet->get_plugin_chain()->remove_plugin(m_meter, false));
			timer.stop();
		}
	}
}

void MeterView::show_event()
{
	if (m_sheet) {
		if (m_meter) {
			Command::process_command(m_sheet->get_plugin_chain()->add_plugin(m_meter, false));
			timer.start(40);
		} else {
			set_sheet(m_sheet);
		}
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
 
