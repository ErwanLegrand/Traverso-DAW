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
 
    $Id: ViewPort.h,v 1.4 2006/10/18 12:08:56 r_sijrier Exp $
*/

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QMouseEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QWidget>
#include <QPixmap>
#include <QPainter>

class ViewItem;
class ContextItem;
class Import;
class Track;
class HoldCursor;

class ViewPort : public QWidget
{
        Q_OBJECT

public :
        ViewPort(QWidget* widget);
        ~ViewPort();

        // Get functions
        void get_pointed_view_items(QList<ViewItem* > &list);

        // Set functions


        void schedule_for_repaint(ViewItem* view);
        void register_predraw_item(ViewItem* item);
        void register_postdraw_item(ViewItem* item);

        void register_viewitem(ViewItem* item);
        void unregister_viewitem(ViewItem* item);
        
	void set_hold_cursor(const QString& cursorName);
        void reset_context();

        QPixmap* 	pixmap;

protected:
        void leaveEvent ( QEvent * );
        void enterEvent ( QEvent * );
        void resizeEvent(QResizeEvent* e);
        void paintEvent( QPaintEvent* e);
        void mouseMoveEvent(QMouseEvent* e);
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);


private:
        QList<ViewItem* > repaintViewItemList;
        QList<ViewItem* > viewItemList;
        QList<ViewItem* > predrawItemList;
        QList<ViewItem* > postdrawItemList;

	HoldCursor* 	m_holdCursor;
        Import* 	import;
        Track*		importTrack;
        QString		importFileName;
	
	bool		scheduledForRepaint;

        void clear_repaintviewitemlist();


signals:
        void resized();
        void pointChanged();
        void contextChanged();

};

#endif

//eof
