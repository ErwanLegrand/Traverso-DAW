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

$Id: CommandGroup.cpp,v 1.6 2007/04/30 10:09:11 r_sijrier Exp $
*/

#include "CommandGroup.h"
#include <cstdio>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/** 	\class CommandGroup 
 *	\brief A class to return a group of Command objects as one history object to the historystack
 *	
 */


CommandGroup::~ CommandGroup()
{
	foreach(TCommand* cmd, m_commands) {
		delete cmd;
	}
}

int CommandGroup::prepare_actions()
{
	if (m_commands.size() == 0) {
		return -1;
	}
	
	int result = 1;
	
	foreach(TCommand* cmd, m_commands) {
		if (cmd->prepare_actions() == -1) {
			printf("one of the commands in the group failed prepare_actions\n");
			result = -1;
		}
	}
	
	return result;
}

int CommandGroup::do_action()
{
	foreach(TCommand* cmd, m_commands) {
		cmd->do_action();
	}
	
	return 1;
}

int CommandGroup::undo_action()
{
	foreach(TCommand* cmd, m_commands) {
		cmd->undo_action();
	}
	
	return 1;
}

// eof

