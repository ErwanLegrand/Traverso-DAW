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

$Id: PCommand.cpp,v 1.5 2007/11/30 19:31:49 r_sijrier Exp $
*/

#include "PCommand.h"
#include "ContextItem.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


PCommand::PCommand(ContextItem* item, char* slot, const QString& des)
	: Command(item, des)
	, m_contextitem(item)
	, m_slot(slot)
{
}

PCommand::PCommand(ContextItem * item, char * slot, QVariant doValue, QVariant undoValue, const QString & des)
	: Command(item, des)
	, m_contextitem(item)
	, m_slot(slot)
	, m_doValue(doValue)
	, m_undoValue(undoValue)
{
	
}


PCommand::~PCommand()
{}

int PCommand::prepare_actions()
{
	return 1;
}

int PCommand::do_action()
{
	PENTER;
	if (!m_doValue.isNull()) {
		if (m_doValue.typeName() == QString("TimeRef")) {
			if (QMetaObject::invokeMethod(m_contextitem, m_slot, Qt::DirectConnection, Q_ARG(TimeRef, m_doValue.value<TimeRef>()))) {
				return 1;
			}
		}
		
		return -1;
	}
	 
	return QMetaObject::invokeMethod(m_contextitem, m_slot);
}

int PCommand::undo_action()
{
	PENTER;
	if (!m_undoValue.isNull()) {
		if (m_undoValue.typeName() == QString("TimeRef")) {
			if (QMetaObject::invokeMethod(m_contextitem, m_slot, Qt::DirectConnection, Q_ARG(TimeRef, m_undoValue.value<TimeRef>()))) {
				return 1;
			}
		}
		
		return -1;
	}
	
	return QMetaObject::invokeMethod(m_contextitem, m_slot);
}

int PCommand::finish_hold( )
{
	return 1;
}

int PCommand::begin_hold( )
{
	return QMetaObject::invokeMethod(m_contextitem, m_slot);
}

