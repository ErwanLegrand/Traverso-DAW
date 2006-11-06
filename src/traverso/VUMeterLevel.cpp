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

    $Id: VUMeterLevel.cpp,v 1.8 2006/11/06 19:28:53 n_doebelin Exp $
*/

#include <libtraverso.h>

#include "VUMeterLevel.h"
#include "VUMeter.h"

#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QLinearGradient>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <math.h>

#include "ColorManager.h"
#include "Debugger.h"

static const int MINIMUM_WIDTH = 4;		// minimum width of the widget
static const int THREE_D_LIMIT = 8;		// threshold for 3D-look
static const int OVER_SAMPLES_COUNT = 2;	// sensitivity of the 'over' indicator
static const int RMS_SAMPLES = 50;		// number of updates to be stored for RMS calculation
static const int UPDATE_FREQ = 40;		// frame rate of the level meter (update interval in ms)
static const float LUT_MULTIPLY = 5.0;		// depends on the resolution of the Lookup table
static const int PEAK_HOLD_TIME = 1000;		// peak hold time (ms)
static const int PEAK_HOLD_MODE = 1;		// 0 = no peak hold, 1 = dynamic, 2 = constant
static const bool SHOW_RMS = false;		// toggle RMS lines on / off

VUMeterLevel::VUMeterLevel(QWidget* parent, AudioChannel* chan)
                : QWidget(parent), m_channel(chan)
{
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        levelClearColor  = QColor("black");
        peak = 0.0;
	tailDeltaY = peakHoldValue = rms = -120.0;
	overCount = rmsIndex = 0;
	peakHoldFalling = false;
	create_gradients();

	for (int i = 0; i < RMS_SAMPLES; i++) {
		peakHistory[i] = 0.0;
	}

	// falloff speed, according to IEC 60268-18: 20 dB in 1.7 sec.
	maxFalloff = 20.0 / (1700.0 / (float)UPDATE_FREQ);

        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_PaintOnScreen);
	setAutoFillBackground(false);
        setMinimumWidth(MINIMUM_WIDTH);

	connect(&audiodevice(), SIGNAL(stopped()), this, SLOT(stop()));
	connect(&timer, SIGNAL(timeout()), this, SLOT(update_peak()));
	connect(&phTimer, SIGNAL(timeout()), this, SLOT(reset_peak_hold_value()));

	timer.start(UPDATE_FREQ);
	if (PEAK_HOLD_MODE == 1) {
		phTimer.start(PEAK_HOLD_TIME);
	}
}

void VUMeterLevel::paintEvent( QPaintEvent*  )
{
        PENTER4;
	QPixmap pix(width(), height());
        QPainter painter(&pix);
	QPainter directpainter(this);

	// convert the peak value to dB and make sure it's in a valid range
	float dBVal = coefficient_to_dB(peak);
	if (dBVal > 6.0) {
		dBVal = 6.0;
	}

	// calculate smooth falloff
	if (tailDeltaY - dBVal > maxFalloff) {
		dBVal = tailDeltaY - maxFalloff;
		tailDeltaY -= maxFalloff;
	} else {
		tailDeltaY = dBVal;
	}

	// check for a new peak hold value
	if (peakHoldFalling && (peakHoldValue >= -120.0)) {
		// smooth falloff, a little faster than the level meter, looks better
		peakHoldValue -= 1.2*maxFalloff;
	}

	if (peakHoldValue <= dBVal) {
		peakHoldFalling = false;
		peakHoldValue = dBVal;
		
		// Comment by Remon: EHH?? The phTimer is still running since it _is_ allready started from the 
		// constructor, right? We don't stop the timer anywhere...
		// Reply by Nic: The timer is REstarted here, which is done by QTimer::start(int) as well.
		//	We want the new peak hold value to be held for 1 sec., so we restart the timer
		//	as soon as a new phvalue was detected.
		if (PEAK_HOLD_MODE == 1) {
			phTimer.start(PEAK_HOLD_TIME);
		}
	}

	// convert dB values into widget position
	int meterLevel = get_meter_position(dBVal);
	int rmsLevel = get_meter_position(coefficient_to_dB(rms));
	int peakHoldLevel = get_meter_position(peakHoldValue);

	// draw levels
	painter.drawPixmap(0, 0, clearPixmap, 0, 0, width(), meterLevel);

	if (meterLevel < height()) {
		painter.drawPixmap(0, meterLevel, levelPixmap, 0, meterLevel, width(), height() - meterLevel);
	}

	// draw RMS lines
	if (SHOW_RMS) {
		painter.setPen(Qt::white);
		// Comment by Remon: Somehow the line seems to be one pixel shorter
		// at the right size. Makes the lines look odd (just one pixel)
		// when vu's are very small. Changing - 4 to  - 3 solves it for me
		// any ideas?
		// Reply by Nic: Indeed, obviously drawing stops 1 pixel before the
		//	length specified.
		painter.drawLine(2, rmsLevel, width() - 3, rmsLevel);
	}

	// draw Peak hold lines
	if (PEAK_HOLD_MODE) {
		painter.setPen(Qt::red);
		// Comment by Remon: Same here
		painter.drawLine(2, peakHoldLevel, width() - 3, peakHoldLevel);
	}

	directpainter.drawPixmap(0, 0, pix);
}

