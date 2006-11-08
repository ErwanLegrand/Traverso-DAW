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
 
    $Id: AddRemoveItemCommand.h,v 1.2 2006/11/08 14:52:11 r_sijrier Exp $
*/

#ifndef ADD_ITEM_COMMAND_H
#define ADD_ITEM_COMMAND_H

#include "Command.h"
#include <Tsar.h>

class ContextItem;
class Song;

class AddRemoveItemCommand : public Command
{
public :
        AddRemoveItemCommand(ContextItem* parent, ContextItem* child, const QString& des);
        AddRemoveItemCommand(ContextItem* parent,
        			 ContextItem* child,
        			 bool historable,
        			 Song* song,
        			 char* doActionSlot,
        			 char* doSignal,
        			 char* undoActionSlot,
        			 char* undoSignal,
				 const QString& des);
        ~AddRemoveItemCommand();

        int prepare_actions();
        int do_action();
        int undo_action();


private :
        ContextItem*	m_parentItem;
        ContextItem*	m_childItem;
        TsarEvent	m_doActionEvent;
        TsarEvent	m_undoActionEvent;
        Song*		m_song;

        char*		m_doActionSlot;
        char*		m_undoActionSlot;
        char*		m_doSignal;
        char*		m_undoSignal;
};

#endif
 
 
