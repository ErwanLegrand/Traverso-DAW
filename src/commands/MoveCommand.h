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

#ifndef MOVE_COMMAND_H
#define MOVE_COMMAND_H

#include "TCommand.h"
#include <defines.h>

class MoveCommand : public TCommand
{
        Q_OBJECT

public :
        MoveCommand (const QString& description);
        MoveCommand (ContextItem* item, const QString& description);
        virtual ~MoveCommand (){}

        void set_collected_number(const QString & collected);

protected :
        int             m_speed;
        bool            m_doSnap;

public slots:
        void move_faster(bool autorepeat);
        void move_slower(bool autorepeat);
        void toggle_snap_on_off(bool autorepeat);

};

#endif // MOVECOMMAND_H