void VUMeterLevel::resize_level_pixmap( )
{
        levelPixmap = QPixmap(width(), height());
        QPainter painter(&levelPixmap);

	gradient2D.setFinalStop(QPointF(0.0, height()));

	painter.fillRect(0, 0, width(), height(), gradient2D);

	// 3D look if there's enough space
	if (width() >= THREE_D_LIMIT) {
		gradient3DLeft.setFinalStop(QPointF(0.0, height()));
		gradient3DRight.setFinalStop(QPointF(0.0, height()));

		painter.fillRect(0, 0, 2, height(), gradient3DLeft);
		painter.fillRect(width()-2, 0, width(), height(), gradient3DRight);

		painter.setPen(QPen(Qt::black));
		painter.drawLine(0, 0, 0, height());
		painter.drawLine(width()-1, 0, width()-1, height());
	}

        clearPixmap = QPixmap(width(), height());
        clearPixmap.fill(levelClearColor);
}

void VUMeterLevel::update_peak( )
{
        peak = m_channel->get_peak_value();

	// if the meter drops to -inf, reset the 'over LED' and peak hold values
	if ((peak == 0.0) && (tailDeltaY <= -70.0)) {
		peakHoldValue = -120.0;
		emit activate_over_led(false);
                return;
        }

	// RMS lines
	if (SHOW_RMS) {
		// use some kind of 'ring buffer' to store the last couple of peak values
		if (rmsIndex >= RMS_SAMPLES) {
			rmsIndex = 0;
		}

		peakHistory[rmsIndex] = peak;

		// calculate the RMS
		float squares = 0.0;

		for (int i = 0; i < RMS_SAMPLES; i++) {
			squares += peakHistory[i] * peakHistory[i];
		}

		rms = sqrt(squares / float(RMS_SAMPLES));
		rmsIndex++;
	}

	// 'over' detection
	if (peak >= 1.0) overCount++;
	if (peak <  1.0) overCount = 0;

	if (overCount >= OVER_SAMPLES_COUNT) {
		emit activate_over_led(true);
		overCount = 0;
	}

	update();
}

void VUMeterLevel::stop( )
{
	timer.stop();
	emit activate_over_led(false);
}

void VUMeterLevel::start( )
{
}

void VUMeterLevel::resizeEvent(QResizeEvent *)
{
	resize_level_pixmap();
}

void VUMeterLevel::reset_peak_hold_value()
{
	peakHoldFalling = true;
}

void VUMeterLevel::create_gradients()
{
	float zeroDB = 1.0 - 100.0/115.0;  // 0 dB position
	float msixDB = 1.0 -  80.0/115.0;  // -6 dB position
	float smooth = 0.05;

	gradient2D.setStart(0,0);
	gradient3DLeft.setStart(0,0);
	gradient3DRight.setStart(0,0);
	
	gradient2D.setColorAt(0.0 , QColor(255,   0,   0));
	gradient2D.setColorAt(zeroDB-smooth, QColor(255,   0,   0));
	gradient2D.setColorAt(zeroDB+smooth, QColor(255, 255,   0));
	gradient2D.setColorAt(msixDB-smooth, QColor(255, 255,   0));
	gradient2D.setColorAt(msixDB+smooth, QColor(0, 255,   0));
	gradient2D.setColorAt(1.0 , QColor(  0, 255,   0));

	gradient3DLeft.setColorAt(0.0, QColor(255, 128, 128));
	gradient3DLeft.setColorAt(zeroDB-smooth, QColor(255, 128,   128));
	gradient3DLeft.setColorAt(zeroDB+smooth, QColor(255, 255,   128));
	gradient3DLeft.setColorAt(msixDB-smooth, QColor(255, 255,   128));
	gradient3DLeft.setColorAt(msixDB+smooth, QColor(128, 255,   128));
	gradient3DLeft.setColorAt(1.0, QColor(128, 255, 128));

	gradient3DRight.setColorAt(0.0, QColor(192,   0,   0));
	gradient3DRight.setColorAt(zeroDB-smooth, QColor(192,   0,   0));
	gradient3DRight.setColorAt(zeroDB+smooth, QColor(192, 192,   0));
	gradient3DRight.setColorAt(msixDB-smooth, QColor(192, 192,   0));
	gradient3DRight.setColorAt(msixDB+smooth, QColor(  0, 192,   0));
	gradient3DRight.setColorAt(1.0, QColor(  0, 192,   0));
}

// accepts dB-values and returns the position in the widget from top
int VUMeterLevel::get_meter_position(float f)
{
	int idx = int(LUT_MULTIPLY * (-f + 6.0));

	// Comment by Remon: If it happens, then it's a coding error?
	// if so, it could be an idea to hard check for this by using:
	// Q_ASSERT(idx < 0) (iirc) or, using something like a 
	// qDebug("....") thing. Just an idea...

	// Reply by Nic: It's not a coding error, it happens for f > +6.0.
	//	The following part clips the value to +6.0, which is the
	//	highest value the LUT contains. I changed the comment accordingly.

	// clipping to the highest value covered by the LUT (+6.0 dB)
	if (idx < 0) {
		idx = 0;
	}

	// if idx > size of the LUT, dBVal is somewhere < -70 dB, which is not displayed
	if (idx >= VUMeter::vumeter_lut()->size()) {
		return height();
	} else {
		return  height() - int(VUMeter::vumeter_lut()->at(idx)/115.0 * (float)height());
	}
}



/** EOF **/
