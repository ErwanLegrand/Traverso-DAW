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
 
    $Id: LocatorView.cpp,v 1.2 2006/05/01 21:31:58 r_sijrier Exp $
*/

#include "LocatorView.h"

#include <QPainter>

#include "SongView.h"
#include "TrackView.h"
#include "Peak.h"
#include "Utils.h"
#include "ColorManager.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

LocatorView::LocatorView(SongView* parent, ViewPort* vp)
                : ViewItem(vp, parent), m_sv(parent)
{
        PENTERCONS2;
        paintLocator = true;
        paintLocatorInfo = true;

        connect(m_vp, SIGNAL(resized()), this, SLOT(schedule_for_repaint()));
}

LocatorView::~ LocatorView()
{
        PENTERDES;
}

QRect LocatorView::draw(QPainter& p)
{
        if (paintLocator) {
                draw_locator(p);
                draw_locator_info(p);
                paintLocator = false;
        }

        return QRect();
}


void LocatorView::hzoom_changed( )
{
        paintLocator = true;
        m_vp->schedule_for_repaint(this);
}


void LocatorView::point_changed( )
{
        /*	paintLocatorInfo = true;
        	m_vp->schedule_for_repaint(this);*/
}

void LocatorView::draw_locator( QPainter & p )
{
        PENTER;
        int clipAreaBaseX = TrackView::CLIPAREABASEX;
        int clipAreaWidth = m_vp->width() - TrackView::CLIPAREABASEX;

        // This can happen when ViewPort didn't resize in time. Seems to happen under
        // Qt 4.0.1, but not with Qt4.1.0 ... But it's not a big deal to check for this...
        if (clipAreaWidth < 0)
                return;


        p.fillRect(clipAreaBaseX, 0, clipAreaWidth, LOCATOR_HEIGHT, cm().get("LOCATOR_BACKGROUND") );
        p.setPen(cm().get("LOCATOR_TEXT"));
        p.setFont( QFont( "Bitstream Vera Sans", 8) );
        /*	if (ie().is_jogging() == m_sv->get_song()->JogCreateRegion)
        		{
        		int xs = m_sv->get_song()->block_to_xpos(m_sv->get_song()->origBlockL) + clipAreaBaseX;
        		int xe = m_vp->get_mouse_x();
        		if (xs<0) xs=0;
        		if ( xe > clipAreaWidth )
        			 xe = clipAreaWidth;
        		p.fillRect(xs,0,xe-xs,LOCATOR_HEIGHT,QColor(55,100,150));
        		}*/
        int k=0;
        int rate = pm().get_project()->get_rate();
        int zoomStep = Peak::zoomStep[m_sv->get_song()->get_hzoom()];
        nframes_t lastb = m_sv->get_song()->get_firstblock() + clipAreaWidth * zoomStep;
        nframes_t firstFrame = m_sv->get_song()->get_firstblock();

        for (nframes_t b = firstFrame; b < lastb; b += (zoomStep * 10) ) {
                if (b < firstFrame)
                        continue;
                int x = m_sv->get_song()->block_to_xpos(b) + clipAreaBaseX;
                p.drawLine(x, 19, x, LOCATOR_HEIGHT);
                if (++k>10) {
                        p.drawLine(x, 8, x, LOCATOR_HEIGHT);
                        p.drawText(x + 7, 15, frame_to_smpte(b, rate) );
                        k=0;
                }
        }
}

void LocatorView::draw_locator_info( QPainter & p )
{
        p.fillRect(0, 0, TrackView::TRACKPANELWIDTH, LOCATOR_HEIGHT, cm().get("LOCATOR_BACKGROUND"));
        p.setFont( QFont( "Bitstream Vera Sans", 9));
        p.drawText(120, 18, "SMPTE: ");
}


void LocatorView::schedule_for_repaint( )
{
        paintLocator = true;
        paintLocatorInfo = true;
        m_vp->schedule_for_repaint(this);
}


//eof
