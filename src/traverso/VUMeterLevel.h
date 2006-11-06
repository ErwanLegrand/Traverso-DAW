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
 
    $Id: VUMeterLevel.h,v 1.4 2006/11/06 19:22:27 n_doebelin Exp $
*/

#ifndef VUMETERLEVEL_H
#define VUMETERLEVEL_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <QColor>
#include <QTimer>
#include <QTimerEvent>
#include <QVector>
#include <QLinearGradient>

class AudioChannel;

class VUMeterLevel : public QWidget
{
	Q_OBJECT
	
public:

        VUMeterLevel(QWidget* parent, AudioChannel* chan);

protected:
        void paintEvent( QPaintEvent* e);
        void resizeEvent( QResizeEvent * );


private:
        bool 		activeTail;
	bool		peakHoldFalling;
        AudioChannel*	m_channel;
        QColor		levelClearColor;
        QPixmap		levelPixmap;
        QPixmap		clearPixmap;
        QTimer 		timer,
			phTimer;
	QLinearGradient	gradient2D,
			gradient3DLeft,
			gradient3DRight;

        float 			presetMark[7];
        float			tailDeltaY;
        float			prevPeakValue;
        float			peak;
	float			rms;
	float			maxFalloff;
	float			peakHoldValue;
	float			peakHistory[50];
	short unsigned int	rmsIndex;
	short unsigned int	overCount;

        void resize_level_pixmap();
	void create_gradients();
	int get_meter_position(float);

private slots:
	void stop();
	void start();
        void update_peak();
	void reset_peak_hold_value();

signals:
	void activate_over_led(bool);

};

#endif

