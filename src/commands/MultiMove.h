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
 
    $Id: MultiMove.h,v 1.1 2008/01/02 03:03:38 benjie Exp $
*/

#ifndef MULTIMOVE_H
#define MULTIMOVE_H

#include "Command.h"
#include "defines.h"
#include <QList>

class AudioClip;
class Marker;
class Song;
class Track;
class SongView;


class MultiMove : public Command
{
public :
        MultiMove(SongView* sv, bool allTracks);
        ~MultiMove();

        int begin_hold();
        int finish_hold();
	void cancel_action();
        int prepare_actions();
        int do_action();
        int undo_action();

        int jog();

private :
	QList<AudioClip* >	m_clips;
	QList<Marker* >		m_markers;
	SongView*		m_sv;
	Song*			m_song;
	Track*			m_track;
	bool			m_allTracks;
	TimeRef			m_originalPos;
	TimeRef			m_lastPos;
	TimeRef			m_newPos;
	TimeRef			m_selectionStartPos;	// Position of left-most "selected" item
							// Don't move more than this amount to the left
							// to avoid moving anything past the beginning of the file.
};

#endif

