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

$Id: Fade.h,v 1.13 2008/01/21 16:22:11 r_sijrier Exp $
*/

#ifndef FADE_H
#define FADE_H

#include "TCommand.h"
#include "MoveCommand.h"

class Curve;
class AudioClip;
class FadeCurve;
class FadeCurveView;
class SheetView;
class Sheet;

class FadeRange : public MoveCommand
{
        Q_OBJECT

public :
        FadeRange(AudioClip* clip, FadeCurve* curve, qint64 scalefactor);
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
	FadeCurve*	m_curve;
	double 		m_origRange;
	double 		m_newRange;
	class Private {
		public:
                        Sheet* sheet;
                        AudioClip* clip;
			int origX;
			int direction;
			QPoint	mousePos;
			qint64 scalefactor;
	};
	Private* d;

        void do_keyboard_move(double range);


public slots:
        void next_snap_pos(bool autorepeat);
        void prev_snap_pos(bool autorepeat);
        void move_left(bool autorepeat);
        void move_right(bool autorepeat);
};


class FadeStrength : public TCommand
{
        Q_OBJECT

public :
	FadeStrength(FadeCurveView* FadeCurveView);
	FadeStrength(FadeCurve* fade, double val);
        ~FadeStrength(){}

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
	FadeCurveView*	m_fv;
	QPoint		mousePos;
};


class FadeBend : public TCommand
{
        Q_OBJECT

public :
	FadeBend(FadeCurveView* FadeCurveView);
	FadeBend(FadeCurve* fade, double val);
        ~FadeBend(){}

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
	FadeCurveView*	m_fv;
	QPoint		mousePos;
};


class FadeMode : public TCommand
{
        Q_OBJECT

public :
	FadeMode(FadeCurve* fade, int oldMode, int newMode);
        ~FadeMode(){}

        int prepare_actions();
        int do_action();
        int undo_action();
	int begin_hold() {return 1;}
	int finish_hold() { return 1;}

private :
	int		m_oldMode;
	int		m_newMode;
	FadeCurve*	m_fade;
};


#endif


