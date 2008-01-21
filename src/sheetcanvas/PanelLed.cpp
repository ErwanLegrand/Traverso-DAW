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
 
    $Id: PanelLed.cpp,v 1.1 2008/01/21 16:17:29 r_sijrier Exp $
*/

#include "PanelLed.h"

#include <QPainter>

#include "TrackPanelView.h"
#include <Utils.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

const int LED_Y = 18;
const int LED_WIDTH = 35;
const int LED_HEIGHT = 14;


PanelLed::PanelLed(TrackPanelView* parent, int xpos, char* on, char* off )
	: QObject(parent),  m_xpos(xpos), onType(on), offType(off)
{
        m_isOn = false;
}

PanelLed::~PanelLed( )
{
        PENTERDES2;
}

void PanelLed::paint(QPainter *painter)
{

	if (m_isOn) {
		painter->drawPixmap(m_xpos, LED_Y, find_pixmap(onType));
	} else {
		painter->drawPixmap(m_xpos, LED_Y, find_pixmap(offType));
	}
}

void PanelLed::ison_changed( bool isOn )
{
        PENTER2;
        m_isOn = isOn;
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

//eof
