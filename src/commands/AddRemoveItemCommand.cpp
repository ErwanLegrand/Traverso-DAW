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

$Id: AddRemoveItemCommand.cpp,v 1.5 2007/01/24 21:15:47 r_sijrier Exp $
*/

#include "AddRemoveItemCommand.h"
#include "ContextItem.h"
#include <Song.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AddRemoveItemCommand::AddRemoveItemCommand(ContextItem* parent, ContextItem* child, const QString& des)
	: Command(parent, des),
	m_parentItem(parent),
	m_childItem(child),
	m_song(0),
	m_doActionSlot(""),
	m_undoActionSlot(""),
	m_doSignal(""),
	m_undoSignal("")
{
	m_parentItem = parent;
	m_childItem = child;
	m_doActionEvent.valid = false;
	m_undoActionEvent.valid = false;
}


AddRemoveItemCommand::AddRemoveItemCommand(
	ContextItem * parent,
	ContextItem * child,
	bool historable,
	Song* song,
	char * doActionSlot,
	char * doSignal,
	char * undoActionSlot,
	char * undoSignal,
	const QString& des)
  	: Command(parent, des),
	  m_parentItem(parent),
	  m_childItem(child),
	  m_song(song),
	  m_doActionSlot(doActionSlot),
	  m_undoActionSlot(undoActionSlot),
	  m_doSignal(doSignal),
	  m_undoSignal(undoSignal)
{
	m_isHistorable = historable;
}

AddRemoveItemCommand::~AddRemoveItemCommand()
{}

int AddRemoveItemCommand::prepare_actions()
{
	Q_ASSERT(m_parentItem);
	Q_ASSERT(m_childItem);
	Q_ASSERT(m_doActionSlot != "");
	Q_ASSERT(m_undoActionSlot != "");
	
	m_doActionEvent = tsar().create_event(m_parentItem, m_childItem, m_doActionSlot, m_doSignal);
	
	
	m_undoActionEvent = tsar().create_event(m_parentItem, m_childItem, m_undoActionSlot, m_undoSignal);
	
	return 1;
}

int AddRemoveItemCommand::do_action()
{
	PENTER;
	if ( ! m_doActionEvent.valid ) {
		PWARN("No do action defined for this Command");
		return -1;
	}
	
	if (m_song) {
		if (m_song->is_transporting()) {
			PMESG("Using Thread Save add/remove");
			tsar().add_event(m_doActionEvent);
		} else {
			tsar().process_event_slot_signal(m_doActionEvent);
		}
	} else {
		tsar().add_event(m_doActionEvent);
	}
	
	return 1;
}

int AddRemoveItemCommand::undo_action()
{
	PENTER;
	
	if ( ! m_undoActionEvent.valid ) {
		PWARN("No undo action defined for this Command");
		return -1;
	}
	
	if (m_song) {
		if (m_song->is_transporting()) {
			PMESG("Using Thread Save add/remove");
			tsar().add_event(m_undoActionEvent);
		} else {
			tsar().process_event_slot_signal(m_undoActionEvent);
		}
	} else {
		PMESG("Using direct add/remove/signaling");
			tsar().add_event(m_undoActionEvent);
	}
	
	return 1;
}


// eof
