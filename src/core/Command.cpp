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

$Id: Command.cpp,v 1.9 2007/01/16 14:06:05 r_sijrier Exp $
*/

#include "Command.h"
#include "ContextPointer.h"
#include <ViewPort.h>
#include <Utils.h>
#include "ContextItem.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Command::Command( const QString& des )
	: QUndoCommand(des)
{
	m_historyStack = 0;
	m_isHistorable = true;
}

Command::Command(ContextItem* item, const QString& des)
	: QUndoCommand(des)
{
	m_historyStack = item->get_history_stack();
	m_isHistorable = true;
}

Command::~Command()
{}

int Command::begin_hold(int useX, int useY)
{
	PERROR("Hold actions should re-implement this function!!");
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

void Command::set_valid(bool valid)
{
	PENTER;
	m_isValid = valid;
}

int Command::push_to_history_stack( )
{
	PENTER;
	
	if (! m_isHistorable) {
		PMESG("Not a Historable command, deleting the command");
		return -1;
	}
	
	if (! m_isValid) {
		PMESG("This command is invalid, deleting the command");
		return -1;
	}
	
	if (! m_historyStack) {
		PMESG("This command has no HistoryStack, deleting the command");
		return -1;
	}
		
	m_historyStack->push(this);
	
	return 1;
}

int Command::prepare_actions( )
{
	return -1;
}

int Command::do_action( )
{
	return -1;
}

int Command::undo_action( )
{
	return -1;
}


void Command::set_cursor_shape( int useX, int useY )
{
	ViewPort* view = cpointer().get_viewport();
	
	if (useX && useY) {
		view->viewport()->setCursor(QCursor(find_pixmap(":/cursorHoldLrud")));
	} else if (useX) {
		view->viewport()->setCursor(QCursor(find_pixmap(":/cursorHoldLr")));
	} else if (useY) {
		view->viewport()->setCursor(QCursor(find_pixmap(":/cursorHoldUd")));
	} else{
		view->reset_context();
	}
	
}

//eof
