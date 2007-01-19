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

    $Id: VUMeter.cpp,v 1.12 2007/01/19 12:34:04 r_sijrier Exp $
*/

#include "VUMeter.h"

#include <QPainter>
#include <QColor>
#include <QGradient>
#include <QSpacerItem>
#include <QFontMetrics>

#include "ColorManager.h"
#include "Mixer.h"
#include <AudioDevice.h>
#include <AudioChannel.h>
#include <AudioBus.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
 * \class VUMeter
 * \brief A widget holding level indicators, 'over' LEDs, and a scale
 *
 * The VUMeter class is a widget that holds one or more level indicators (VUMeterLevel),
 * one or more 'over' LEDs (VUMeterOverLed), and one scale (VUMeterRuler). Usually one
 * VUMeter per track is created, which contains one level indicator and 'over' LED per
 * audio channel (2 for stereo) and one scale. The name of the track is shown below
 * the level indicators.
 *
 * The scale is hidden automatically if the width of the widget becomes too narrow.
 *
 * All VUMeter related classes are designed to be arranged in a QGridLayout so as to 
 * resize correctly.
 */

static const int MAXIMUM_WIDTH	= 150;
static const int FONT_SIZE 	= 7;

// initialize static variables
QVector<float> VUMeter::lut;

VUMeter::VUMeter(QWidget* parent, AudioBus* bus)
	: QWidget(parent)
{
	setMaximumWidth(MAXIMUM_WIDTH);

	// the base layout
	glayout= new QGridLayout(this);
	int oldSpacing = glayout->spacing();
	glayout->setSpacing(0);
	setLayout( glayout );

	// min_space holds the width above which the tick marks are shown
	minSpace = 2*glayout->margin();
	int yOffset = 0;

	// adding all widget elements to the layout. j counts the channels, i counts
	// the layout rows
	int j = 0;
	for (int i = 0; i < bus->get_channel_count() * 2; ++i) {
		VUMeterOverLed* led = new VUMeterOverLed(this);
		VUMeterLevel* level = new VUMeterLevel(this, bus->get_channel(j));
		connect(level, SIGNAL(activate_over_led(bool)), led, SLOT(set_active(bool)));

		glayout->addWidget(led, 0, i, 1, 1);
		glayout->addWidget(level, 1, i, 1, 1);
		minSpace += level->minimumWidth();
		yOffset = led->minimumHeight();
		++i;

		// we don't want a spacer item after the last level meter, since the
		// ruler widget should touch the last meter
		if (i < bus->get_channel_count() * 2 - 1) {
			glayout->addItem(new QSpacerItem(oldSpacing, 0, QSizePolicy::Fixed,
					 QSizePolicy::Expanding), 0, i, 2, 1);
			minSpace += oldSpacing;
			++j;
		}
	}

	// add a ruler with tickmarks and labels
	ruler = new VUMeterRuler(this);
	ruler->setYOffset(yOffset);
	glayout->addWidget(ruler, 0, glayout->columnCount(), 2, 1 );
	minSpace += ruler->maximumWidth();

	// add a tooltip showing the channel name
	m_name = bus->get_name();
	m_channels = bus->get_channel_count();
	setToolTip(m_name);

	// add the channel name to the bottom of the widget
	glayout->addItem(new QSpacerItem(0, oldSpacing, QSizePolicy::Expanding, QSizePolicy::Fixed),
			 glayout->rowCount(), 0, 1, glayout->columnCount());
	channelNameLabel = new QLabel(0);
	channelNameLabel->setFont(QFont("Bitstream Vera Sans", FONT_SIZE));
	channelNameLabel->setAlignment(Qt::AlignHCenter);
	glayout->addWidget(channelNameLabel, glayout->rowCount(), 0, 1, glayout->columnCount());

	// define the background gradient
	bgGradient.setStart(0,0);
	bgGradient.setColorAt(0.0, QColor(255, 255, 255));
	bgGradient.setColorAt(0.2, QColor(220, 220, 220));
	bgGradient.setColorAt(0.8, QColor(200, 200, 200));
	bgGradient.setColorAt(1.0, QColor(170, 170, 170));
	bgBrush = QBrush(bgGradient);

	// initialize some stuff
	isActive = drawTickmarks = false;
	
	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent);
}

