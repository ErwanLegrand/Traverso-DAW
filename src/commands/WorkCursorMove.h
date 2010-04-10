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
#include <defines.h>

class Sheet;
class SheetView;
class PlayHead;
class WorkCursor;

class WorkCursorMove : public Command
{
        Q_OBJECT
        Q_CLASSINFO("move_left", tr("Move Left"));
        Q_CLASSINFO("move_right", tr("Move Right"));
        Q_CLASSINFO("next_snap_pos", tr("To next snap position"));
        Q_CLASSINFO("prev_snap_pos", tr("To previous snap position"));
        Q_CLASSINFO("move_faster", tr("Move Faster"));
        Q_CLASSINFO("move_slower", tr("Move Slower"));

public :
	WorkCursorMove (WorkCursor* wc, PlayHead* cursor, SheetView* sv);
	~WorkCursorMove (){};

	int finish_hold();
	int begin_hold();
	void cancel_action();
	int jog();

	void set_cursor_shape(int useX, int useY);

private :
	Sheet*		m_sheet;
	SheetView*	m_sv;
	PlayHead*	m_playCursor;
        WorkCursor*     m_workCursor;
	TimeRef		m_origPos;
        int             m_holdCursorSceneY;
        int             m_speed;

        void do_keyboard_move(TimeRef newLocation, bool centerInView = false);

public slots:
        void move_left(bool autorepeat);
        void move_right(bool autorepeat);
        void move_faster(bool autorepeat);
        void move_slower(bool autorepeat);
        void next_snap_pos(bool autorepeat);
        void prev_snap_pos(bool autorepeat);

};

#endif
