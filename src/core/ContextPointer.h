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
 
    $Id: ContextPointer.h,v 1.7 2006/12/01 13:58:45 r_sijrier Exp $
*/

#ifndef CONTEXTPOINTER_H
#define CONTEXTPOINTER_H

#include <QObject>

#include "InputEngine.h"
#include "ViewPort.h"

class ContextItem;

class ContextPointer : public QObject
{
public:
        inline int x() const {return m_x;}
        inline int y() const {return m_y;}
	inline QPoint pos() {return QPoint(m_x, m_y);}
	
	inline int scene_x() const {
		Q_ASSERT(currentViewPort);
		return (int) currentViewPort->mapToScene(m_x, m_y).x();
	}

	inline int scene_y() const {
		Q_ASSERT(currentViewPort);
		return (int) currentViewPort->mapToScene(m_x, m_y).y();
	}
	
	inline QPointF scene_pos() const {
		Q_ASSERT(currentViewPort);
		return currentViewPort->mapToScene(m_x, m_y);
	}
	
	inline void set_point(int x, int y)
        {
                m_x = x;
                m_y = y;
                ie().jog();
        }

        void grab_mouse();
        void release_mouse();
 
        ViewPort* get_viewport();

        void set_current_viewport(ViewPort* vp)
        {
                currentViewPort = vp;
        }
        QList<QObject* > get_context_items();

        void add_contextitem(QObject* item);

        void remove_contextitem(QObject* item);


private:
        ContextPointer();
        ContextPointer(const ContextPointer&);

        // allow this function to create one instance
        friend ContextPointer& cpointer();

        int m_x;
        int m_y;

        ViewPort* currentViewPort;
        QList<QObject* > contextItemsList;

};

#endif

// use this function to access the context pointer
ContextPointer& cpointer();

//eof


