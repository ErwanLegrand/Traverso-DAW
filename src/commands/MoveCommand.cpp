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

#include "MoveCommand.h"


MoveCommand::MoveCommand(const QString &description)
        : Command(description)
        , m_speed(1)
        , m_arrowKeysDoSnap(false)
{
}

MoveCommand::MoveCommand(ContextItem* item, const QString &description)
        : Command(item, description)
        , m_speed(1)
        , m_arrowKeysDoSnap(false)
{
}

void MoveCommand::move_faster(bool autorepeat)
{
        if (m_speed == 1) {
                m_speed = 2;
        } else if (m_speed == 2) {
                m_speed = 4;
        } else if (m_speed == 4) {
                m_speed = 8;
        } else if (m_speed == 8) {
                m_speed = 16;
        } else if (m_speed == 16) {
                m_speed = 32;
        }
}


void MoveCommand::move_slower(bool autorepeat)
{
        if (m_speed == 32) {
                m_speed = 16;
        } else if (m_speed == 16) {
                m_speed = 8;
        } else if (m_speed == 8) {
                m_speed = 4;
        } else if (m_speed == 4) {
                m_speed = 2;
        } else if (m_speed == 2) {
                m_speed = 1;
        }
}

void MoveCommand::toggle_snap_on_off(bool autorepeat)
{
        if (autorepeat) {
                return;
        }
        m_arrowKeysDoSnap = ! m_arrowKeysDoSnap;
}
