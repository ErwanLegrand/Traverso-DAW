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

$Id: Command.cpp,v 1.7 2006/10/18 12:01:44 r_sijrier Exp $
*/

#include "Command.h"
#include "HistoryStack.h"
#include "ContextPointer.h"
#include <ViewPort.h>
#include <Utils.h>
#include "ContextItem.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Command::Command( const QString& des )
	: m_description(des)
{
	m_historyStack = 0;
	m_historable = true;
}

Command::Command(ContextItem* item, const QString& des)
	: m_description(des)
{
	m_context = item;
	m_historyStack = item->get_history_stack();
	m_historable = true;
}

Command::~Command()
{}

int Command::begin_hold(int useX, int useY)
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

void Command::set_valid(bool valid)
{
	m_isValid = valid;
}

int Command::push_to_history_stack( )
{
	PENTER;
	
	if (! m_historable) {
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

void Command::set_description(const QString& des )
{
	m_description = des;
}

bool Command::merg_with(const Command* )
{
	return false;
}

int Command::command_type( )
{
	return -1;
}

void Command::set_cursor_shape( int useX, int useY )
{
	ViewPort* view = cpointer().get_viewport();
	
	if (useX && useY) {
		view->setCursor(QCursor(find_pixmap(":/cursorHoldLrud")));
	} else if (useX) {
		view->setCursor(QCursor(find_pixmap(":/cursorHoldLr")));
	} else if (useY) {
		view->setCursor(QCursor(find_pixmap(":/cursorHoldUd")));
	} else{
		view->reset_context();
	}
	
}

//eof
