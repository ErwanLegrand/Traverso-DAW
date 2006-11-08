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
 
    $Id: ContextItem.cpp,v 1.5 2006/11/08 14:49:37 r_sijrier Exp $
*/

#include "ContextItem.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ContextItem::ContextItem(QObject* parent)
        : QObject(parent), m_hs(0), m_contextItem(0)
{
}

ContextItem::ContextItem()
	: QObject(), m_hs(0), m_contextItem(0)
{
}

ContextItem::~ ContextItem( )
{}

void ContextItem::set_history_stack( QUndoStack * hs )
{
	m_hs = hs;
}

QUndoStack* ContextItem::get_history_stack( ) const
{
	return m_hs;
}

void ContextItem::set_context_item( ContextItem * item )
{
	m_contextItem = item;
}
