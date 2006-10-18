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
 
    $Id: Cursor.h,v 1.3 2006/10/18 12:08:56 r_sijrier Exp $
*/


#ifndef CURSOR_H
#define CURSOR_H

#include "ViewItem.h"

#include <QTimer>

class ViewPort;
class SongView;
class QPainter;
class QRect;
class Song;
class QPixmap;


class FCursor : public QWidget
{
	Q_OBJECT
	
public:

        FCursor(ViewPort* parent);

protected:
        void paintEvent( QPaintEvent* e);
	
private:
	ViewPort*	m_vp;
        QPixmap		pixmap;
};


class Cursor : public ViewItem
{
        Q_OBJECT

        static const int FLOATCURSORWIDTH = 1;
        static const int WORKCURSORWIDTH = 1;
        static const int PLAYCURSORWIDTH = 1;

public:

        enum mode { PLAYMODE, NORMALMODE};

        Cursor(SongView* sv, ViewPort* vp, Song* song);
        ~Cursor();

        QRect draw(QPainter& )
        {
                return QRect();
        }

        QRect predraw(QPainter& painter);
        QRect postdraw(QPainter& painter);


private:
        QPixmap floatCursorBackgroundBackup;
        QPixmap workCursorBackgroundBackup;
        QPixmap playCursorBackgroundBackup;

        QPixmap playCursor;
        QPixmap workCursor;
        QPixmap floatCursor;

        QTimer playTimer;

        SongView* m_sv;
        ViewPort* m_vp;
        Song* m_song;


        int floatCursorOldPos;
        int playCursorOldPos;
        int workCursorOldPos;

        int floatCursorNewPos;
        int playCursorNewPos;
        int workCursorPos;

        int currentMode;

        double time;
	
	FCursor*	fCursor;

public slots:
        void set_cursor_playmode();
        void set_cursor_normalmode();
        void set_workcursor_newpos();
        void schedule_for_repaint();
        void play_cursor_repaint();
};


class QPoint;

class HoldCursor
{

public:
        HoldCursor(ViewPort* vp, QPoint pos, const QString& name);
        ~HoldCursor() {};

        QRect draw(QPainter& painter);
	
	QRect get_geometry();

private:
	QPixmap		pixmap;
	QPoint		m_pos;
	ViewPort*	m_vp;

};


#endif

//eof

