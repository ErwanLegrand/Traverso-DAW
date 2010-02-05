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

#include "Shuttle.h"

#include "SheetView.h"

Shuttle::Shuttle(SheetView* sv)
        : Command("Shuttle")
        , m_sv(sv)
{
}

int Shuttle::begin_hold()
{
        m_sv->update_shuttle_factor();
        m_sv->start_shuttle(true);
        return 1;
}

int Shuttle::finish_hold()
{
        m_sv->start_shuttle(false);
        return 1;
}

int Shuttle::jog()
{
        m_sv->update_shuttle_factor();
        return 1;
}

