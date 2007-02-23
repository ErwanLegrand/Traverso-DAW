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

$Id: AddRemoveItemCommand.cpp,v 1.9 2007/02/23 14:00:53 r_sijrier Exp $
*/

#include "AddRemoveItemCommand.h"
#include "ContextItem.h"
#include <Song.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/** 	\class AddRemoveItemCommand 
 *	\brief Historably add/remove objects into the audio processing path without using locks
 *	
 *	AddRemoveItemCommand is a flexible class that let's you insert/remove Object that inherit from
 *	QObject into the audio processing execution path. 
 *
 *	Usage: Create one public and one private function (slot) for both the add and remove function
 *	The public function returns the AddRemoveItemCommand, it's constructor holds the private function
 *	signatures for the do and undo action. If the class has signals for the to be added/removed object, 
 *	then you can/should add those signatures to the constructor as well. Finally, the constructor 
 *	has a boolean flag that indicate if the Command should be historable (pushed to the history stack)
 *	or not. The signals (if you added the signatures) will be emited from within the gui thread, the 
 *	private functions that will do the actuall adding/removal of the object will be called from within
 *	the audio thread. DO NOT USE LOCKS OR ANY OTHER BLOCKING FUNCTION CALLS in the private functions!!
 */


AddRemoveItemCommand::AddRemoveItemCommand(ContextItem* parent, ContextItem* child, const QString& des)
	: Command(parent, des),
	m_parentItem(parent),
	m_childItem(child),
	m_song(0),
	m_doActionSlot(""),
	m_undoActionSlot(""),
	m_doSignal(""),
	m_undoSignal(""),
	m_instantanious(0)
{
	m_parentItem = parent;
	m_childItem = child;
	m_doActionEvent.valid = false;
	m_undoActionEvent.valid = false;
}


/**
 * 			Constructor with all the paramaters needed for proper operation.
 *
 * @param parent 	The object (which need to inherit ContextItem) where the object
 			will be added or removed.
 * @param child 	The object (which need to inherit ContextItem) that will be added/removed
 * @param historable 	Makes the command historable if set to true, else it will be deleted
 			after the do_action call. Only works when the command was requested 
 			from the InputEngine. If not, you are responsible yourself to call the 
 			do_action, and pushing it to the correct history stack.
 * @param song 		If a related Song object is available you can use it, else supply a 0
 * @param doActionSlot 	(private) slot signature which will be called on do_action()
 * @param doSignal 	If supplied, the signal with this signature will be emited in the GUI thread
 			AFTER the actuall  adding/removing action in do_action() has happened!
 * @param undoActionSlot (private) slot signature which will be called in undo_action();
 * @param undoSignal 	If supplied, the signal with this signature will be emited in the GUI thread
 			AFTER the actuall adding/removing in undo_action has happened!
 * @param des 		Short description that will show up in the history view.
 */
AddRemoveItemCommand::AddRemoveItemCommand(
	ContextItem * parent,
 	void* child,
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
	  m_undoSignal(undoSignal),
	  m_instantanious(0)
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
	PENTER3;
	if ( ! m_doActionEvent.valid ) {
		PWARN("No do action defined for this Command");
		return -1;
	}
	
	if (m_instantanious) {
		tsar().process_event_slot_signal(m_doActionEvent);
		return 1;
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
	PENTER3;
	
	if ( ! m_undoActionEvent.valid ) {
		PWARN("No undo action defined for this Command");
		return -1;
	}
	
	if (m_instantanious) {
		tsar().process_event_slot_signal(m_undoActionEvent);
		return 1;
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

/**
 * 	Set's the command to instantanious, the do/undo actions will call
 *	the slot and emit the signal (if they exist) instantaniously,
 *	and thus bypassing the RT thread save nature of Tsar.
 */
void AddRemoveItemCommand::set_instantanious(bool instant)
{
	m_instantanious = instant;
}


// eof
