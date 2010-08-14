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
 
    $Id: AddRemove.h,v 1.3 2008/01/21 16:22:11 r_sijrier Exp $
*/

#ifndef ADD_ITEM_COMMAND_H
#define ADD_ITEM_COMMAND_H

#include "TCommand.h"
#include <Tsar.h>

class ContextItem;
class TSession;

class AddRemove : public TCommand
{
public :
        AddRemove(ContextItem* parent, ContextItem* item, const QString& des);
        AddRemove(ContextItem* parent,
			void*  arg,
			bool historable,
                        TSession* sheet,
			const char* doActionSlot,
			const char* doSignal,
			const char* undoActionSlot,
			const char* undoSignal,
			const QString& des);
        AddRemove(ContextItem* parent,
                        ContextItem*  item,
                        bool historable,
                        TSession* sheet,
                        const char* doActionSlot,
                        const char* doSignal,
                        const char* undoActionSlot,
                        const char* undoSignal,
                        const QString& des);
        ~AddRemove();

        int prepare_actions();
        int do_action();
        int undo_action();
        
	void set_instantanious(bool instant);


private :
        ContextItem*	m_parentItem;
	void* 		m_arg;
        TsarEvent	m_doActionEvent;
        TsarEvent	m_undoActionEvent;
        TSession*	m_sheet;

        const char*	m_doActionSlot;
	const char*	m_undoActionSlot;
	const char*	m_doSignal;
	const char*	m_undoSignal;
        bool		m_instantanious;
};

#endif
 
 
