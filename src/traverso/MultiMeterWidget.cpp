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

    $Id: MultiMeterWidget.cpp,v 1.3 2006/11/21 19:38:25 n_doebelin Exp $
*/

#include <libtraverso.h>

#include "MultiMeterWidget.h"
#include "MultiMeter.h"
#include "ProjectManager.h"
#include "Project.h"

#include <QPainter>
#include <QColor>
#include <QGradient>
#include <QPixmap>
#include <QFontMetrics>
#include <QTimer>
#include <QLinearGradient>
#include <QColor>
#include <QPointF>
#include <QDebug>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const float SMOOTH_SHIFT = 0.05;

MultiMeterWidget::MultiMeterWidget(QWidget* parent)
	: QWidget(parent)
{
	setMinimumWidth(40);
	setMinimumHeight(10);
	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));

	connect(&timer, SIGNAL(timeout()), this, SLOT(update_data()));
	timer.start(40);
}

void MultiMeterWidget::paintEvent( QPaintEvent *  )
{
	PENTER3;

	QPixmap pix(width(), height());
	QPainter painter(&pix);
	QPainter directpainter(this);

	painter.fillRect(0, 0, width(), height(), Qt::black);

	int lend = int(0.5*width() - (-coeff + 1.0) * 0.25 * width() * (1.0 - fabs(direction)));
	int rend = int(0.5*width() + (-coeff + 1.0) * 0.25 * width() * (1.0 - fabs(direction)));
	int wdt = abs(lend - rend);
	int centerOffset = int(width() * 0.25 * direction);

	painter.drawPixmap(lend + centerOffset, 0, wdt, height(),
		pixPhase, lend, 0, wdt, height());

	painter.setPen(QColor(128, 128, 128));
	painter.drawLine(int(0.25*width()), 0, int(0.25*width()), height());
	painter.drawLine(int(0.50*width()), 0, int(0.50*width()), height());
	painter.drawLine(int(0.75*width()), 0, int(0.75*width()), height());

	painter.setPen(QColor(0, 255, 0));
	painter.drawLine(width()/2 + centerOffset, 0, width()/2 + centerOffset, height());

	directpainter.drawPixmap(0, 0, pix);
}

void MultiMeterWidget::resizeEvent( QResizeEvent *  )
{
	PENTER3;
	update_gradient();
}

void MultiMeterWidget::update_data()
{
	if (!m_multimeter) return;

	coeff = m_multimeter->get_correlation_coefficient();
	direction = m_multimeter->get_direction();

	update();
}

void MultiMeterWidget::set_project(Project *project)
{
	if (project) {
		connect(project, SIGNAL(currentSongChanged(Song *)), this, SLOT(set_song(Song*)));
	}
}

void MultiMeterWidget::set_song(Song *song)
{
	m_multimeter = song->get_multimeter();
}

void MultiMeterWidget::update_gradient()
{
	gradPhase.setStart(0,0);
	gradPhase.setColorAt(0.0,  QColor(100,   0, 0));
	gradPhase.setColorAt(0.25, QColor(100, 100, 0));
	gradPhase.setColorAt(0.5,  QColor(  0, 200, 0));
	gradPhase.setColorAt(0.75, QColor(100, 100, 0));
	gradPhase.setColorAt(1.0,  QColor(100,   0, 0));

	gradPhase.setFinalStop(QPointF((float)width(), 0.0));

	pixPhase = QPixmap(width(), height());

	QPainter pPhase(&pixPhase);

	pPhase.fillRect(0, 0, width(), height(), gradPhase);
}

//eof
