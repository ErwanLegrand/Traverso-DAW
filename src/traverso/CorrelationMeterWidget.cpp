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

    $Id: CorrelationMeterWidget.cpp,v 1.3 2007/01/15 20:17:02 n_doebelin Exp $
*/

#include <libtraverso.h>

#include "CorrelationMeterWidget.h"
#include <PluginChain.h>
#include <CorrelationMeter.h>
#include "ProjectManager.h"
#include "Project.h"
#include "InputEngine.h"
#include "Song.h"

#include <QPainter>
#include <QColor>
#include <QGradient>
#include <QPixmap>
#include <QFontMetrics>
#include <QTimer>
#include <QLinearGradient>
#include <QColor>
#include <QPointF>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const float SMOOTH_SHIFT = 0.05;
static const int FONT_SIZE = 7;

CorrelationMeterWidget::CorrelationMeterWidget(QWidget* parent)
	: QWidget(parent)
{
	setMinimumWidth(40);
	setMinimumHeight(10);
	
	update_gradient();
	
	// We paint all our pixels ourselves, so no need to let Qt
	// erase and fill it for us prior to the paintEvent.
	// @ Nicola : This is where the high load comes from!
        setAttribute(Qt::WA_OpaquePaintEvent);
	
	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));

	connect(&timer, SIGNAL(timeout()), this, SLOT(update_data()));
}

void CorrelationMeterWidget::paintEvent( QPaintEvent *  )
{
	PENTER3;
	// Since all painting is allready double buffered by Qt, we don't need
	// to create a buffer to paint in ourselves, using a painter like below
	// has the same results (at a lower cost of course, buffering again here
	// means triple buffering which makes no sense.
	QPainter painter(this);

	painter.fillRect(0, 0, width(), height(), QColor(246, 250, 255));

	int lend = int(0.5*width() - (-coeff + 1.0) * 0.25 * width() * (1.0 - fabs(direction)));
	int rend = int(0.5*width() + (-coeff + 1.0) * 0.25 * width() * (1.0 - fabs(direction)));
	
	int wdt = abs(lend - rend);
	int centerOffset = int(width() * 0.25 * direction);

	painter.drawPixmap(lend + centerOffset, 0, wdt, height(), pixPhase);

	painter.setPen(QColor(205, 222, 255));

	int lpos = int(0.25*width());
	int cpos = int(0.50*width());
	int rpos = int(0.75*width());

	painter.drawLine(lpos, 0, lpos, height());
	painter.drawLine(cpos, 0, cpos, height());
	painter.drawLine(rpos, 0, rpos, height());

	// center line
	QPen pen(QColor(180, 190, 189));
	pen.setWidth(3);
	painter.setPen(pen);
	painter.drawLine(width()/2 + centerOffset, 0, width()/2 + centerOffset, height());

	painter.setFont(QFont("Bitstream Vera Sans", FONT_SIZE));
	QFontMetrics fm(QFont("Bitstream Vera Sans", FONT_SIZE));
	
	if (height() < 2*fm.height()) {
		return;
	}

	painter.setPen(QColor(0, 0, 0));
	painter.fillRect(0, 0, width(), fm.height() + 1, QColor(246, 246, 255));
	painter.drawText(lpos - fm.width("L")/2, fm.ascent() + 1, "L");
	painter.drawText(cpos - fm.width("C")/2, fm.ascent() + 1, "C");
	painter.drawText(rpos - fm.width("R")/2, fm.ascent() + 1, "R");
}

void CorrelationMeterWidget::resizeEvent( QResizeEvent *  )
{
	PENTER3;
	update_gradient();
}

void CorrelationMeterWidget::update_data()
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

void CorrelationMeterWidget::set_project(Project *project)
{
	if (project) {
		connect(project, SIGNAL(currentSongChanged(Song *)), this, SLOT(set_song(Song*)));
	} else {
		timer.stop();
	}
}

void CorrelationMeterWidget::set_song(Song *song)
{
	PluginChain* chain = song->get_plugin_chain();
	
	foreach(Plugin* plugin, chain->get_plugin_list()) {
		m_meter = dynamic_cast<CorrelationMeter*>(plugin);
		
		if (m_meter) {
			timer.start(40);
			return;
		}
	}
	
	m_meter = new CorrelationMeter();
	m_meter->init();
	ie().process_command( chain->add_plugin(m_meter, false) );
	timer.start(40);
}

void CorrelationMeterWidget::update_gradient()
{
	gradPhase.setStart(0,0);
	gradPhase.setColorAt(0.0,  QColor(205, 202, 246));
	gradPhase.setColorAt(0.5,  QColor( 82,  80, 123));
	gradPhase.setColorAt(1.0,  QColor(205, 202, 246));

	gradPhase.setFinalStop(QPointF((float)width(), 0.0));

	pixPhase = QPixmap(width(), height());

	QPainter pPhase(&pixPhase);

	pPhase.fillRect(0, 0, width(), height(), gradPhase);
}

//eof
