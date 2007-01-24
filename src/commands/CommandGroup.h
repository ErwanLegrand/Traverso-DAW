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
 
    $Id: CommandGroup.h,v 1.3 2007/01/24 21:16:11 r_sijrier Exp $
*/

#ifndef COMMAND_GROUP_H
#define COMMAND_GROUP_H

#include "Command.h"

#include <QList>

class CommandGroup : public Command
{
public :
        CommandGroup(ContextItem* parent, const QString& des, bool historable=true)
        	: Command(parent, des) 
        {
        	m_isHistorable = historable;
        };
        ~CommandGroup() {};

        int prepare_actions();
        int do_action();
        int undo_action();

        void add_command(Command* cmd) {
        	Q_ASSERT(cmd);
		m_commands.append(cmd);
	}
;

private :
        QList<Command* >	m_commands;

};

#endif
 
 
 
