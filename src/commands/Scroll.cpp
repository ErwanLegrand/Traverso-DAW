/*
    Copyright (C) 2008 Remon Sijrier
    Copyright (C) 2007 Ben Levitt 
 
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

#include "Scroll.h"

#include "SheetView.h"
#include "ContextPointer.h"
#include "ClipsViewPort.h"
#include "QScrollBar"

Scroll::Scroll(SheetView* sv, QVariantList args)
	: MoveCommand("Scroll")
{
	m_sv = sv;
	m_dx = m_dy = 0;
	
	if (args.size() > 0) {
		m_dx = args.at(0).toInt();
	}
	if (args.size() > 1) {
		m_dy = args.at(1).toInt();
	}
}


int Scroll::prepare_actions()
{
	return -1;
}


int Scroll::begin_hold()
{
        m_sv->start_shuttle(true, true);
	m_sv->set_shuttle_factor_values(m_dx, m_dy);
	if (m_dx) {
                cpointer().get_viewport()->set_cursor_shape("LR");
	} else {
                cpointer().get_viewport()->set_cursor_shape("UD");
	}
	
	return 1;
}


int Scroll::finish_hold()
{
	m_sv->start_shuttle(false);

	return 1;
}

int Scroll::do_action( )
{
	return -1;
}

int Scroll::undo_action( )
{
	return -1;
}

void Scroll::move_up(bool )
{
	int step = m_sv->getVScrollBar()->pageStep();
	m_sv->set_vscrollbar_value(m_sv->vscrollbar_value() - step * m_speed);
}

void Scroll::move_down(bool )
{
	int step = m_sv->getVScrollBar()->pageStep();
	m_sv->set_vscrollbar_value(m_sv->vscrollbar_value() + step * m_speed);
}

void Scroll::move_left(bool )
{
	m_sv->set_hscrollbar_value(m_sv->hscrollbar_value() - (m_speed * 5));
}

void Scroll::move_right(bool )
{
	m_sv->set_hscrollbar_value(m_sv->hscrollbar_value() + (m_speed * 5));
}


// eof
