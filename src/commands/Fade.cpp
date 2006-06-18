/*
Copyright (C) 2006 Remon Sijrier 

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

$Id: Fade.cpp,v 1.2 2006/06/18 19:40:02 r_sijrier Exp $
*/


#include "Fade.h"

#include "Curve.h"
#include "AudioClip.h"
#include "ContextPointer.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Fade::Fade(AudioClip* clip, Curve* curve, int direction)
	: Command(clip)
{
	m_curve = curve;
	m_direction = direction;
}


Fade::~Fade()
{}

int Fade::prepare_actions()
{
	return 1;
}

int Fade::begin_hold()
{
	origX = cpointer().x();
	newFade = origFade = m_curve->get_range();
	return 1;
}


int Fade::finish_hold()
{
	return 1;
}


int Fade::do_action()
{
	m_curve->set_range( newFade );
	return 1;
}


int Fade::undo_action()
{
	m_curve->set_range( origFade );
	return 1;
}


int Fade::jog()
{
	int dx = (origX - (cpointer().x()) ) * m_direction;
	newFade = origFade - ( dx * 1000 );
	
	if (newFade < 1) {
		newFade = 1;
	}
	
	m_curve->set_range( newFade );
	
	return 1;
}

// eof

