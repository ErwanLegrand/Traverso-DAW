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

    $Id: VUMeter.cpp,v 1.9 2006/11/10 22:54:30 n_doebelin Exp $
*/

#include <libtraverso.h>

#include "VUMeter.h"

#include <QPainter>
#include <QColor>
#include <QGradient>
#include <QSpacerItem>
#include <QFontMetrics>

#include "ColorManager.h"
#include "Mixer.h"
#include "VUMeterLevel.h"
#include "VUMeterOverLed.h"

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
 * All VUMeter related classes are designed to be arranged in a QGridLayout as in the
 * following example showing a stereo track with two channels (O = VUMeterOverLed,
 * L = VUMeterLevel, R = VUMeterRuler):
 *
 *  O OR
 *  L LR
 *  L LR
 *  L LR
 *  L LR
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


//eof
