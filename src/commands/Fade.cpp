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

$Id: Fade.cpp,v 1.5 2006/11/08 14:52:11 r_sijrier Exp $
*/


#include "Fade.h"

#include "Curve.h"
#include "AudioClip.h"
#include "ContextPointer.h"
#include <ViewPort.h>
#include <FadeCurve.h>
		
// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const float CURSOR_SPEED		= 150.0;
static const float RASTER_SIZE		= 0.05;


Fade::Fade(AudioClip* clip, Curve* curve, int direction)
	: Command(clip, QObject::tr("Fade"))
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

int Fade::begin_hold(int useX, int useY)
{
	set_cursor_shape(useX, useY);

	origX = cpointer().x();
	newFade = origFade = m_curve->get_range();
	return 1;
}


int Fade::finish_hold()
{
	cpointer().get_viewport()->reset_context();
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


static float round_float( float f)
{
	return float(int(0.5 + f / RASTER_SIZE)) * RASTER_SIZE;
}

/********** FadeBend **********/
/******************************/

int FadeBend::begin_hold(int useX, int useY)
{
	PENTER;
	origY = cpointer().y();
	oldValue =  m_fade->get_bend_factor();
	return 1;
}


int FadeBend::jog()
{
	int direction = (m_fade->get_fade_type() == FadeCurve::FadeIn) ? 1 : -1;

	if (m_fade->get_raster()) {
		float value = round_float(oldValue + ((origY - cpointer().y()) / CURSOR_SPEED) * direction);
		m_fade->set_bend_factor(value);
	} else {
		m_fade->set_bend_factor(oldValue + (float(origY - cpointer().y()) / CURSOR_SPEED) * direction);
	}

	return 1;
}

/********** FadeStrength **********/
/******************************/

int FadeStrength::begin_hold(int useX, int useY)
{
	PENTER;
	origY = cpointer().y();
	oldValue =  m_fade->get_strenght_factor();
	return 1;
}


int FadeStrength::jog()
{
	if (m_fade->get_bend_factor() >= 0.5) {
		m_fade->set_strength_factor(oldValue + float(origY - cpointer().y()) / CURSOR_SPEED);
	} else {
		if (m_fade->get_raster()) {
			float value = round_float(oldValue + (origY - cpointer().y()) / CURSOR_SPEED);
			m_fade->set_strength_factor(value);
		} else {
			m_fade->set_strength_factor(oldValue - float(origY - cpointer().y()) / CURSOR_SPEED);
		}
	}
	

	return 1;
}


// eof

