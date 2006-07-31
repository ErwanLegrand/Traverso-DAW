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
 
    $Id: ContextPointer.h,v 1.2 2006/07/31 13:40:08 r_sijrier Exp $
*/

#ifndef CONTEXTPOINTER_H
#define CONTEXTPOINTER_H

#include <QObject>

#include "InputEngine.h"

class ContextItem;
class ViewPort;

class ContextPointer : public QObject
{
public:
        int x() const
        {
                return m_x;
        }
        int y() const
        {
                return m_y;
        }

        int clip_area_x() const;

        void set_point(int x, int y)
        {
                m_x = x;
                m_y = y;
                ie().jog();
        }

        void grab_mouse();
        void release_mouse();

        void set_current_viewport(ViewPort* vp)
        {
                currentViewPort = vp;
        }
        QList<QObject* > get_context_items();
        int get_viewport_width();
        
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


