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

$Id: ContextPointer.cpp,v 1.10 2007/01/16 20:21:08 r_sijrier Exp $
*/

#include "ContextPointer.h"
#include "ContextItem.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

ContextPointer& cpointer()
{
	static ContextPointer contextPointer;
	return contextPointer;
}

ContextPointer::ContextPointer()
{
	m_x = 0;
	m_y = 0;
	currentViewPort = 0;
}

QList< QObject * > ContextPointer::get_context_items( )
{
	PENTER;
	QList<ContextItem* > pointedViewItems;
	
	if (currentViewPort) {
		currentViewPort->get_pointed_context_items(pointedViewItems);
	}

	QList<QObject* > contextItems;
	ContextItem* item;
	ContextItem*  nextItem;
	
	for (int i=0; i < pointedViewItems.size(); ++i) {
		item = pointedViewItems.at(i);
		contextItems.append(item);
		while ((nextItem = item->get_context())) {
			contextItems.append(nextItem);
			item = nextItem;
		}
	}

	if (currentViewPort) {
		contextItems.append(currentViewPort);
	}

	for (int i=0; i < contextItemsList.size(); ++i) {
		contextItems.append(contextItemsList.at(i));
	}


	return contextItems;
}

void ContextPointer::add_contextitem( QObject * item )
{
	if (! contextItemsList.contains(item))
		contextItemsList.append(item);
}

void ContextPointer::remove_contextitem(QObject* item)
{
	int index = contextItemsList.indexOf(item);
	contextItemsList.removeAt(index);
}

void ContextPointer::grab_mouse( )
{
	if (currentViewPort)
		currentViewPort->viewport()->grabMouse();

}

void ContextPointer::release_mouse( )
{
	if (currentViewPort)
		currentViewPort->viewport()->releaseMouse();
}

ViewPort * ContextPointer::get_viewport( )
{
	if (currentViewPort) {
		return currentViewPort;
	}
	
	return 0;
}

void ContextPointer::reset_cursor( )
{
	Q_ASSERT(currentViewPort);
		
	currentViewPort->reset_cursor();
}

//eof
