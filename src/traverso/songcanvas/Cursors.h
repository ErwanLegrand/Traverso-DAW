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
 
    $Id: Cursors.h,v 1.5 2007/01/26 13:24:41 r_sijrier Exp $
*/

#ifndef CURSORS_H
#define CURSORS_H

#include "ViewItem.h"
#include <QTimer>

class Song;
class SongView;
class ClipsViewPort;
		
class PlayCursor : public ViewItem
{
        Q_OBJECT

public:
        PlayCursor(SongView* sv, Song* song, ClipsViewPort* vp);
        ~PlayCursor();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void set_bounding_rect(QRectF rect);
	
	bool is_active();
	void set_active(bool active);
	
	enum PlayCursorMode {
		FLIP_PAGE,
		CENTERED,
		SCROLLED_FLIP_PAGE
	};
	
	void set_mode(PlayCursorMode mode);
	void toggle_folow();

private:
	Song*		m_song;
        QTimer		m_playTimer;
        ClipsViewPort*	m_vp;
        bool 		m_follow;
        PlayCursorMode	m_mode;

private slots:
        void play_start();
        void play_stop();
public slots:
        void update_position();
};



class WorkCursor : public ViewItem
{
        Q_OBJECT

public:
        WorkCursor(SongView* sv, Song* song);
        ~WorkCursor();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void set_bounding_rect(QRectF rect);

private:
	Song*		m_song;
	SongView*	m_sv;

public slots:
        void update_position();
};


#endif

//eof
