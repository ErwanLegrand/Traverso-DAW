/*
Copyright (C) 2005-2010 Remon Sijrier

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

#include "Track.h"

#include "Sheet.h"

#include "Debugger.h"

Track::Track(Sheet *sheet)
        : ProcessingData(sheet)
{

}

Command* Track::solo(  )
{
        m_sheet->solo_processing_data(this);
        return (Command*) 0;
}



bool Track::is_solo()
{
        return m_isSolo;
}

bool Track::is_muted_by_solo()
{
        return mutedBySolo;
}


void Track::set_height(int h)
{
        m_height = h;
        emit heightChanged();
}


void Track::set_muted_by_solo(bool muted)
{
        PENTER;
        mutedBySolo = muted;
        emit audibleStateChanged();
}

void Track::set_solo(bool solo)
{
        m_isSolo = solo;
        if (solo)
                mutedBySolo = false;
        emit soloChanged(m_isSolo);
        emit audibleStateChanged();
}

void Track::set_sort_index( int index )
{
        m_sortIndex = index;
}

int Track::get_sort_index( ) const
{
        return m_sortIndex;
}
