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

$Id: FadeView.h,v 1.1 2006/08/03 14:33:02 r_sijrier Exp $
*/

#ifndef FADE_VIEW_H
#define FADE_VIEW_H

#include "ViewItem.h"
#include <QPainterPath>

class FadeCurve;
class FadeContextDialog;
class AudioClip;
class AudioClipView;

class FadeView : public ViewItem
{
	Q_OBJECT
public:
	FadeView(ViewPort* vp, AudioClipView* parent, FadeCurve* fadeCuve);
	~FadeView();
	
	QRect draw(QPainter& painter);
        
        bool is_pointed() const;

private:
	AudioClipView*		m_clipView;
	FadeCurve*		m_fadeCurve;
	AudioClip*		m_clip;
	FadeContextDialog* 	m_dialog;
	QPainterPath 		m_path;
	
	void draw_fade_in(QPainter& painter);
	void draw_fade_out(QPainter& painter);
	
public slots:
	Command* edit_properties();

};

#endif

//eof


