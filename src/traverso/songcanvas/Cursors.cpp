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
 
    $Id: Cursors.cpp,v 1.14 2007/03/29 11:10:43 r_sijrier Exp $
*/

#include "Cursors.h"
#include "SongView.h"
#include "ClipsViewPort.h"
#include <QPen>
#include <Song.h>
#include <Config.h>
#include <Themer.h>
#include <QScrollBar>
		
// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

PlayHead::PlayHead(SongView* sv, Song* song, ClipsViewPort* vp)
	: ViewItem(0, song)
	, m_song(song)
	, m_vp(vp)
{
	m_sv = sv;
	m_mode = (PlayHeadMode) config().get_property("PlayHead", "Scrollmode", ANIMATED_FLIP_PAGE).toInt();
	m_follow = config().get_property("PlayHead", "Follow", true).toBool();
	
	m_animation.setDuration(1300);
	m_animation.setCurveShape(QTimeLine::SineCurve);
	
	connect(m_song, SIGNAL(transferStarted()), this, SLOT(play_start()));
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(play_stop()));
	
	connect(&m_playTimer, SIGNAL(timeout()), this, SLOT(update_position()));
	
	connect(&m_animation, SIGNAL(frameChanged(int)), this, SLOT(set_animation_value(int)));
	connect(&m_animation, SIGNAL(finished()), this, SLOT(animation_finished()));
	
	setZValue(100);
}

PlayHead::~PlayHead( )
{
        PENTERDES2;
}

void PlayHead::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QColor color;
	
	if (m_song->is_transporting()) {
	 	color = themer()->get_color("Playhead:active");
	} else {
		color = themer()->get_color("Playhead:inactive");
	}
	
	painter->fillRect(0, 0, (int)m_boundingRect.width(), (int)m_boundingRect.height(), color);
}

void PlayHead::play_start()
{
	m_playTimer.start(20);
	
	if (m_animation.state() == QTimeLine::Running) {
		m_animation.stop();
		m_animation.setCurrentTime(0);
	}
}

void PlayHead::play_stop()
{
	m_playTimer.stop();
	
	if (m_animation.state() == QTimeLine::Running) {
		m_animation.stop();
		m_animation.setCurrentTime(0);
	}
	
	update();
}

void PlayHead::update_position()
{
	QPointF newPos(m_song->get_transport_frame() / m_sv->scalefactor, 1);
	
	if (newPos != pos() && (m_animation.state() != QTimeLine::Running)) {
		setPos(newPos);
	} else {
		return;
	}
	
	QScrollBar* horizontalScrollbar = m_vp->horizontalScrollBar();
	int vpWidth = m_vp->viewport()->width();
	
	if ( ! m_follow) {
		return;
	}
	
	if (m_mode == CENTERED) {
		horizontalScrollbar->setValue((int)scenePos().x() - (int)(0.5 * vpWidth));
		return;
	}
	 
	QPoint vppoint = m_vp->mapFromScene(pos());
	
	if (vppoint.x() < 0 || (vppoint.x() > vpWidth)) {
		
		// If the playhead is _not_ in the viewports range, center it in the middle!
		horizontalScrollbar->setValue((int) ((int)scenePos().x() - (0.5 * vpWidth)) );
	
	} else if (vppoint.x() > ( vpWidth * 0.82) ) {
		
		// If the playhead is in the viewports range, and is nearing the end
		// either start the animated flip page, or flip the page and place the 
		// playhead cursor ~ 1/10 from the left viewport border
		if (m_mode == ANIMATED_FLIP_PAGE) {
			if (m_animation.state() != QTimeLine::Running) {
				m_animation.setFrameRange(0, (int)(vpWidth * 0.75));
				m_animationScrollPosition = horizontalScrollbar->value();
				//during the animation, we stop the play update timer
				// to avoid unnecessary update/paint events
				play_stop();
				m_animation.start();
			}
		} else {
			horizontalScrollbar->setValue((int) ((int)scenePos().x() - (0.1 * vpWidth)) );
		}
	}
}


void PlayHead::set_animation_value(int value)
{
	QPointF newPos(m_song->get_transport_frame() / m_sv->scalefactor, 0);
	// calculate the motion distance of the playhead.
	qreal deltaX = newPos.x() - pos().x();

	// 16 seems to be the division factor with a QTimeLine running for
	// 1300 ms, and 3/4 of the viewport width. Don't ask me why :-) 
	// Due the playhead moves as well during the animation, we have to 
	// compensate for this, by adding it's delta x to the animation 
	// 'scroll' position
	m_animationScrollPosition += (int)(value/16 + deltaX);
	
	if (newPos != pos()) {
		setPos(newPos);
	}
	
	m_vp->horizontalScrollBar()->setValue(m_animationScrollPosition);
}


void PlayHead::animation_finished()
{
	if (m_song->is_transporting()) {
		play_start();
	}
}


void PlayHead::set_bounding_rect( QRectF rect )
{
	m_boundingRect = rect;
}

bool PlayHead::is_active()
{
	return m_playTimer.isActive();
}

void PlayHead::set_active(bool active)
{
	if (active) {
		play_start();
	} else {
		play_stop();
	}
}

void PlayHead::set_mode( PlayHeadMode mode )
{
	m_mode = mode;
}

void PlayHead::toggle_follow( )
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
	
	painter->fillRect(0, 0, 2, (int)m_boundingRect.height(), themer()->get_color("Workcursor:default"));
}

void WorkCursor::update_position()
{
	setPos(m_song->get_working_frame() / m_sv->scalefactor, 1);
}

void WorkCursor::set_bounding_rect( QRectF rect )
{
	m_boundingRect = rect;
}


//eof


