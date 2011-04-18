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
#include <FadeCurveView.h>
#include <Peak.h>
#include <Sheet.h>
#include "SnapList.h"
#include "Project.h"
#include "ProjectManager.h"
#include "TInputEventDispatcher.h"
		
// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

static const float CURSOR_SPEED		= 150.0;
static const float RASTER_SIZE		= 0.05;


FadeRange::FadeRange(AudioClip* clip, FadeCurve* curve, qint64 scalefactor)
        : MoveCommand(clip, "")
	, d(new Private())
{
	m_curve = curve;
	d->direction = (m_curve->get_fade_type() == FadeCurve::FadeIn) ? 1 : -1;
	d->scalefactor = scalefactor;
        d->clip = clip;
        d->sheet = clip->get_sheet();
	setText( (d->direction == 1) ? tr("Fade In: length") : tr("Fade Out: length"));
}


FadeRange::FadeRange(AudioClip* clip, FadeCurve* curve, double newRange)
        : MoveCommand(clip, "")
	, d(new Private())
{
	m_curve = curve;
	d->direction = (m_curve->get_fade_type() == FadeCurve::FadeIn) ? 1 : -1;
        d->clip = clip;
        d->sheet = clip->get_sheet();
        m_origRange = m_curve->get_range();
	m_newRange = newRange;
	setText( (d->direction == 1) ? tr("Fade In: remove") : tr("Fade Out: remove"));
}


FadeRange::~FadeRange()
{}

int FadeRange::prepare_actions()
{
	return 1;
}

int FadeRange::begin_hold()
{
	d->origX = cpointer().on_first_input_event_x();
	m_newRange = m_origRange = m_curve->get_range();
	return 1;
}


int FadeRange::finish_hold()
{
	delete d;
	return 1;
}


int FadeRange::do_action()
{
	m_curve->set_range( m_newRange );
	return 1;
}


int FadeRange::undo_action()
{
	m_curve->set_range( m_origRange );
	return 1;
}


void FadeRange::cancel_action()
{
	finish_hold();
	undo_action();
}

void FadeRange::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	d->mousePos = QCursor::pos();
	
	cpointer().setCursor(":/cursorHoldLr");
}


int FadeRange::jog()
{
	int dx = (d->origX - (cpointer().x()) ) * d->direction;
	m_newRange = m_origRange - ( dx * d->scalefactor);
	
	if (m_newRange < 1) {
		m_newRange = 1;
	}
	
	m_curve->set_range( m_newRange );
	
	TimeRef location = TimeRef(m_newRange);
	cpointer().setCursorText(timeref_to_ms_3(location));
	
	return 1;
}


void FadeRange::move_left(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        if (m_doSnap) {
                return prev_snap_pos(autorepeat);
        }
        do_keyboard_move(m_newRange - (d->scalefactor * m_speed * d->direction));
}


void FadeRange::move_right(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        if (m_doSnap) {
                return next_snap_pos(autorepeat);
        }

        do_keyboard_move(m_newRange + (d->scalefactor * m_speed * d->direction));
}


void FadeRange::next_snap_pos(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        TimeRef snap = d->sheet->get_snap_list()->next_snap_pos(d->clip->get_track_start_location() + m_newRange);
        TimeRef newpos = snap - d->clip->get_track_start_location();
        do_keyboard_move(newpos.universal_frame());
}

void FadeRange::prev_snap_pos(bool autorepeat)
{
        Q_UNUSED(autorepeat);
        TimeRef snap = d->sheet->get_snap_list()->prev_snap_pos(d->clip->get_track_start_location() + m_newRange);
        TimeRef newpos = snap - d->clip->get_track_start_location();
        do_keyboard_move(newpos.universal_frame());
}

void FadeRange::do_keyboard_move(double range)
{
        ied().bypass_jog_until_mouse_movements_exceeded_manhattenlength();
        m_newRange = range;

	TimeRef location = TimeRef(m_newRange);
	cpointer().setCursorText(timeref_to_ms_3(location));

        do_action();
}


static float round_float( float f)
{
	return float(int(0.5 + f / RASTER_SIZE)) * RASTER_SIZE;
}

/********** FadeBend **********/
/******************************/

