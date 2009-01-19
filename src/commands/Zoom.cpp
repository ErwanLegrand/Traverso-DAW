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
 
*/

#include "Zoom.h"

#include "SheetView.h"
#include "Sheet.h"
#include "ClipsViewPort.h"
#include "ContextPointer.h"
#include <QPoint>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Zoom::Zoom(SheetView* sv, QVariantList args)
	: Command("Zoom")
{
	m_jogHorizontal = m_jogVertical = false;
	
	if (args.size() > 0) {
		QString type = args.at(0).toString();
		if (type == "JogZoom") {
			m_jogHorizontal = m_jogVertical = true;
		} else if (type == "HJogZoom") {
			m_jogHorizontal = true;
		} else if (type == "VJogZoom") {
			m_jogVertical = true;
		}
	}
	if (args.size() > 1) {
		m_xScalefactor = args.at(1).toDouble();
	} else {
		m_xScalefactor = 1;
	}
	if (args.size() > 2) {
		m_yScalefactor = args.at(2).toDouble();
	} else {
		m_yScalefactor = 0;
	}
	
        m_sv = sv;
}


int Zoom::prepare_actions()
{
	return 1;
}


int Zoom::begin_hold()
{
	verticalJogZoomLastY = cpointer().y();
	horizontalJogZoomLastX = cpointer().x();
	origPos = cpointer().pos();
	
	return 1;
}


int Zoom::finish_hold()
{
	QCursor::setPos(mousePos);
	return -1;
}


void Zoom::set_cursor_shape( int useX, int useY )
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	if (useX && useY) {
		cpointer().get_viewport()->set_holdcursor(":/cursorZoom");
	} else if(useX) {
		cpointer().get_viewport()->set_holdcursor(":/cursorZoomHorizontal");
	} else if (useY) {
		cpointer().get_viewport()->set_holdcursor(":/cursorZoomVertical");
	}
	
	mousePos = QCursor::pos();	
}

int Zoom::jog()
{
        PENTER;
	
	if (m_jogVertical) {
		int y = cpointer().y();
		int dy = y - verticalJogZoomLastY;
		
		if (abs(dy) > 8) {
			verticalJogZoomLastY = y;
			if (dy > 0) {
				m_sv->vzoom(1 + m_yScalefactor);
			} else {
				m_sv->vzoom(1 - m_yScalefactor);
			}
		}
	} 
	
	if (m_jogHorizontal) {
		int x = cpointer().x();
		int dx = x - horizontalJogZoomLastX;
		
		// TODO
		// values between /* */ are for use when using non power of 2 zoom levels!
		
		if (abs(dx) > 10  /*1*/) {
			horizontalJogZoomLastX = x;
			Sheet* sheet = m_sv->get_sheet();
			if (dx > 0) {
				sheet->set_hzoom(sheet->get_hzoom() / 2 /*(m_xScalefactor + dx/18)*/);
			} else {
				sheet->set_hzoom(sheet->get_hzoom() * 2 /*(m_xScalefactor + abs(dx)/18)*/);
			}
			m_sv->center();
		}
	}
	
	cpointer().get_viewport()->set_holdcursor_pos(m_sv->get_clips_viewport()->mapToScene(origPos).toPoint());
	
        return 1;
}

int Zoom::do_action( )
{
	if (m_yScalefactor != 0) {
		m_sv->vzoom(1 + m_yScalefactor);
	}
	if (m_xScalefactor != 1) {
		m_sv->hzoom(m_xScalefactor);
// 		m_sv->center();
	}
	
	return -1;
}

int Zoom::undo_action( )
{
	return -1;
}

void Zoom::vzoom_in(bool autorepeat)
{
	m_sv->vzoom(1.3);
}

void Zoom::vzoom_out(bool autorepeat)
{
	m_sv->vzoom(0.7);
}

void Zoom::toggle_vertical_horizontal_jog_zoom(bool autorepeat)
{
	if (autorepeat) return;
	
	if (m_jogVertical) {
		cpointer().get_viewport()->set_holdcursor(":/cursorZoomHorizontal");
		cpointer().get_viewport()->set_holdcursor_pos(m_sv->get_clips_viewport()->mapToScene(origPos).toPoint());
		m_jogVertical = false;
		m_jogHorizontal = true;
	} else {
		cpointer().get_viewport()->set_holdcursor(":/cursorZoomVertical");
		cpointer().get_viewport()->set_holdcursor_pos(m_sv->get_clips_viewport()->mapToScene(origPos).toPoint());
		m_jogVertical = true;
		m_jogHorizontal = false;
	}
}

