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

$Id: TimeLineView.cpp,v 1.5 2007/02/05 17:12:02 r_sijrier Exp $
*/

#include "TimeLineView.h"

#include <QPainter>

#include "ProjectManager.h"
#include "Project.h"
#include "Themer.h"
#include "SongView.h"
#include <cmath>
#include <Utils.h>
#include <defines.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

TimeLineView::TimeLineView(SongView* view)
	: ViewItem(0, 0)
{
	PENTERCONS2;
	m_sv = view;
	m_boundingRectangle = QRectF(0, 0, pow(2, 31), 21);
}

TimeLineView::~ TimeLineView()
{
	PENTERDES;
}


void TimeLineView::hzoom_changed( )
{
	update();
}

void TimeLineView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	PENTER;
	Q_UNUSED(widget);
	
	// When the scrollarea moves by a small value, the text
	// can be screwed up, so give it some room, 100 pixels should do!
	int xstart = (int) option->exposedRect.x() - 100;
	int pixelcount = (int) option->exposedRect.width() + 100;
	if (xstart < 0) {
		xstart = 0;
	}
	

	int height = 21;
	
	painter->fillRect(xstart, 0,  pixelcount, height, themer()->get_color("LOCATOR_BACKGROUND") );
	
	painter->setPen(themer()->get_color("LOCATOR_TEXT"));
	painter->setFont( QFont( "Bitstream Vera Sans", 9) );
	
	int rate = pm().get_project()->get_rate();
	nframes_t lastb = xstart * m_sv->scalefactor + pixelcount * m_sv->scalefactor;
	nframes_t firstFrame = xstart * m_sv->scalefactor;

	int x = xstart;
	
	for (nframes_t b = firstFrame; b < lastb; b += (m_sv->scalefactor) ) {
		if (x %10 == 0) {
			painter->drawLine(x, height - 5, x, height - 1);
		}
		if (x % 100 == 0) {
			painter->drawLine(x, height - 13, x, height - 1);
			painter->drawText(x + 4, height - 8, frame_to_smpte(b, rate) );
		}
		x++;
	}
}



//eof
