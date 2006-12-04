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

$Id: ContextItem.h,v 1.8 2006/12/04 19:24:54 r_sijrier Exp $
*/
#ifndef CONTEXTITEM_H
#define CONTEXTITEM_H

#include <QObject>

class Command;
class QUndoStack;
class QUndoGroup;

class ContextItem : public QObject
{
	Q_OBJECT
public:
	ContextItem(QObject* parent);
	ContextItem();
	~ContextItem();

	ContextItem* get_context() const
	{
		return m_contextItem;
	}
	
	QUndoStack* get_history_stack() const;

	void set_history_stack(QUndoStack* hs);
	
	void set_context_item(ContextItem* item);
	

protected:
	QUndoStack* m_hs;

private:
	ContextItem* m_contextItem;

	friend class Tsar;
};

#endif

//eof
