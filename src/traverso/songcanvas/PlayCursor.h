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
 
    $Id: PlayCursor.h,v 1.1 2006/11/08 14:45:22 r_sijrier Exp $
*/

#ifndef PLAY_CURSOR_H
#define PLAY_CURSOR_H

#include "ViewItem.h"
		
class Song;
		
class PlayCursor : public ViewItem
{
        Q_OBJECT

public:
        PlayCursor(Song* song);
        ~PlayCursor();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QRectF boundingRect() const;
	
// 	int type() const;

private:
	Song*		m_song;
        QTimer		m_playTimer;

private slots:
        void play_start();
        void play_stop();
        void update_position();
};

// inline int PlayCursor::type( ) const {return PLAYCURSOR;}

#endif

//eof
