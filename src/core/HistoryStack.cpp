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
 
    $Id: HistoryStack.cpp,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#include "HistoryStack.h"
#include "Command.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

HistoryStack::HistoryStack()
{
        moreRedoActionsAvailable = false;
        currentAction = (Command*) 0;
        firstAction = (Command*) 0;
}


HistoryStack::~HistoryStack()
{
        if (firstAction) {
                currentAction = firstAction;
                delete_from_current();
        }
        delete firstAction;
}


Command* HistoryStack::undo()
{
        Command* a = (Command*) 0;
        if (currentAction) {
                a = currentAction;
                currentAction = currentAction->prev;
                moreRedoActionsAvailable = true;
        }
        return a;
}


Command* HistoryStack::redo()
{
        Command* a = (Command*) 0;
        if(currentAction && currentAction->next) {
                a = currentAction->next;
                currentAction = a;
        }
        if ((!currentAction) && moreRedoActionsAvailable) {
                currentAction = firstAction;
                a = currentAction;
        }
        return a;
}


int HistoryStack::push(Command* a)
{
        Command* al;
        moreRedoActionsAvailable = false;

        cleanup_actionlist();

        al = currentAction;
        if (firstAction) {
                al->next = a;
                a->prev = al;
                a->next = 0;
                currentAction = a;
        } else {
                currentAction = a;
                currentAction->next = 0;
                currentAction->prev = 0;
                firstAction = currentAction;
        }
        return 1;
}


Command* HistoryStack::current()
{
        return currentAction;
}


int HistoryStack::cleanup_actionlist()
{

        if (currentAction && currentAction->next) {
                delete_from_current();
        } else if (!currentAction && firstAction) {
                currentAction = firstAction;
                delete_from_current();
                delete currentAction;
                currentAction = 0;
                firstAction = 0;
        }
        return 1;
}


int HistoryStack::delete_from_current()
{
        Command* al;
        while (currentAction->next) {
                al = currentAction->next;
                currentAction->next = al->next;
                delete al;
        }
        return 1;
}
//eof
