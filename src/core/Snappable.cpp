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

$Id: Snappable.cpp,v 1.1 2007/03/29 22:18:38 benjie Exp $
*/

#include "Snappable.h"
#include "SnapList.h"

#include <Debugger.h>


Snappable::Snappable()
{
	m_isSnappable = true;
	snapList = 0;
}

void Snappable::set_snappable(bool snap)
{
	// Temporarily set to true so the snapList will rebuild on mark_dirty()
	m_isSnappable = true;

	if (snapList) {
		snapList->mark_dirty(this);
	}
	m_isSnappable = snap;
}

void Snappable::set_snap_list(SnapList *sList)
{
	snapList = sList;
}

bool Snappable::is_snappable() const
{
	return m_isSnappable;
}

/* EOF */
