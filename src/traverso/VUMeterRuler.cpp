/*
    Copyright (C) 2006 Nicola Doebelin

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

    $Id: VUMeterRuler.cpp,v 1.1 2006/11/06 19:22:27 n_doebelin Exp $
*/

#include <libtraverso.h>

#include "VUMeterRuler.h"
#include "VUMeter.h"
#include "Mixer.h"

#include <QPainter>
#include <QRect>
#include <QFontMetrics>
#include <QDebug>

#include "Debugger.h"

static const int FONT_SIZE		= 7;
static const int TICK_LINE_LENGTH	= 3;
static const float LUT_MULTIPLY		= 5.0;

VUMeterRuler::VUMeterRuler(QWidget* parent)
                : QWidget(parent)
{
	setAutoFillBackground(false);
	QFontMetrics fm(QFont("Bitstream Vera Sans", FONT_SIZE));
	setMinimumWidth(fm.width("-XX")+TICK_LINE_LENGTH + 3);
	setMaximumWidth(fm.width("-XX")+TICK_LINE_LENGTH + 4);

	yOffset = 0;

	// labels
	presetMark.push_back(6);
	presetMark.push_back(0);
	presetMark.push_back(-5);
	presetMark.push_back(-10);
	presetMark.push_back(-15);
	presetMark.push_back(-20);
	presetMark.push_back(-25);
	presetMark.push_back(-30);
	presetMark.push_back(-35);
	presetMark.push_back(-40);
	presetMark.push_back(-50);
	presetMark.push_back(-60);

	// tick marks
	for (int i = 6; i >= -20; --i) {
		lineMark.push_back(i);
	}
	lineMark.push_back(-25);
	lineMark.push_back(-30);
	lineMark.push_back(-35);
	lineMark.push_back(-40);
	lineMark.push_back(-45);
	lineMark.push_back(-50);
	lineMark.push_back(-60);
}

void VUMeterRuler::paintEvent( QPaintEvent*  )
{
	PENTER4;

	QFontMetrics fm(QFont("Bitstream Vera Sans", FONT_SIZE));
	QString spm;
	int deltaY;

	QPainter painter(this);
	painter.setFont(QFont("Bitstream Vera Sans", FONT_SIZE));

	// offset is the space occupied by the 'over' LED
	float levelRange = float(height() - yOffset);

	// draw line marks
	painter.setPen(Qt::gray);
	for (uint j = 0; j < lineMark.size(); ++j) {
		int idx = int(LUT_MULTIPLY * float((-lineMark[j] + 6)));

		if ((idx < 0) || (idx >= VUMeter::vumeter_lut()->size())) {
			continue;
		}

		deltaY = (int) ( VUMeter::vumeter_lut()->at(idx)/115.0  * levelRange );
		painter.drawLine(0, height() - deltaY, TICK_LINE_LENGTH, height() - deltaY);
	}

	painter.setPen(Qt::black);
	QRect markRect(0, 0, width(), fm.ascent());

	// draw the labels
	for (uint j = 0; j < presetMark.size(); ++j) {

		// skip some labels if the widget is too small
		if ((height() < 120) && ((j == 0) || (j == 2) || (j == 4) || (j == 6) || 
					(j == 7) || (j == 9))) {
			continue;
		}

		// skip some labels if the widget is too small
		if ((height() < 220) && ((j == 8) || (j == 10))) {
			continue;
		}

		int idx = int(LUT_MULTIPLY * float(-presetMark[j] + 6));

		// check the LUT index (I had exceptions without that check)
		if ((idx < 0) || (idx >= VUMeter::vumeter_lut()->size())) {
			continue;
		}

		deltaY = (int) ( VUMeter::vumeter_lut()->at(idx)/115.0  * levelRange );
		spm.sprintf("%2i", presetMark[j]);

		markRect.setY(height() - deltaY - fm.ascent()/2 - 1);
		markRect.setHeight(fm.ascent());
		if (markRect.bottom() >= height()) {
			markRect.translate(0, height() - markRect.bottom() - 1);
		}
		painter.drawText(markRect, Qt::AlignRight, spm);
		painter.drawLine(0, height() - deltaY, TICK_LINE_LENGTH, height() - deltaY);
	}
}

void VUMeterRuler::setYOffset(int i)
{
	yOffset = i;
}


/** EOF **/
