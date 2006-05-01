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

$Id: ContextItem.h,v 1.3 2006/05/01 21:21:37 r_sijrier Exp $
*/
#ifndef CONTEXTITEM_H
#define CONTEXTITEM_H

#include <QObject>
#include "HistoryStack.h"

class ContextItem : public QObject
{
	Q_OBJECT
public:
	ContextItem(ContextItem* item=0, QObject* parent=0);
	~ContextItem();

	ContextItem* get_context() const
	{
		return m_contextItem;
	}
	
	HistoryStack* get_history_stack() const;

	void set_history_stack(HistoryStack* hs);
	
	void set_context_item(ContextItem* item);

protected:
	HistoryStack* m_hs;

private:
	ContextItem* m_contextItem;

};

#endif

//eof
