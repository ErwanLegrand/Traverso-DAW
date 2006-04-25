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
 
    $Id: VUMeterLevel.h,v 1.2 2006/04/25 17:22:13 r_sijrier Exp $
*/

#ifndef VUMETERLEVEL_H
#define VUMETERLEVEL_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <QColor>
#include <QBasicTimer>

class AudioChannel;

class VUMeterLevel : public QWidget
{
	Q_OBJECT
	
public:

        VUMeterLevel(QWidget* parent, AudioChannel* chan);

protected:
        void paintEvent( QPaintEvent* e);
        void timerEvent(QTimerEvent *event);


private:
        bool 			activeTail;
        AudioChannel*	m_channel;
        QColor		levelClearColor;
        QPixmap		levelPixmap;
        QPixmap		clearPixmap;
        QBasicTimer 	timer;

        float 			presetMark[7];
        float			tailDeltaY;
        float			prevPeakValue;
        float			 peak;

        void resize_level_pixmap();
        void update_peak();

private slots:
	void stop();
	void start();
};

#endif

