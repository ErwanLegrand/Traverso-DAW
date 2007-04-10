/*
    Copyright (C) 2007 Remon Sijrier 
 
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

#ifndef WORKCURSOR_MOVE_H
#define WORKCURSOR_MOVE_H

#include <Command.h>

class Song;
class SongView;
class PlayHead;

class WorkCursorMove : public Command
{
public :
	WorkCursorMove (PlayHead* cursor, SongView* sv);
	~WorkCursorMove (){};

	int finish_hold();
	int begin_hold();
	int jog();

	void set_cursor_shape(int useX, int useY);

private :
	Song*		m_song;
	SongView*	m_sv;
	PlayHead*	m_playCursor;
};

#endif
