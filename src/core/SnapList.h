/*
Copyright (C) 2006-2008 Nicola Doebelin, Remon Sijrier

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

#ifndef SNAPLIST_H
#define SNAPLIST_H

#include <QList>

#include "defines.h"

class TSession;

class SnapList
{

public:
        SnapList(TSession* sheet);
        ~SnapList() {}

	TimeRef get_snap_value(const TimeRef& location);
	bool is_snap_value(const TimeRef& location);
	qint64 get_snap_diff(const TimeRef& location);
	TimeRef next_snap_pos(const TimeRef& location);
	TimeRef prev_snap_pos(const TimeRef& location);
	
	TimeRef calculate_snap_diff(TimeRef leftlocation, TimeRef rightlocation);

	void set_range(const TimeRef& start, const TimeRef& end, int scalefactor);
	void mark_dirty();
	bool was_dirty();

private:
        TSession*	m_sheet;
	QList<TimeRef> 	m_xposList;
	QList<TimeRef> 	m_xposLut;
	QList<bool> 	m_xposBool;
	bool		m_isDirty;
        bool		m_wasDirty;
	TimeRef		m_rangeStart;
	TimeRef		m_rangeEnd;
	qint64		m_scalefactor;

	void update_snaplist();
};

#endif

/* EOF */
