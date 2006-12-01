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
 
    $Id: ViewPort.h,v 1.1 2006/12/01 13:59:36 r_sijrier Exp $
*/

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QtGui>

class ViewItem;
class ContextItem;
class Import;
class Track;
class HoldCursor;

class ViewPort : public QGraphicsView
{
        Q_OBJECT

public :
        ViewPort(QWidget* parent);
        ViewPort(QGraphicsScene* scene, QWidget* parent);
        ~ViewPort();

        // Get functions
        virtual void get_pointed_context_items(QList<ContextItem* > &list);

        // Set functions

	void set_hold_cursor(const QString& cursorName);
	void set_hold_cursor_text(const QString& text);
        void reset_context();


protected:
        void leaveEvent ( QEvent * );
        void enterEvent ( QEvent * );
        void resizeEvent(QResizeEvent* e);
        virtual void paintEvent( QPaintEvent* e);
        void mouseMoveEvent(QMouseEvent* e);

private:


signals:
        void contextChanged();

};

#endif

//eof
