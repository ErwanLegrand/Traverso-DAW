/*
Copyright (C) 2006-2007 Remon Sijrier 

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

*/


#include "Fade.h"

#include "Curve.h"
#include "AudioClip.h"
#include "ContextPointer.h"
#include <ViewPort.h>
#include <FadeCurve.h>
#include <FadeView.h>
#include <Peak.h>
#include <Song.h>
		
// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const float CURSOR_SPEED		= 150.0;
static const float RASTER_SIZE		= 0.05;


FadeRange::FadeRange(AudioClip* clip, FadeCurve* curve, int direction)
	: Command(clip, "")
{
	m_curve = curve;
	m_direction = direction;
	setText( (direction == 1) ? tr("Fade In: range") : tr("Fade Out: range"));
}


FadeRange::~FadeRange()
{}

int FadeRange::prepare_actions()
{
	return 1;
}

int FadeRange::begin_hold()
{
	origX = cpointer().on_first_input_event_x();
	newFade = origFade = m_curve->get_range();
	return 1;
}


int FadeRange::finish_hold()
{
	return 1;
}


int FadeRange::do_action()
{
	m_curve->set_range( newFade );
	return 1;
}


int FadeRange::undo_action()
{
	m_curve->set_range( origFade );
	return 1;
}


void FadeRange::cancel_action()
{
	finish_hold();
	undo_action();
}


int FadeRange::jog()
{
	int dx = (origX - (cpointer().x()) ) * m_direction;
	newFade = origFade - ( dx * Peak::zoomStep[m_curve->get_song()->get_hzoom() ]);
	
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

FadeBend::FadeBend(FadeView * fadeview)
	: Command("FadeBend")
	, m_fade(fadeview->get_fade())
	, m_fv(fadeview) 
{
}

int FadeBend::begin_hold()
{
	PENTER;
	origY = cpointer().on_first_input_event_y();
	oldValue =  m_fade->get_bend_factor();
	m_fv->set_holding(true);
	return 1;
}

int FadeBend::finish_hold()
{
	QCursor::setPos(mousePos);
	m_fv->set_holding(false);
	return 1;
}

void FadeBend::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	mousePos = QCursor::pos();
	cpointer().get_viewport()->set_holdcursor(":/cursorHoldUd");
}

int FadeBend::jog()
{
	int direction = (m_fade->get_fade_type() == FadeCurve::FadeIn) ? 1 : -1;
	
	float dx = (float(origY - cpointer().y()) / CURSOR_SPEED);

	if (m_fade->get_raster()) {
		float value = round_float(oldValue + dx * direction);
		m_fade->set_bend_factor(value);
	} else {
		m_fade->set_bend_factor(oldValue + dx * direction);
	}

	oldValue = m_fade->get_bend_factor();
	
	origY = cpointer().y();
	
	return 1;
}

/********** FadeStrength **********/
/******************************/

FadeStrength::FadeStrength(FadeView* fadeview)
	: Command("FadeStrength")
	, m_fade(fadeview->get_fade())
	, m_fv(fadeview)
{
}

int FadeStrength::begin_hold()
{
	PENTER;
	origY = cpointer().on_first_input_event_y();
	oldValue =  m_fade->get_strenght_factor();
	m_fv->set_holding(true);
	return 1;
}


int FadeStrength::finish_hold()
{
	QCursor::setPos(mousePos);
	m_fv->set_holding(false);
	return 1;
}


void FadeStrength::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	mousePos = QCursor::pos();	
	cpointer().get_viewport()->set_holdcursor(":/cursorHoldUd");
}

int FadeStrength::jog()
{
	float dy = float(origY - cpointer().y()) / CURSOR_SPEED;
	
	if (m_fade->get_bend_factor() >= 0.5) {
		m_fade->set_strength_factor(oldValue + dy );
	} else {
		if (m_fade->get_raster()) {
			float value = round_float(oldValue + dy);
			m_fade->set_strength_factor(value);
		} else {
			m_fade->set_strength_factor(oldValue - dy);
		}
	}
	
	oldValue = m_fade->get_strenght_factor();
	
	origY = cpointer().y();

	return 1;
}



// eof
