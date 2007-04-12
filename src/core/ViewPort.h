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
 
    $Id: ViewPort.h,v 1.13 2007/04/12 12:32:07 r_sijrier Exp $
*/

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QGraphicsView>
#include <QGraphicsItem>
#include "ContextItem.h"

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
        virtual void get_pointed_context_items(QList<ContextItem* > &list) = 0;

        // Set functions
	void set_holdcursor(const QString& cursorName);
	void set_holdcursor_text(const QString& text);
	void set_holdcursor_pos(QPoint pos);
	void set_current_mode(int mode);
        
	void reset_cursor();
	void set_jogging(bool jog);
	
	int get_current_mode() const {return m_mode;}


protected:
        virtual void enterEvent ( QEvent * );
        virtual void paintEvent( QPaintEvent* e);
        virtual void mouseMoveEvent(QMouseEvent* e);
        virtual void mousePressEvent ( QMouseEvent * e );
        virtual void mouseReleaseEvent ( QMouseEvent * e );
        virtual void mouseDoubleClickEvent ( QMouseEvent * e );
	virtual void wheelEvent ( QWheelEvent* e );
	virtual void keyPressEvent ( QKeyEvent* e);
	virtual void keyReleaseEvent ( QKeyEvent* e);
	void tabletEvent ( QTabletEvent * event );
	
private:
	int m_mode;
	HoldCursor*	m_holdcursor;
	QPoint		m_oldMousePos;
	QPointF lastMouseMoveScenePoint;
};


class HoldCursor : public ContextItem, public QGraphicsItem
{
	Q_OBJECT
public:
	HoldCursor();
	~HoldCursor();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void set_text(const QString& text);
	void set_type(const QString& type);
	void reset();

	QRectF boundingRect() const;

private:
	QPoint          m_pos;
	QPixmap		m_pixmap;
	QString         m_text;

};


#endif

//eof
