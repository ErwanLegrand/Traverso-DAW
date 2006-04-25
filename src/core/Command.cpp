/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: Command.cpp,v 1.2 2006/04/25 17:24:57 r_sijrier Exp $
*/

#include "Command.h"
#include "IEMessage.h"
#include "HistoryStack.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Command::Command( )
{
        m_historyStack = 0;
        handleByIE=true;
}

Command::Command(ContextItem* item)
{
        handleByIE=true;
        m_context = item;
        m_historyStack = item->get_history_stack();
}

Command::~Command()
{}

int Command::begin_hold()
{
        return -1;
}

int Command::finish_hold()
{
        return -1;
}

int Command::jog()
{
        return -1;
}

bool Command::valid()
{
        return isValid;
}

void Command::set_valid(bool valid)
{
        isValid = valid;
}

int Command::push_to_history_stack( )
{
        PENTER;
        if (m_historyStack) {
                m_historyStack->push(this);
                return 0;
        } else {
                PMESG("No history stack??");
                return -1;
        }
}

//eof
