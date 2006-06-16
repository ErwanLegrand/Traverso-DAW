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

$Id: VUMeter.cpp,v 1.5 2006/06/16 18:45:50 r_sijrier Exp $
*/

#include <libtraverso.h>

#include <VUMeter.h>

#include <QPainter>
#include <QColor>
#include <QHBoxLayout>

#include "ColorManager.h"
#include "Mixer.h"
#include "VUMeterLevel.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"



static const int VU_CHANNEL_WIDTH 			= 7;
static const int TITLE_TO_BUS_DISTANCE		= 18;
static const int SPACE_BETWEEN_CHANNELS 	= 2;
static const int BUS_WIDTH 				= 54;
static const int CARD_INFO_BOX_WIDTH 		= 70;
static const int INDICATOR_WIDTH 			= 25;
static const int MAX_PRESET_MARKS			=  7;
static const float PERCENTUAL_OF_HEIGHT_USED_FOR_VUS = 0.90;


VUMeter::VUMeter(QWidget* parent, AudioBus* bus)
		: QWidget(parent)
{
	QHBoxLayout* hlayout= new QHBoxLayout(this);
	setLayout( hlayout );

	for (int i=0; i<bus->get_channel_count(); ++i) {
		VUMeterLevel* level = new VUMeterLevel(this, bus->get_channel(i));
		level->move( TITLE_TO_BUS_DISTANCE + i * (VU_CHANNEL_WIDTH + SPACE_BETWEEN_CHANNELS), 10);
	}

	m_name = bus->get_name();
	m_channels = 2;
	isActive = greyOut = false;

	setMinimumWidth(BUS_WIDTH);

	presetMark[0]=-3.0f;
	presetMark[1]=-6.0f;
	presetMark[2]=-9.0f;
	presetMark[3]=-18.0f;
	presetMark[4]=-24.0f;
	presetMark[5]=-36.0f;
	presetMark[6]=-96.0f;

	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent);
}

VUMeter::~ VUMeter( )
{}


void VUMeter::paintEvent( QPaintEvent *  )
{
	PENTER3;
	QPainter painter(this);
	int maxh =  height();
	int levelRange = (int) (maxh * PERCENTUAL_OF_HEIGHT_USED_FOR_VUS);
	int vuBottonYoffset = (maxh - levelRange)/2;

	QColor clearColor;

	// draw the base rectangle and bus title
	if (!greyOut) {
		painter.fillRect( 0 , 0 , width(), height() , isActive ? cm().get("BUS_BACKGROUND_ACTIVE") : cm().get("BUS_BACKGROUND") );
		clearColor = cm().get("BUS_LEVEL_CLEAR_COLOR");
	} else {
		painter.fillRect( 0 , 0 , width(), height(), cm().get("BUS_BACKGROUND_GREYD_OUT") );
		clearColor = cm().get("BUS_LEVEL_CLEAR_COLOR_GREYD_OUT");
	}

	//draw bus title
	painter.setPen(Qt::black);
	painter.setFont(QFont("Bitstream Vera Sans", 7));
	painter.save();
	painter.rotate(-90);
	painter.drawText(- maxh + vuBottonYoffset, 12, m_name);
	painter.restore();


	int markX = TITLE_TO_BUS_DISTANCE + ( m_channels * VU_CHANNEL_WIDTH );
	
// 	painter.drawText(markX + 5, maxh - vuBottonYoffset - levelRange + 8, " 0" ); // 0 is always drawn
	
	QFontMetrics fm(QFont("Bitstream Vera Sans", 7));
	QRect markRect(markX + 3, maxh - vuBottonYoffset - levelRange, 
	               fm.width("-96"), fm.ascent());
	
	painter.drawText(markRect, Qt::AlignRight, "0" ); // 0 is always drawn

	QString spm;
	int deltaY;

	for (int j=0; j<MAX_PRESET_MARKS; j++) {
		if ((levelRange<80) && (j==1 ||j==3 || j==4))
			continue; // skip some of them if visible range is not big enough
		if ((levelRange<170) && (j==4))
			continue; // skip this one if visible range is not enough big

		deltaY = (int) ( dB_to_scale_factor(presetMark[j] / 2)  * levelRange );
		spm.sprintf("%2.0f", presetMark[j]);
// 		painter.drawText(markX + 3, maxh - deltaY + 2 - vuBottonYoffset, spm);
		
		markRect.setY(maxh - deltaY + 2 - vuBottonYoffset - fm.ascent());
		markRect.setHeight(fm.ascent());
		painter.drawText(markRect, Qt::AlignRight, spm);
	}

}

void VUMeter::resizeEvent( QResizeEvent *  )
{
	PENTER3;
}



//eof
