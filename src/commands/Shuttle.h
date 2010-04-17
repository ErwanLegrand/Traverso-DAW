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


#ifndef SHUTTLE_H
#define SHUTTLE_H

#include "MoveCommand.h"

class SheetView;

class Shuttle : public MoveCommand
{
        Q_OBJECT
        Q_CLASSINFO("move_up", tr("Move Up"));
        Q_CLASSINFO("move_down", tr("Move Down"));
        Q_CLASSINFO("move_left", tr("Move Left"));
        Q_CLASSINFO("move_right", tr("Move Right"));

public :
        Shuttle(SheetView* sv);

        int begin_hold();
        int finish_hold();
        int jog();

private :
        SheetView*	m_sv;

public slots:
        void move_up(bool autorepeat);
        void move_down(bool autorepeat);
        void move_left(bool autorepeat);
        void move_right(bool autorepeat);

};


#endif // SHUTTLE_H
