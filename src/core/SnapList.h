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

$Id: SnapList.h,v 1.8 2007/04/02 00:35:13 benjie Exp $
*/

#ifndef SNAPLIST_H
#define SNAPLIST_H

#include <QObject>
#include <QList>

#include "defines.h"

class Song;
class AudioClip;
class Snappable;

class SnapList : public QObject
{
	Q_OBJECT

public:
	SnapList(Song* song);
	~SnapList() {};

	int get_snap_value(nframes_t);
	bool is_snap_value(nframes_t);
	int get_snap_diff(nframes_t);
	
	void set_range(nframes_t start, nframes_t end, int scalefactor);

private:
	Song* 		m_song;
	QList<nframes_t> 	xposList;
	QList<int> 	xposLut;
	QList<bool> 	xposBool;
	bool		m_isDirty;
	nframes_t	m_rangeStart;
	nframes_t	m_rangeEnd;
	int		m_scalefactor;

	void update_snaplist();

public slots:
	void mark_dirty(Snappable *item);
};

inline void SnapList::set_range(nframes_t start, nframes_t end, int scalefactor)
{
// 	printf("setting xstart %d, xend %d scalefactor %d\n", start, end, scalefactor);
	m_rangeStart = start;
	m_rangeEnd = end;
	m_scalefactor = scalefactor;
	m_isDirty = true;
};


#endif

/* EOF */