VUMeter::~ VUMeter( )
{}


void VUMeter::paintEvent( QPaintEvent *  )
{
	PENTER3;

	QPainter painter(this);
	painter.fillRect( 0 , 0 , width(), height() , bgBrush );
}

void VUMeter::resizeEvent( QResizeEvent *  )
{
	PENTER3;
	bgGradient.setFinalStop(QPointF(width(), 0));
	bgBrush = QBrush(bgGradient);

	QFontMetrics fm(QFont("Bitstream Vera Sans", FONT_SIZE));
	
	// Comment by Remon: Why the -1 here???? Without the -1 it seems to work correctly too?
	// Reply by Nic: It doesn't here (PPC). The label can't become smaller than the text width,
	//	so we have to elide the text before the label reaches it's minimum width.
	QString label = fm.elidedText(m_name, Qt::ElideMiddle, channelNameLabel->width()-1);
	if (label.length() == 1) {
		label = m_name.left(1) + m_name.right(1);
	}
	channelNameLabel->setText(label);

	bool dt = false;
	if (width() >= minSpace) dt = true;

	// check if the status of drawing tickmarks has changed
	if (dt != drawTickmarks) {
		if (dt) ruler->show();
		else ruler->hide();
		drawTickmarks = dt;
	}
}

void VUMeter::calculate_lut_data()
{
	printf("calculating lut data\n");
	for (int i = 60; i >= -700; i -= 2) {
		if (i >= -200) {
			lut.push_back(100.0 + (float)i * 2.5 / 10.0);
		} else if (i >= -300) {
			lut.push_back( 50.0 + float(i+200) * 2.0 / 10.0);
		} else if (i >= -400) {
			lut.push_back( 30.0 + float(i+300) * 1.5 / 10.0);
		} else if (i >= -500) {
			lut.push_back( 15.0 + float(i+400) * 0.75 / 10.0);
		} else {
			lut.push_back(  7.5 + float(i+500) * 0.5 / 10.0);
		}
	}
}


/**********************************************************************/
/*                      VUMeterRuler                                  */
/**********************************************************************/


/**
 * \class VUMeterRuler
 * \brief A scale which places tick marks and labels from +6.0 to -60.0 dB
 *
 * The scale is compliant with IEC standard 60268-18 and uses VUMeter::vumeter_lut()
 * to map dB values to widget positions.
 */

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



/**********************************************************************/
/*                      VUMeterOverLed                                */
/**********************************************************************/


/**
 * \class VUMeterOverLed
 * \brief An LED-styled widget indicating audio levels > 0.0 dB
 *
 * An LED-styled widget that indicates audio levels > 0.0 dB. It looks like a warning lamp
 * and can be switched on and off. It doesn't analyze audio data itself.
 *
 * A VUMeterOverLed is usually constructed within a VUMeter object in conjunction with a
 * VUMeterLevel object. It can be switched on to indicate the occurrence of audio levels
 * > 0.0 dB.
 */

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



/**********************************************************************/
/*                      VUMeterLevel                                    */
/**********************************************************************/


/**
 * \class VUMeterLevel
 * \brief An audio level indicator widget
 *
 * This is a digital audio level meter which is compliant with the IEC standard
 * 60268-18. It ranges from +6.0 dB to -70.0 dB.
 *
 * A VUMeterLevel is usually constructed within a VUMeter object. The audio level is
 * read from an AudioChannel object, which must be given in the constructor. The
 * VUMeterLevel is optimized for efficient space usage and switches from 3D look
 * to 2D look for narrow sizes.
 */

static const int OVER_SAMPLES_COUNT = 2;	// sensitivity of the 'over' indicator
static const int RMS_SAMPLES = 50;		// number of updates to be stored for RMS calculation
static const int UPDATE_FREQ = 40;		// frame rate of the level meter (update interval in ms)
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


//eof
