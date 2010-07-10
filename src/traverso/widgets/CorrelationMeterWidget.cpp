/*
    Copyright (C) 2008 Remon Sijrier
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


#include "CorrelationMeterWidget.h"

#include <PluginChain.h>
#include <CorrelationMeter.h>
#include <Command.h>
#include <Sheet.h>
#include <TBusTrack.h>
#include <Themer.h>
#include <Config.h>
#include <cmath> // used for fabs

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const float SMOOTH_SHIFT = 0.05;

CorrelationMeterWidget::CorrelationMeterWidget(QWidget* parent)
	: MeterWidget(parent, new CorrelationMeterView(this))
{
	PENTERCONS;
}


CorrelationMeterView::CorrelationMeterView(CorrelationMeterWidget* widget)
	: MeterView(widget)
{
	load_theme_data();
	load_configuration();
	connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
}

void CorrelationMeterView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	PENTER3;

	float r = 90.0f / range;

	painter->fillRect(0, 0, m_widget->width(), m_widget->height(), m_bgBrush);

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

void CorrelationMeterView::update_data()
{
	if (!m_meter) {
		return;
	}

	// MultiMeter::get_data() will assign it's data to coef and direction
	// if no data was available, return, so we _only_ update the widget when
	// it needs to be!
	if (((CorrelationMeter*)m_meter)->get_data(coeff, direction) == 0) {
		return;
	}

	update();
}

void CorrelationMeterView::set_sheet(Sheet *sheet)
{
	MeterView::set_sheet(sheet);
	
	if ( ! m_sheet ) {
		return;
	}
	
        PluginChain* chain = m_sheet->get_master_out()->get_plugin_chain();
	
	foreach(Plugin* plugin, chain->get_plugin_list()) {
		m_meter = dynamic_cast<CorrelationMeter*>(plugin);
		
		if (m_meter) {
			return;
		}
	}
	
	m_meter = new CorrelationMeter();
	m_meter->init();
	Command::process_command( chain->add_plugin(m_meter, false) );
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

void CorrelationMeterView::load_theme_data()
{
	gradPhase = themer()->get_gradient("CorrelationMeter:foreground");

/** TODO: When I replace QPoint(0, 100) with QPoint(0, m_widget->height()) I get a segmentation fault. WHY??? **/
	m_bgBrush = themer()->get_brush("CorrelationMeter:background", QPoint(0, 0), QPoint(0, 100));
}

//eof
