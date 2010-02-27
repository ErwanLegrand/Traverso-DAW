/*
    Copyright (C) 2010 Remon Sijrier

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

#ifndef MOVETRACK_H
#define MOVETRACK_H


#include "Command.h"

class SheetView;
class Track;
class TrackView;

class MoveTrack : public Command
{
        Q_OBJECT
        Q_CLASSINFO("move_up", tr("Move Up"));
        Q_CLASSINFO("move_down", tr("Move Down"));

public :
        MoveTrack(TrackView* view);
        ~MoveTrack();

        int begin_hold();
        int finish_hold();
        int prepare_actions();
        int do_action();
        int undo_action();
        void cancel_action();
        int jog();

        void set_cursor_shape(int useX, int useY);

private:
        SheetView*      m_sv;
        TrackView*      m_trackView;

public slots:
        void move_up(bool autorepeat);;
        void move_down(bool autorepeat);
};



#endif // MOVETRACK_H
