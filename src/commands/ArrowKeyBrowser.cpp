/*
Copyright (C) 2010 Remon Sijrier

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

#include "ArrowKeyBrowser.h"

#include "SheetView.h"

#include "Debugger.h"

ArrowKeyBrowser::ArrowKeyBrowser(SheetView *sv, QVariantList args)
{
        m_sv = sv;
}

int ArrowKeyBrowser::begin_hold()
{
        return 1;
}

int ArrowKeyBrowser::finish_hold()
{
        return -1;
}

void ArrowKeyBrowser::set_cursor_shape(int useX, int useY)
{
        if (useX) {
                cpointer().get_viewport()->set_holdcursor(":/cursorHoldLr");
        }
        if (useY) {
                cpointer().get_viewport()->set_holdcursor(":/cursorHoldUd");
        }
}

void ArrowKeyBrowser::up(bool autorepeat)
{
	m_sv->browse_to_context_item_above();
}

void ArrowKeyBrowser::down(bool autorepeat)
{
	m_sv->browse_to_context_item_below();
}

void ArrowKeyBrowser::left(bool autorepeat)
{
	m_sv->browse_to_previous_context_item();
}

void ArrowKeyBrowser::right(bool autorepeat)
{
	m_sv->browse_to_next_context_item();
}
