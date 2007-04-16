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
 
    $Id: ContextPointer.h,v 1.14 2007/04/16 09:08:31 r_sijrier Exp $
*/

#ifndef CONTEXTPOINTER_H
#define CONTEXTPOINTER_H

#include <QObject>
#include <QTimer>

#include "ViewPort.h"

class ContextItem;

class ContextPointer : public QObject
{
	Q_OBJECT

public:
        /**
 	 * 	Returns the current ViewPort's mouse x coordinate
	
         * @return The current ViewPort's mouse x coordinate
         */
        inline int x() const {return m_x;}

	/**
	 * 	Returns the current ViewPort's mouse y coordinate
	
         * @return The current ViewPort's mouse y coordinate
         */
        inline int y() const {return m_y;}
	
	/**
	 *        Convenience function, equals QPoint(cpointer().x(), cpointer().y())
	
	* @return The current ViewPorts mouse position;
	 */
	
	inline QPoint pos() {return QPoint(m_x, m_y);}
	
	/**
	 * 	Convenience function that maps the ViewPort's mouse x <br />
		coordinate to the scene x coordinate
	
	 * @return The current scene x coordinate, mapped from the ViewPort's mouse x coordinate
	 */
	inline int scene_x() const {
		Q_ASSERT(currentViewPort);
		return (int) currentViewPort->mapToScene(m_x, m_y).x();
	}

	/**
	 * 	Convenience function that maps the ViewPort's mouse y <br />
		coordinate to the ViewPort's scene y coordinate
	
	 * @return The current ViewPort's scene y coordinate, mapped from the ViewPort's mouse y coordinate
	 */
	inline int scene_y() const {
		Q_ASSERT(currentViewPort);
		return (int) currentViewPort->mapToScene(m_x, m_y).y();
	}
	
	/**
	 * 	Returns the current's ViewPort's mouse position in the ViewPort's scene position.
	 * @return The current's ViewPort's mouse position in the ViewPort's scene position.
	 */
	inline QPointF scene_pos() const {
		Q_ASSERT(currentViewPort);
		return currentViewPort->mapToScene(m_x, m_y);
	}
	
	/**
	 *     	Used by ViewPort to update the internal state of ContextPointer
		Not intended to be used somewhere else.
	 * @param x The ViewPort's mouse x coordinate
	 * @param y The ViewPort's mouse y coordinate
	 */
	inline void set_point(int x, int y)
        {
		m_x = x;
                m_y = y;
                m_jogEvent = true;
        }
	
	/**
	 *        Returns the ViewPort x coordinate on first input event.
	 * @return The ViewPort x coordinate on first input event.
	 */
	inline int on_first_input_event_x() const {return m_onFirstInputEventX; }
	
	/**
	 *        Returns the ViewPort y coordinate on first input event.
	 * @return The ViewPort y coordinate on first input event.
	 */
	inline int on_first_input_event_y() const {return m_onFirstInputEventY; }

	/**
	 *        Returns the scene x coordinate on first input event.
	 * @return The scene x coordinate on first input event.
	 */
	inline int on_first_input_event_scene_x() const {
		return (int) currentViewPort->mapToScene(m_onFirstInputEventX, m_onFirstInputEventY).x(); 
	}
	
	/**
	 *        Returns the scene y coordinate on first input event.
	 * @return The scene y coordinate on first input event.
	 */
	inline int on_first_input_event_scene_y() const {
		return (int) currentViewPort->mapToScene(m_onFirstInputEventX, m_onFirstInputEventY).y(); 
	}
	
	/**
	 *        Called _only_ by InputEngine, not to be used anywhere else.
	 */
	inline void inputengine_first_input_event( )
	{
		m_onFirstInputEventX = m_x;
		m_onFirstInputEventY = m_y;
	}
	
	inline int get_current_mode() const {
		if (currentViewPort) {
			return currentViewPort->get_current_mode();
		}
		return -1;
	}
	
        void jog_start();
        void jog_finished();
	void reset_cursor();
 
        ViewPort* get_viewport();

        void set_current_viewport(ViewPort* vp)
        {
                currentViewPort = vp;
        }
        QList<QObject* > get_context_items();

        void add_contextitem(QObject* item);

        void remove_contextitem(QObject* item);
	
	QList<QObject* > get_contextmenu_items() const;
	void set_contextmenu_items(QList<QObject* > list);


private:
	ContextPointer();
        ContextPointer(const ContextPointer&);

        // allow this function to create one instance
        friend ContextPointer& cpointer();

        int m_x;
        int m_y;
	
	bool m_jogEvent;
	
	QTimer m_jogTimer;
	
	int m_onFirstInputEventX;
	int m_onFirstInputEventY;
	
	ViewPort* currentViewPort;
	QList<QObject* > contextItemsList;
	QList<QObject* > m_contextMenuItems;
	
	
private slots:
	void update_jog();
};

#endif

// use this function to access the context pointer
ContextPointer& cpointer();

//eof


