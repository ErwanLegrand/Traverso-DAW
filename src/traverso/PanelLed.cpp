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
 
    $Id: PanelLed.cpp,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#include "PanelLed.h"

#include <QPainter>

#include "TrackView.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

const int LED_Y = 18;
const int LED_WIDTH = 35;
const int LED_HEIGHT = 14;


PanelLed::PanelLed( ViewPort * vp, TrackView* parent, int xpos, char* on, char* off )
                : ViewItem(vp, parent), m_trackView(parent), m_xpos(xpos), onType(on), offType(off)
{
        m_isOn = false;
}

PanelLed::~PanelLed( )
{
        PENTERDES2;
}

QRect PanelLed::draw( QPainter & p )
{
        if (m_isOn)
                p.drawPixmap(m_xpos, LED_Y + m_trackView->get_base_y(), QPixmap(onType));
        else
                p.drawPixmap(m_xpos, LED_Y + m_trackView->get_base_y(), QPixmap(offType));

        return QRect();
}

void PanelLed::ison_changed( bool isOn )
{
        PENTER;
        m_isOn = isOn;
        schedule_for_repaint();
}

void PanelLed::set_on_type( char * type )
{
        onType = type;
}

void PanelLed::set_of_type( char * type )
{
        offType = type;
}


void PanelLed::set_xpos( int x )
{
        m_xpos = x;
}

void PanelLed::schedule_for_repaint( )
{
        m_vp->schedule_for_repaint(this);
}

//eof
