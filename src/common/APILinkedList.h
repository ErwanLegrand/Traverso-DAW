/*
Copyright (C) 2007 Remon Sijrier

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

*/

#ifndef API_LINKED_LIST_H
#define API_LINKED_LIST_H

#include <QDebug>

#include "AudioProcessingItem.h"

class APILinkedList
{
public:
	APILinkedList() : m_size(0), m_head(0) {}
	~APILinkedList() {}
	
	void append(AudioProcessingItem* item);
	void prepend(AudioProcessingItem * item);
	void add_after(AudioProcessingItem* after, AudioProcessingItem* item);
	int remove(AudioProcessingItem* item);
	
	AudioProcessingItem* begin() {return m_head;}
	AudioProcessingItem* get_next(AudioProcessingItem* item) {return item->next;}
	int size() const {return m_size;}

			
private:
	int m_size;
	AudioProcessingItem* m_head;
};

inline void APILinkedList::prepend(AudioProcessingItem * item)
{
	item->next = m_head;
	m_head = item;
}

inline void APILinkedList::append(AudioProcessingItem * item)
{
	if(m_head) {
		AudioProcessingItem *q,*temp;
		q = m_head;
		while( q->next != 0 ) {
			q = q->next;
		}

		temp = item;
		temp->next = 0;
		q->next = temp;
	} else {
		m_head = item;
		m_head->next = 0;
	}
	m_size++;
}

inline int APILinkedList::remove(AudioProcessingItem * item)
{
	AudioProcessingItem *q,*r;
	q = m_head;
	if(q == item)
	{
		m_head = q->next;
		m_size--;
		return 1;
	}

	r = q;
	while( q!=0 )
	{
		if( q == item )
		{
			r->next = q->next;
			m_size--;
			return 1;
		}

		r = q;
		q = q->next;
	}
		
	return 0;
}

inline void APILinkedList::add_after(AudioProcessingItem* after, AudioProcessingItem* item)
{
	Q_ASSERT(after);
	Q_ASSERT(item);
	
	AudioProcessingItem* temp;

	temp = item;
	temp->next = after->next;
	after->next = temp;
}

#endif
