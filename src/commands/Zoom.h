/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: Zoom.h,v 1.6 2008/01/21 16:22:12 r_sijrier Exp $
*/

#ifndef ZOOM_H
#define ZOOM_H

#include "Command.h"

#include <QTimer>

class SheetView;
class QPoint;

class Zoom : public Command
{
	Q_OBJECT
	Q_CLASSINFO("vzoom_in", tr("Zoom Vertical In"));
	Q_CLASSINFO("vzoom_out", tr("Zoom Vertical Out"));
	Q_CLASSINFO("toggle_vertical_horizontal_jog_zoom", tr("Toggle Vertical / Horizontal"));

public :
	Zoom(SheetView* sv, QVariantList args);
        ~Zoom() {};

        int begin_hold();
        int finish_hold();
        int prepare_actions();
	int do_action();
	int undo_action();

        int jog();

        void set_cursor_shape(int useX, int useY);

private :
	int horizontalJogZoomLastX;
        int verticalJogZoomLastY;
	bool m_jogVertical;
	bool m_jogHorizontal;
	qreal m_xScalefactor;
	qreal m_yScalefactor;
	QPoint	mousePos;
	QPoint	origPos;
	QTimer	m_jogVerticalResetTimer;
	
        SheetView* m_sv;
	
public slots:
	void vzoom_in(bool autorepeat);
	void vzoom_out(bool autorepeat);
	void toggle_vertical_horizontal_jog_zoom(bool autorepeat);
};

#endif

