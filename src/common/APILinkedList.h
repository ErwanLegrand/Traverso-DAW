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

class APILinkedListNode
{
public:
	APILinkedListNode () : next(0) {}
	virtual ~APILinkedListNode () {}
	APILinkedListNode* next;
	virtual bool is_smaller_then(APILinkedListNode* node) = 0;
};

#define apill_foreach(variable, upcasttype, apillist) \
        for(APILinkedListNode* apillnode = apillist.first(); apillnode!=0; apillnode = apillnode->next) \
                if (variable = (upcasttype*)apillnode) \

class APILinkedList
{
public:
	APILinkedList() : m_size(0), m_head(0), m_last(0) {}
	~APILinkedList() {}
	
	void append(APILinkedListNode* item);
	void add_and_sort(APILinkedListNode* node);
	void prepend(APILinkedListNode * item);
	int remove(APILinkedListNode* item);
	
	APILinkedListNode* first() const {return m_head;}
	APILinkedListNode* last() const {return m_last;}
	int size() const {return m_size;}
	void clear() {m_head = 0; m_last = 0; m_size=0;}
	void sort(APILinkedListNode* node);
	bool isEmpty() {return m_size == 0 ? true : false;}
	int indexOf(APILinkedListNode* node);
	APILinkedListNode* at(int i);

private:
	int m_size;
	APILinkedListNode* m_head;
	APILinkedListNode* m_last;
	APILinkedListNode* slow_last() const;

	void insert(APILinkedListNode* after, APILinkedListNode* item);
};

// T = O(1)
inline void APILinkedList::prepend(APILinkedListNode * item)
{
	item->next = m_head;
	m_head = item;
	if (!m_size) {
		m_last = item;
		m_last->next = 0;
	}
	m_size++;
	Q_ASSERT(m_last == slow_last());
}

// T = O(1)
inline void APILinkedList::append(APILinkedListNode * item)
{
	if(m_head) {
		m_last->next = item;
		m_last = item;
	} else {
		m_head = item;
		m_last = item;
	}
	
	m_last->next = 0;
	m_size++;
	
	Q_ASSERT(m_last == slow_last());
}

// T = O(n)
inline int APILinkedList::remove(APILinkedListNode * item)
{
	Q_ASSERT(item);
	
	if (!m_size) {
		return 0;
	}
	
	if(m_head == item) {
		m_head = m_head->next;
		m_size--;
		if (m_size == 0) {
			m_last = m_head = 0;
		}
		Q_ASSERT(m_last == slow_last());
		return 1;
	}

	APILinkedListNode *q,*r;
	r = m_head;
	q = m_head->next;
	
	while( q!=0 ) {
		if( q == item ) {
			r->next = q->next;
			m_size--;
			if (!q->next) {
				m_last = r;
				m_last->next = 0;
			}
			Q_ASSERT(m_last == slow_last());
			return 1;
		}

		r = q;
		q = q->next;
	}
		
	return 0;
}

// T = O(1)
inline void APILinkedList::insert(APILinkedListNode* after, APILinkedListNode* item)
{
	Q_ASSERT(after);
	Q_ASSERT(item);
	
	APILinkedListNode* temp;

	temp = item;
	temp->next = after->next;
	after->next = temp;
	
	if (after == m_last) {
		m_last = item;
		m_last->next = 0;
	}
	m_size++;
	
	Q_ASSERT(m_last == slow_last());
}

// T = O(n)
inline APILinkedListNode * APILinkedList::slow_last() const
{
	if (!m_size) {
		return 0;
	}
	
	APILinkedListNode* last = m_head;
	
	while(last->next) {
		last = last->next;
	}
	
	return last;
}

// T = O(n)
inline void APILinkedList::add_and_sort(APILinkedListNode * node)
{
	if (!m_size) {
		append(node);
	} else {
		APILinkedListNode* q = m_head;
		if (node->is_smaller_then(q)) {
			prepend(node);
		} else {
			APILinkedListNode* afternode = q;
			while (q) {
				if (!node->is_smaller_then(q)) {
					afternode = q;
				} else {
					break;
				}
				q = q->next;
			}
			insert(afternode, node);
		}
	}
}

inline int APILinkedList::indexOf(APILinkedListNode* node)
{
	Q_ASSERT(node);
	int index = 0;
	
	APILinkedListNode* q = m_head;
	while (q) {
		if (q == node) {
			return index;
		}
		++index;
		q = q->next;
	}
	return -1;
}

inline APILinkedListNode * APILinkedList::at(int i)
{
	Q_ASSERT(i >= 0);
	Q_ASSERT(i < m_size);
	
	int loopcounter = 0;
	APILinkedListNode* q = m_head;
	while (q) {
		if (loopcounter == i) {
			return q;
		}
		q = q->next;
		++loopcounter;
	}
	return 0;
}

inline void APILinkedList::sort(APILinkedListNode* node)
{
	if (remove(node)) {
		add_and_sort(node);
	}
}

#endif
