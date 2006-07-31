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

$Id: SnapList.h,v 1.2 2006/07/31 14:48:38 r_sijrier Exp $
*/

#ifndef SNAPLIST_H
#define SNAPLIST_H

#include <QList>

#include "defines.h"

class Song;
class AudioClip;

class SnapList
{

public:
	SnapList(Song* song);
	~SnapList();

	int get_snap_value(int);
	bool is_snap_value(int);
	void update_snaplist(AudioClip *);
	void update_snaplist();
	int get_snap_diff(int);

private:
	Song* m_song;
	QList<int> xposList;
	QList<int> xposLut;
	QList<bool> xposBool;
	QList<AudioClip* > m_clips;

	void process_snaplist();
};


#endif

/* EOF */
