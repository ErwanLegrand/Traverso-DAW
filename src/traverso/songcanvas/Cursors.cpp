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
 
    $Id: Cursors.cpp,v 1.4 2007/01/26 13:24:41 r_sijrier Exp $
*/

#include "Cursors.h"
#include "SongView.h"
#include "ClipsViewPort.h"
#include <QPen>
#include <Song.h>
#include <QScrollBar>
		
// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PlayCursor::PlayCursor(SongView* sv, Song* song, ClipsViewPort* vp)
	: ViewItem(0, song)
	, m_song(song)
	, m_vp(vp)
{
	m_sv = sv;
	m_mode = FLIP_PAGE;
	m_follow = true;
	
	connect(m_song, SIGNAL(transferStarted()), this, SLOT(play_start()));
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(play_stop()));
	
	connect(&m_playTimer, SIGNAL(timeout()), this, SLOT(update_position()));
	
	setZValue(100);
}

PlayCursor::~PlayCursor( )
{
        PENTERDES2;
}

void PlayCursor::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QColor color = QColor(180, 189, 240, 140);
	painter->fillRect(0, 0, 2, (int)m_boundingRectangle.height(), color);
}

void PlayCursor::play_start()
{
	m_playTimer.start(20);
}

void PlayCursor::play_stop()
{
	m_playTimer.stop();
}

void PlayCursor::update_position()
{
	QPointF newPos(m_song->get_transport_frame() / m_sv->scalefactor, 0);
	
	if (newPos != pos()) {
		setPos(newPos);
	} else {
		return;
	}
	
	QScrollBar* horizontalScrollbar = m_vp->horizontalScrollBar();
	int vpWidth = m_vp->viewport()->width();
	
	if (m_follow) {
		switch (m_mode) {
		case FLIP_PAGE: 
			{
				QPoint vppoint = m_vp->mapFromScene(pos());
				if (vppoint.x() < 0) {
					horizontalScrollbar->setValue((int) (pos().x() - (0.5 * vpWidth)) );
				} else if (vppoint.x() > ( vpWidth * 0.9) ) {
					horizontalScrollbar->setValue((int) (horizontalScrollbar->value() + (0.8 * vpWidth)) );
				}
			}
			break;
		
		case CENTERED:
			m_vp->centerOn(pos());
			break;
		
		case SCROLLED_FLIP_PAGE: 
			// anyone ?
			break;
		
		// never reached
		default: break;
		}
	}
}

void PlayCursor::set_bounding_rect( QRectF rect )
{
	m_boundingRectangle = rect;
}

bool PlayCursor::is_active()
{
	return m_playTimer.isActive();
}

void PlayCursor::set_active(bool active)
{
	if (active) {
		play_start();
	} else {
		play_stop();
	}
}

void PlayCursor::set_mode( PlayCursorMode mode )
{
	m_mode = mode;
}

void PlayCursor::toggle_folow( )
{
	m_follow = ! m_follow;
}


/**************************************************************/
/*                    WorkCursor                              */
/**************************************************************/


WorkCursor::WorkCursor(SongView* sv, Song* song)
	: ViewItem(0, song)
	, m_song(song)
	, m_sv(sv)
{
	connect(m_song, SIGNAL(workingPosChanged()), this, SLOT(update_position()));
	
	setZValue(99);
}

WorkCursor::~WorkCursor( )
{
        PENTERDES2;
}

void WorkCursor::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	QPen pen;
	pen.setWidth((int)m_boundingRectangle.width());
	pen.setBrush(QColor(255, 0, 0, 100));
	pen.setStyle(Qt::DashLine);
	painter->setPen(pen);
	painter->drawLine(0, 0, 0, (int)m_boundingRectangle.height());
}

void WorkCursor::update_position()
{
	setPos(m_song->get_working_frame() / m_sv->scalefactor, 0);
}

void WorkCursor::set_bounding_rect( QRectF rect )
{
	m_boundingRectangle = rect;
}


//eof
