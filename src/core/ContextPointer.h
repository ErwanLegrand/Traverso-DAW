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
 
    $Id: ContextPointer.h,v 1.10 2007/01/21 14:20:02 r_sijrier Exp $
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
	
	inline int on_first_input_event_x() const {return m_onFirstInputEventX; }
	inline int on_first_input_event_y() const {return m_onFirstInputEventY; }
        
	inline int on_first_input_event_scene_x() const {
		return (int) currentViewPort->mapToScene(m_onFirstInputEventX, m_onFirstInputEventY).x(); 
	}
	inline int on_first_input_event_scene_y() const {
		return (int) currentViewPort->mapToScene(m_onFirstInputEventX, m_onFirstInputEventY).y(); 
	}
	
	inline void inputengine_first_input_event( )
	{
		m_onFirstInputEventX = m_x;
		m_onFirstInputEventY = m_y;
	}
	
        void grab_mouse();
        void release_mouse();
	void reset_cursor();
 
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
        
	int m_onFirstInputEventX;
	int m_onFirstInputEventY;

        ViewPort* currentViewPort;
        QList<QObject* > contextItemsList;

};

#endif

// use this function to access the context pointer
ContextPointer& cpointer();

//eof


