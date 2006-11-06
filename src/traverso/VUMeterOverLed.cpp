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

    $Id: VUMeterOverLed.cpp,v 1.1 2006/11/06 19:22:27 n_doebelin Exp $
*/

#include <libtraverso.h>

#include "VUMeterOverLed.h"

#include <QPainter>

#include "ColorManager.h"
#include "Debugger.h"

static const int MINIMUM_WIDTH		= 4;
static const int MINIMUM_HEIGHT		= 8;
static const int THREE_D_LIMIT		= 8;

VUMeterOverLed::VUMeterOverLed(QWidget* parent)
                : QWidget(parent)
{
	setAutoFillBackground(false);
        setMinimumWidth(MINIMUM_WIDTH);
        setMinimumHeight(MINIMUM_HEIGHT);

	isActive = false;
}

void VUMeterOverLed::paintEvent( QPaintEvent*  )
{
        PENTER4;
        QPainter painter(this);

	if (!isActive) {
		painter.fillRect(0, 0, width(), height(), QColor(128, 0, 0));
		return;
	}

	if (width() < THREE_D_LIMIT) {
		painter.fillRect(0, 0, width(), height(), QColor(255, 0, 0));
		return;
	}

	// draw in 3d mode
	painter.fillRect(0, 0, width(), height(), QColor(  0, 0, 0));
	painter.fillRect(2, 2, width()-4, height()-4, QColor(255, 0, 0));
	painter.setPen(QColor(255, 128, 128));
	painter.drawLine(1, 1, 1, height()-2);
	painter.drawLine(1, 1, width()-2, 1);
	painter.setPen(QColor(192, 0, 0));
	painter.drawLine(width()-2, 1, width()-2, height()-2);
	painter.drawLine(1, height()-2, width()-2, height()-2);

}

void VUMeterOverLed::set_active(bool b)
{
	if (b == isActive) {
		return;
	}

	isActive = b;
	update();
}

/** EOF **/
