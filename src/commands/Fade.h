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

$Id: Fade.h,v 1.10 2007/06/26 20:08:51 r_sijrier Exp $
*/

#ifndef FADE_H
#define FADE_H

#include "Command.h"

class Curve;
class AudioClip;
class FadeCurve;
class FadeView;
class SongView;

class FadeRange : public Command
{
public :
        FadeRange(AudioClip* clip, FadeCurve* curve);
	FadeRange(AudioClip* clip, FadeCurve* curve, double newVal);
        ~FadeRange();

        int begin_hold();
        int finish_hold();
        int prepare_actions();
        int do_action();
        int undo_action();
	void cancel_action();

        int jog();
	
	void set_cursor_shape(int useX, int useY);

private :
        int origX;
        int m_direction;
        double origFade;
        double newFade;
	QPoint	m_mousePos;

	FadeCurve*	m_curve;
};


class FadeStrength : public Command
{
public :
	FadeStrength(FadeView* fadeview);
        ~FadeStrength(){};

        int begin_hold();
	int finish_hold();
        int prepare_actions();
        int do_action();
        int undo_action();
	void cancel_action();

	int jog();

	void set_cursor_shape(int useX, int useY);
	
private :
	float	oldValue;
	int	origY;
	double	origStrength;
	double	newStrength;
	FadeCurve*	m_fade;
	FadeView*	m_fv;
	QPoint		mousePos;
};


class FadeBend : public Command
{
public :
	FadeBend(FadeView* fadeview);
        ~FadeBend(){};

        int begin_hold();
	int finish_hold();
        int prepare_actions();
        int do_action();
        int undo_action();
	void cancel_action();

	int jog();
	
	void set_cursor_shape(int useX, int useY);

private :
	float	oldValue;
	int	origY;
	double	origBend;
	double	newBend;
	FadeCurve*	m_fade;
	FadeView*	m_fv;
	QPoint		mousePos;
};


#endif


