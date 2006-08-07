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

$Id: FadeContextDialog.h,v 1.2 2006/08/07 19:15:23 r_sijrier Exp $
*/

#ifndef FADE_CONTEXT_DIALOG_H
#define FADE_CONTEXT_DIALOG_H

#include "ContextDialog.h" 

#include <QLabel>

class FadeContextDialogView;
class FadeCurve;

class FadeContextDialog : public ContextDialog
{
	Q_OBJECT
	
public:
	FadeContextDialog(FadeCurve* fadeCurve);
	~FadeContextDialog();
	
private:
	FadeContextDialogView*	m_fadeCDV;
	FadeCurve*		m_fade;
	
	QLabel*		m_bendLabel;
	QLabel*		m_strengthLabel;
	QLabel*		m_modeLabel;
	QLabel*		m_rasterLabel;
	
public slots:
	void update_bend_value();
	void update_strength_value();
	void update_raster_value();
	void update_mode_value();
};

#endif

//eof
