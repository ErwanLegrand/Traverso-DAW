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
 
    $Id: OverViewWidget.cpp,v 1.5 2006/05/11 13:57:09 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "OverViewWidget.h"
#include "ColorManager.h"

#include <QColor>
#include <QPainter>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

OverViewWidget::OverViewWidget(QWidget* parent)
                : QWidget(parent)
{
        QPalette palette;
        palette.setColor(QPalette::Background, QColor(Qt::black));
        setPalette(palette);
        setAutoFillBackground(true);

        setMinimumHeight(15);

        m_song = 0;
}

OverViewWidget::~ OverViewWidget()
{}


void OverViewWidget::set_song(Song* song)
{
        PENTER2;
        m_song = song;
        connect(m_song, SIGNAL(firstVisibleFrameChanged() ), this, SLOT(update( )));
        connect(m_song, SIGNAL(hzoomChanged() ), this, SLOT(update( )));
	connect(m_song, SIGNAL(lastFramePositionChanged() ), this, SLOT(update( )));
}

void OverViewWidget::paintEvent( QPaintEvent*  )
{
        if (!m_song) {
//                 PWARN("No Song/?????");
                return;
        }

        QPainter painter(this);

        double scale = ( (double) width() ) /   m_song->get_last_frame();

        nframes_t totalFrames = m_song->get_last_frame();
        nframes_t startFrame = m_song->get_first_visible_frame();
        nframes_t visibleFrames = width() * Peak::zoomStep[m_song->get_hzoom()];

        if (totalFrames == 0)
                return;

        int startPosition = (int) (startFrame * scale);
        int barWidth = (int) (visibleFrames * scale);

        /*	PWARN("scale is %f", scale);
        	PWARN("visibleFrames is %d", visibleFrames);
        	PWARN("startFrame is %d", startFrame);
        	PWARN("totalFrames is %d", totalFrames);
        	PWARN("width is %d", width());
        	
        	PWARN("startPosition is %d", startPosition);
        	PWARN("barWidth is %d", barWidth);*/

        if (visibleFrames > totalFrames)
                barWidth = width();

        // This creates a round capped location bar.
        QPen pen(cm().get("CLIP_PEAK_MACROVIEW"), height());
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.drawLine(startPosition, height() / 2, startPosition + barWidth, height() / 2);

        // A non round capped location bar....
        // 	painter.fillRect(startPosition, 0, barWidth, height(), cm().get("CLIP_PEAK_MACROVIEW"));

}


//eof

