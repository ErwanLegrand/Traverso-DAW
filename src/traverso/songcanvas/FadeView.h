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

$Id: FadeView.h,v 1.5 2007/06/27 19:32:43 benjie Exp $
*/

#ifndef FADE_VIEW_H
#define FADE_VIEW_H

#include "ViewItem.h"

class Curve;
class FadeCurve;
class FadeContextDialog;
class AudioClipView;

class FadeView : public ViewItem
{
	Q_OBJECT
	
	Q_CLASSINFO("bend", tr("Bend"))
	Q_CLASSINFO("strength", tr("Strength"))
	
public:
	FadeView(SongView* sv, AudioClipView* parent, FadeCurve* fadeCuve);
	~FadeView();
	
	enum {Type = UserType + 4};
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	int get_vector(int xstart, int pixelcount, float * arg);
	void calculate_bounding_rect();
	void set_holding(bool hold);
	
	FadeCurve* get_fade() const {return m_fadeCurve;}
	
	int type() const;
	void load_theme_data();

private:
	FadeCurve*	m_fadeCurve;
	Curve*		m_guicurve;
	bool		m_holdactive;
	
public slots:
	void state_changed();
	
	Command* bend();
	Command* strength();
// 	Command* edit_properties();

signals :
	// emit from the gui so that we can stop following the playhead only
	// when the user manually edits, not on undo/redo
	void fadeModified();
};

inline int FadeView::type() const {return Type;}

#endif

//eof


