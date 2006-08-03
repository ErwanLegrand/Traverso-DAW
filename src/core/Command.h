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
 
    $Id: Command.h,v 1.2 2006/08/03 14:33:46 r_sijrier Exp $
*/

#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include "ContextItem.h"

class HistoryStack;
class IEMessage;

class Command : public QObject
{
public :
        Command(ContextItem* item);
        Command();
        virtual ~Command();

        virtual int begin_hold();
        virtual int finish_hold();
        virtual int prepare_actions();
        virtual int do_action();
        virtual int undo_action();
        virtual int jog();

        void set_valid(bool valid);
        int push_to_history_stack();

        bool valid();

        bool handleByIE;

        Command* prev;
        Command* next;


protected:
        bool isValid;

private:
        bool expired;
        HistoryStack* m_historyStack;
        ContextItem* m_context;


};


#endif


