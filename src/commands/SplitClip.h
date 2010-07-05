/*
    Copyright (C) 2005-2007 Remon Sijrier 
 
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

#ifndef SPLITCLIPACTION_H
#define SPLITCLIPACTION_H

#include "MoveCommand.h"
#include "defines.h"

class AudioClip;
class AudioTrack;
class SheetView;
class TSession;
class AudioClipView;
class LineView;

class SplitClip : public MoveCommand
{
        Q_OBJECT
public :
	SplitClip(AudioClipView* view);
        ~SplitClip() {}

        int prepare_actions();
        int do_action();
        int undo_action();

	int begin_hold();
	int finish_hold();
	void cancel_action();
	void set_cursor_shape(int useX, int useY);

	int jog();
	
private :
	SheetView* m_sv;
        TSession*  m_session;
	AudioClipView* m_cv;
        AudioTrack* m_track;
        AudioClip* m_clip;
        AudioClip* leftClip;
        AudioClip* rightClip;
	TimeRef m_splitPoint;
	LineView* m_splitcursor;

        void do_keyboard_move(TimeRef location);

public slots:
        void move_left(bool autorepeat);
        void move_right(bool autorepeat);
        void next_snap_pos(bool autorepeat);
        void prev_snap_pos(bool autorepeat);

};

#endif
