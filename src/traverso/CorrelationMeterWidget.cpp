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

#include <libtraverso.h>

#include "CorrelationMeterWidget.h"
#include <PluginChain.h>
#include <CorrelationMeter.h>
#include <Command.h>
#include <ProjectManager.h>
#include <Project.h>
#include <InputEngine.h>
#include <Song.h>
#include <Themer.h>
#include <ContextPointer.h>
#include <Config.h>

#include <QtGui>
#include <QDebug>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const float SMOOTH_SHIFT = 0.05;

CorrelationMeterWidget::CorrelationMeterWidget(QWidget* parent)
	: ViewPort(parent)
{
	PENTERCONS;
	setMinimumWidth(40);
	setMinimumHeight(10);

	m_item = new CorrelationMeterView(this);
	
	QGraphicsScene* scene = new QGraphicsScene(this);
	setScene(scene);
	scene->addItem(m_item);
	m_item->setPos(0,0);

	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

CorrelationMeterWidget::~CorrelationMeterWidget()
{
}

void CorrelationMeterWidget::resizeEvent( QResizeEvent *  )
{
	m_item->resize();
}

void CorrelationMeterWidget::hideEvent(QHideEvent * event)
{
	QWidget::hideEvent(event);
	m_item->hide_event();
}


void CorrelationMeterWidget::showEvent(QShowEvent * event)
{
	QWidget::showEvent(event);
	m_item->show_event();
}

QSize CorrelationMeterWidget::minimumSizeHint() const
{
	return QSize(150, 50);
}

QSize CorrelationMeterWidget::sizeHint() const
{
	return QSize(220, 50);
}

void CorrelationMeterWidget::get_pointed_context_items(QList<ContextItem* > &list)
{
	printf("CorrelationMeterWidget::get_pointed_view_items\n");
	QList<QGraphicsItem *> itemlist = items(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
	foreach(QGraphicsItem* item, itemlist) {
		list.append((ViewItem*)item);
	}
	
	printf("itemlist size is %d\n", itemlist.size());
}





CorrelationMeterView::CorrelationMeterView(CorrelationMeterWidget* widget)
	: ViewItem(0, 0)
	, m_widget(widget)
	, m_meter(0)
	, m_song(0)
{
	gradPhase.setColorAt(0.0,  themer()->get_color("CorrelationMeter:foreground:side"));
	gradPhase.setColorAt(0.5,  themer()->get_color("CorrelationMeter:foreground:center"));
	gradPhase.setColorAt(1.0,  themer()->get_color("CorrelationMeter:foreground:side"));

	load_configuration();

	// Nicola: Not sure if we need to initialize here, perhaps a 
	// call to resize would suffice ?
	m_boundingRect = QRectF();

	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));

	connect(&timer, SIGNAL(timeout()), this, SLOT(update_data()));
}

CorrelationMeterView::~CorrelationMeterView()
{
	if (m_meter) {
		delete m_meter;
	}
}

