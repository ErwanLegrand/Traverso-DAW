/*
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

#include <libtraversocore.h>

#include "Scroll.h"

#include "SheetView.h"
#include <ClipsViewPort.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Scroll::Scroll(int x, int y, SheetView* sv)
	: Command("Scroll")
{
	m_dx = x;
	m_dy = y;
	m_sv = sv;
}


int Scroll::prepare_actions()
{
	return -1;
}


int Scroll::begin_hold()
{
	m_sv->start_shuttle(true);
	m_sv->set_shuttle_factor_values(m_dx, m_dy);
	
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


// eof
