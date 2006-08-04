/*
Copyright (C) 2006 Remon Sijrier, Nicola Doebelin

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

$Id: FadeContextDialogView.h,v 1.3 2006/08/04 11:22:41 r_sijrier Exp $
*/

#ifndef FADE_CONTEXT_DIALOG_VIEW_H
#define FADE_CONTEXT_DIALOG_VIEW_H

#include "ViewItem.h"

#include <QPixmap>

class ViewPort;
class Command;
class FadeCurve;

class FadeContextDialogView : public ViewItem
{
	Q_OBJECT
	
public:
	FadeContextDialogView(ViewPort* viewPort, FadeCurve* fadeCurve);
	~FadeContextDialogView();

        QRect draw(QPainter& p);

	float roundFloat(float f);
	bool get_raster() const {return m_raster;}


private:
	void create_background();
        void schedule_for_repaint();
	
	bool 	m_raster;
	
	QPixmap		background;
	ViewPort*	m_vp;
	FadeCurve*	m_fade;

private slots:
        void resize();
        void state_changed();
	
public slots:
	Command* bend();
	Command* strength();
	Command* toggle_raster();

};


class FadeStrength : public Command
{
public :
        FadeStrength(FadeContextDialogView* view, FadeCurve* curve) : Command(), m_view(view), m_curve(curve) {};
        ~FadeStrength(){};

        int begin_hold();
        int jog();

private :
	float	oldValue;
	int	origY;
	FadeContextDialogView* m_view;
	FadeCurve*	m_curve;

};


class FadeBend : public Command
{
public :
        FadeBend(FadeContextDialogView* view, FadeCurve* curve) : Command(), m_view(view), m_curve(curve) {};
        ~FadeBend(){};

        int begin_hold();
        int jog();

private :
	float	oldValue;
	int	origY;
	FadeContextDialogView* m_view;
	FadeCurve*	m_curve;

};

#endif

//eof
 
