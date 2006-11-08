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
 
    $Id: PlayCursor.cpp,v 1.1 2006/11/08 14:45:22 r_sijrier Exp $
*/

#include "PlayCursor.h"

#include <Song.h>
		
// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PlayCursor::PlayCursor(Song* song)
	: ViewItem(0, song)
	, m_song(song)
{
	connect(m_song, SIGNAL(transferStarted()), this, SLOT(play_start()));
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(play_stop()));
	
	connect(&m_playTimer, SIGNAL(timeout()), this, SLOT(update_position()));
	
	setPos(1, 0);
	setZValue(100);
}

PlayCursor::~PlayCursor( )
{
        PENTERDES2;
}


QRectF PlayCursor::boundingRect( ) const
{
	return QRectF(0, 0, 2, 900);
}

void PlayCursor::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	QColor color = QColor(180, 189, 240, 200);
	painter->fillRect(0, 0, 2, 900, color);
}

void PlayCursor::play_start()
{
	m_playTimer.start(40);
}

void PlayCursor::play_stop()
{
	m_playTimer.stop();
}

void PlayCursor::update_position()
{
	setPos(m_song->get_playing_xpos(), 0);
}

//eof
 