FadeBend::FadeBend(FadeCurveView * FadeCurveView)
	: TCommand(FadeCurveView->get_fade())
	, m_fade(FadeCurveView->get_fade())
	, m_fv(FadeCurveView) 
{
	setText( (m_fade->get_fade_type() == FadeCurve::FadeIn) ? tr("Fade In: bend") : tr("Fade Out: bend"));
}

FadeBend::FadeBend(FadeCurve *fade, double val)
	: TCommand(fade)
	, m_fade(fade)
	, m_fv(0)
{
	setText( (m_fade->get_fade_type() == FadeCurve::FadeIn) ? tr("Fade In: bend") : tr("Fade Out: bend"));
	origBend = m_fade->get_bend_factor();
	newBend = val;
}

int FadeBend::begin_hold()
{
	PENTER;
	origY = cpointer().on_first_input_event_y();
	oldValue = m_fade->get_bend_factor();
	newBend = origBend = oldValue;
	m_fv->set_holding(true);
	return 1;
}

int FadeBend::finish_hold()
{
	QCursor::setPos(mousePos);
	m_fv->set_holding(false);
	return 1;
}

int FadeBend::prepare_actions()
{
	return 1;
}

int FadeBend::do_action()
{
	m_fade->set_bend_factor(newBend);
	return 1;
}

int FadeBend::undo_action()
{
	m_fade->set_bend_factor(origBend);
	return 1;
}

void FadeBend::cancel_action()
{
	finish_hold();
	undo_action();
}

void FadeBend::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	mousePos = QCursor::pos();
	cpointer().setCursor(":/cursorHoldUd");
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
	newBend = oldValue;
	cpointer().setCursorText(QByteArray::number(newBend, 'f', 2));
	
	origY = cpointer().y();
	
	return 1;
}

/********** FadeStrength **********/
/******************************/

FadeStrength::FadeStrength(FadeCurveView* FadeCurveView)
	: TCommand(FadeCurveView->get_fade())
	, m_fade(FadeCurveView->get_fade())
	, m_fv(FadeCurveView)
{
	setText( (m_fade->get_fade_type() == FadeCurve::FadeIn) ? tr("Fade In: strength") : tr("Fade Out: strength"));
}

FadeStrength::FadeStrength(FadeCurve *fade, double val)
	: TCommand(fade)
	, m_fade(fade)
	, m_fv(0)
{
	setText( (m_fade->get_fade_type() == FadeCurve::FadeIn) ? tr("Fade In: strength") : tr("Fade Out: strength"));
	origStrength = m_fade->get_strength_factor();
	newStrength = val;
}

int FadeStrength::begin_hold()
{
	PENTER;
	origY = cpointer().on_first_input_event_y();
	oldValue = m_fade->get_strength_factor();
	newStrength = origStrength = oldValue;
	m_fv->set_holding(true);
	return 1;
}

int FadeStrength::finish_hold()
{
	QCursor::setPos(mousePos);
	m_fv->set_holding(false);
	return 1;
}

int FadeStrength::prepare_actions()
{
	return 1;
}

int FadeStrength::do_action()
{
	m_fade->set_strength_factor(newStrength);
	return 1;
}

int FadeStrength::undo_action()
{
	m_fade->set_strength_factor(origStrength);
	return 1;
}

void FadeStrength::cancel_action()
{
	finish_hold();
	undo_action();
}

void FadeStrength::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	mousePos = QCursor::pos();	
	cpointer().setCursor(":/cursorHoldUd");
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
	
	oldValue = m_fade->get_strength_factor();
	newStrength = oldValue;
	cpointer().setCursorText(QByteArray::number(newStrength, 'f', 2));

	origY = cpointer().y();

	return 1;
}


/********** FadeMode **********/
/******************************/

FadeMode::FadeMode(FadeCurve* fade, int oldMode, int newMode)
	: TCommand(fade)
	, m_fade(fade)
{
	setText( (m_fade->get_fade_type() == FadeCurve::FadeIn) ? tr("Fade In: shape") : tr("Fade Out: shape"));

	m_newMode = newMode;
	m_oldMode = oldMode;
}

int FadeMode::prepare_actions()
{
	return 1;
}

int FadeMode::do_action()
{
	m_fade->set_mode(m_newMode);
	return 1;
}

int FadeMode::undo_action()
{
	m_fade->set_mode(m_oldMode);
	return 1;
}

