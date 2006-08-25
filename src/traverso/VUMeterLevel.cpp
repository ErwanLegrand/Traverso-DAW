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

    $Id: VUMeterLevel.cpp,v 1.5 2006/08/25 11:17:58 r_sijrier Exp $
*/

#include <libtraverso.h>

#include "VUMeterLevel.h"

#include <QPainter>
#include <QPixmap>

#include "ColorManager.h"
#include "Debugger.h"


VUMeterLevel::VUMeterLevel(QWidget* parent, AudioChannel* chan)
                : QWidget(parent), m_channel(chan)
{
        levelClearColor  = QColor("black");
        prevPeakValue = peak = tailDeltaY = 0;

        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_PaintOnScreen);
	setAutoFillBackground(false);
        setFixedSize(7, 120);


	connect(&audiodevice(), SIGNAL(stopped()), this, SLOT(stop()));
        timer.start(40, this);
}

void VUMeterLevel::timerEvent( QTimerEvent * event )
{
        if (event->timerId() == timer.timerId())
                update_peak();
}

void VUMeterLevel::paintEvent( QPaintEvent*  )
{
        PENTER4;
        QPixmap pix(width(),height());
        QPainter painter(&pix);
        QPainter directpainter(this);

        int levelRange = height();
        int thisLevelDeltaY = 0;

        if (levelPixmap.height() != levelRange)
                resize_level_pixmap();


        // draw levels

        if (peak)
                thisLevelDeltaY = (int) ( dB_to_scale_factor(coefficient_to_dB(peak) / 2 ) * levelRange);
        else
                thisLevelDeltaY = 0;

        // vu smooth fall
        float sf = tailDeltaY;
        sf *= 0.85f;
        if ( sf < 2 )
                sf  =0;
        tailDeltaY = sf;

        if (thisLevelDeltaY < sf)
                thisLevelDeltaY = (int) sf;
        else
                tailDeltaY = (float) thisLevelDeltaY;


        painter.drawPixmap(0, 0, levelPixmap);
	if ((levelRange - thisLevelDeltaY) > 0)
	        painter.drawPixmap(0, 0, clearPixmap, 0, 0, width(), levelRange - thisLevelDeltaY);

        directpainter.drawPixmap(0, 0, pix);
}

void VUMeterLevel::resize_level_pixmap( )
{
        int levelRange = height();
        int hlr = levelRange/2;

        levelPixmap = QPixmap(width(), levelRange);
        QPainter painter(&levelPixmap);

        for (int i = 0; i <= levelRange; i++) {
                int cR = (int) ( (i < hlr ) ? ((float)i/hlr) * 255 : 255);
                int cG = (int) ( (i < hlr ) ? 255 : 255 * ((float)(levelRange-i)/hlr) );
                cG = cG > 255 ? 255 : cG;
                cR = cG < 0   ?   0 : cR;
                painter.setPen(QColor(cR, cG, 0));
                int y = levelRange - i;
                painter.drawLine(0, y, width(),  y);
        }

        clearPixmap = QPixmap(width(), height());
        clearPixmap.fill(levelClearColor);
}

void VUMeterLevel::update_peak( )
{
        peak = m_channel->get_peak_value();

        if (peak == 0 && prevPeakValue == 0 && tailDeltaY == 0) {
                return;
        } else	{
                prevPeakValue = peak;
                update(0, 0, width(), height() );
        }
}

void VUMeterLevel::stop( )
{
	timer.stop();
}

void VUMeterLevel::start( )
{
}
