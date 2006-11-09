/*
Copyright (C) 2006 Nicola Doebelin

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

$Id: SnapList.h,v 1.3 2006/11/09 15:45:42 r_sijrier Exp $
*/

#ifndef SNAPLIST_H
#define SNAPLIST_H

#include <QList>

#include "defines.h"

class Song;
class AudioClip;

class SnapList : public QObject
{
	Q_OBJECT

public:
	SnapList(Song* song);
	~SnapList() {};

	int get_snap_value(int);
	bool is_snap_value(int);
	int get_snap_diff(int);
	
	void set_range(int start, int end);

private:
	Song* m_song;
	QList<int> xposList;
	QList<int> xposLut;
	QList<bool> xposBool;
	QList<AudioClip* > m_clips;
	bool	m_isDirty;
	int	m_rangeStart;
	int	m_rangeEnd;

	void update_snaplist();

public slots:
	void mark_dirty();
};

inline void SnapList::set_range(int start, int end)
{
	printf("setting xstart %d, xend %d\n", start, end);
	m_rangeStart = start;
	m_rangeEnd = end;
}

#endif

/* EOF */