void CorrelationMeterView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	PENTER3;

	float r = 90.0f / range;

	painter->fillRect(0, 0, m_widget->width(), m_widget->height(), themer()->get_color("CorrelationMeter:background"));

	int lend = int(0.5*m_widget->width() - (-coeff + 1.0) * r * m_widget->width() * (1.0 - fabs(direction)));
	int rend = int(0.5*m_widget->width() + (-coeff + 1.0) * r * m_widget->width() * (1.0 - fabs(direction)));
	
	int wdt = abs(lend - rend);
	int centerOffset = int(m_widget->width() * r * direction);

	int lpos = int((0.50 - r) * m_widget->width());
	int cpos = m_widget->width()/2;
	int rpos = int((0.50 + r) * m_widget->width());

	gradPhase.setStart(QPointF(float(lend + centerOffset), 0.0));
	gradPhase.setFinalStop(QPointF(float(rend + centerOffset), 0.0));
	painter->fillRect(lend + centerOffset, 0, wdt, m_widget->height(), gradPhase);

	// center line
	QPen pen(themer()->get_color("CorrelationMeter:centerline"));
	pen.setWidth(3);
	painter->setPen(pen);
	painter->drawLine(m_widget->width()/2 + centerOffset, 0, m_widget->width()/2 + centerOffset, m_widget->height());

	painter->setPen(themer()->get_color("CorrelationMeter:grid"));
	painter->drawLine(cpos, 0, cpos, m_widget->height());
	if (range > 180) {
		painter->drawLine(lpos, 0, lpos, m_widget->height());
		painter->drawLine(rpos, 0, rpos, m_widget->height());
	}

	painter->setFont(themer()->get_font("CorrelationMeter:fontscale:label"));
	QFontMetrics fm(themer()->get_font("CorrelationMeter:fontscale:label"));
	
	if (m_widget->height() < 2*fm.height()) {
		return;
	}

	painter->setPen(themer()->get_color("CorrelationMeter:text"));
	painter->fillRect(0, 0, m_widget->width(), fm.height() + 1, themer()->get_color("CorrelationMeter:margin"));
	painter->drawText(cpos - fm.width("C")/2, fm.ascent() + 1, "C");

	if (range == 180) {
		painter->drawText(1, fm.ascent() + 1, "L");
		painter->drawText(m_widget->width() - fm.width("R") - 1, fm.ascent() + 1, "R");	
	} else {
		painter->drawText(lpos - fm.width("L")/2, fm.ascent() + 1, "L");
		painter->drawText(rpos - fm.width("R")/2, fm.ascent() + 1, "R");
	}
}

void CorrelationMeterView::resize()
{
	PENTER;
	
	prepareGeometryChange();
	// Nicola: Make this as large as the CorrelationMeterWidget
	// by setting the boundingrect.
	m_boundingRect = QRectF(0, 0, m_widget->width(), m_widget->height());
}

void CorrelationMeterView::update_data()
{
	if (!m_meter) {
		return;
	}

	// MultiMeter::get_data() will assign it's data to coef and direction
	// if no data was available, return, so we _only_ update the widget when
	// it needs to be!
	if (m_meter->get_data(coeff, direction) == 0) {
		return;
	}

	update();
}

void CorrelationMeterView::set_project(Project *project)
{
	if (project) {
		connect(project, SIGNAL(currentSongChanged(Song *)), this, SLOT(set_song(Song*)));
	} else {
		set_song(0);
		timer.stop();
	}
}

void CorrelationMeterView::set_song(Song *song)
{
	if (m_widget->parentWidget()->isHidden()) {
		m_song = song;
		return;
	}
	

	if (m_song) {
		if (m_meter) {
			// FIXME The removed plugin still needs to be deleted!!!!!!
			Command::process_command(m_song->get_plugin_chain()->remove_plugin(m_meter, false));
			timer.stop();
		}
	}
	
	m_song = song;
	
	if ( ! m_song ) {
		return;
	}
	
	PluginChain* chain = m_song->get_plugin_chain();
	
	foreach(Plugin* plugin, chain->get_plugin_list()) {
		m_meter = dynamic_cast<CorrelationMeter*>(plugin);
		
		if (m_meter) {
			timer.start(40);
			return;
		}
	}
	
	m_meter = new CorrelationMeter();
	m_meter->init();
	Command::process_command( chain->add_plugin(m_meter, false) );
	timer.start(40);
}

void CorrelationMeterView::hide_event()
{
	if (m_song) {
		if (m_meter) {
			Command::process_command(m_song->get_plugin_chain()->remove_plugin(m_meter, false));
			timer.stop();
		}
	}
}

void CorrelationMeterView::show_event()
{
	set_song(m_song);
}



Command* CorrelationMeterView::set_mode()
{
	switch (range) {
		case 180 : range = 240; break;
		case 240 : range = 360; break;
		case 360 : range = 180; break;
	}
	update();
	save_configuration();
	return 0;
}

void CorrelationMeterView::save_configuration()
{
	config().set_property("CorrelationMeter", "Range", range);
}

void CorrelationMeterView::load_configuration()
{
	range = config().get_property("CorrelationMeter", "Range", 360).toInt();
}

//eof
