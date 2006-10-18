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

$Id: Cursor.cpp,v 1.4 2006/10/18 12:08:56 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include <QPixmap>
#include <QRect>
#include <QPainter>
#include <QTimer>
#include "Cursor.h"
#include "ViewPort.h"
#include "SongView.h"
#include "TrackView.h"
#include <Utils.h>



// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Cursor::Cursor(SongView* sv, ViewPort* vp, Song* song)
		: ViewItem(vp, (ViewItem*) 0), m_sv(sv), m_vp(vp), m_song(song)
{
	m_vp->register_postdraw_item(this);
	m_vp->register_predraw_item(this);

	currentMode = NORMALMODE;
	
// 	fCursor = new FCursor(m_vp);
// 	fCursor->show();

	floatCursorOldPos = playCursorOldPos = workCursorOldPos = 0;
	floatCursorBackgroundBackup = QPixmap(0,0);
	workCursorBackgroundBackup = QPixmap(0,0);
	playCursorBackgroundBackup = QPixmap(0,0);

	connect(m_song, SIGNAL(transferStarted()), this, SLOT(set_cursor_playmode()));
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(set_cursor_normalmode()));
	connect(m_song, SIGNAL(workingPosChanged()), this, SLOT(set_workcursor_newpos()));
	connect(m_song, SIGNAL(firstVisibleFrameChanged()), this, SLOT(set_workcursor_newpos()));

	connect(&playTimer, SIGNAL(timeout()), this, SLOT(play_cursor_repaint()));


}

Cursor::~Cursor()
{
	PENTERDES;
}

QRect Cursor::predraw(QPainter& p)
{
	// Restore the background of the floatcursor old pos if necessary
	p.drawPixmap(floatCursorOldPos, 0, floatCursorBackgroundBackup);

	// Restore the background of the workcursor old pos if necessary
	if (workCursorOldPos >= TrackView::CLIPAREABASEX)
		p.drawPixmap(workCursorOldPos, 0, workCursorBackgroundBackup);

	// Restore the background of the playcursor old pos if necessary
	p.drawPixmap(playCursorOldPos, 0, playCursorBackgroundBackup);

	// 	return QRect(cursorOldPos  , 0, FLOATCURSORWIDTH, m_vp->height());
	return QRect();
}


QRect Cursor::postdraw(QPainter& p)
{
/*	if (m_vp->height() != fCursor->height()) {
		fCursor->resize(2, m_vp->height());
	}*/
// 	fCursor->move(playCursorNewPos, 0);


	floatCursorNewPos = cpointer().x();
	if (floatCursorNewPos < TrackView::CLIPAREABASEX)
		floatCursorNewPos = TrackView::CLIPAREABASEX;

	playCursorNewPos = m_song->get_playing_xpos() + TrackView::CLIPAREABASEX;
	if ( ( playCursorNewPos > (m_sv->cliparea_width() + TrackView::CLIPAREABASEX) ) || (playCursorNewPos < TrackView::CLIPAREABASEX) ) {
		playCursorNewPos = TrackView::CLIPAREABASEX;
		if (currentMode == PLAYMODE)
			m_song->set_first_visible_frame(m_song->get_transport_frame());
	}
	
	int workCursorNewPos = m_song->frame_to_xpos( m_song->get_working_frame() ) + TrackView::CLIPAREABASEX;
	if (workCursorPos != workCursorNewPos) {
		if (workCursorNewPos == 0)
			workCursorNewPos = 1;
		workCursorPos = workCursorNewPos;
	}


	floatCursorBackgroundBackup = m_vp->pixmap->copy(floatCursorNewPos, 0, FLOATCURSORWIDTH, m_vp->height());
	playCursorBackgroundBackup = m_vp->pixmap->copy(playCursorNewPos, 0, PLAYCURSORWIDTH, m_vp->height());
	workCursorBackgroundBackup = m_vp->pixmap->copy(workCursorPos, 0, WORKCURSORWIDTH, m_vp->height());


	if (currentMode == PLAYMODE) {
		// Paint the play cursor
		if (playCursorNewPos < TrackView::CLIPAREABASEX)
			playCursorNewPos = TrackView::CLIPAREABASEX;

		int clipAreaWidth = m_sv->cliparea_width();

		if (m_vp->height() != playCursor.height()) {
			playCursor = QPixmap(PLAYCURSORWIDTH, m_vp->height());
			playCursor.fill(QColor(0, 255, 0, 180));
		}

		if ( (playCursorNewPos >= 0 ) && ( playCursorNewPos <= clipAreaWidth + TrackView::CLIPAREABASEX ) ) {
			p.drawPixmap(playCursorNewPos, 0, playCursor);
			/*			p.setPen(QColor(0, 255, 0, 160));
						p.drawLine(playCursorNewPos, 0, playCursorNewPos, m_vp->height());*/
		}
	} else {
		if (m_vp->height() != floatCursor.height()) {
			floatCursor = QPixmap(FLOATCURSORWIDTH, m_vp->height());
			floatCursor.fill(QColor(0, 255, 0, 180));
		}
		p.drawPixmap(floatCursorNewPos, 0, floatCursor);
	}

	// Check if the workCursor needs resizing
	if (m_vp->height() != workCursor.height()) {
		workCursor = QPixmap(WORKCURSORWIDTH, m_vp->height());
		workCursor.fill(QColor(0, 0, 0, 0));

		QPixmap pix(1, 8);
		pix.fill(QColor(255, 0, 0, 190));
		QPainter wcpainter(&workCursor);
		int y=0;
		while (y < (m_vp->height() - 5) ) {
			wcpainter.drawPixmap(0, y, pix);
			y+=14;
		}
	}

// 	Paint the work cursor
	if ( (workCursorPos >= TrackView::CLIPAREABASEX) && (workCursorPos <=  m_sv->cliparea_width() + TrackView::CLIPAREABASEX) ) {
		p.drawPixmap( workCursorPos, 0, workCursor);
	}

	playCursorOldPos = playCursorNewPos;
	floatCursorOldPos = floatCursorNewPos;
	workCursorOldPos = workCursorNewPos;

// 	time = get_microseconds();

	return QRect();
}

void Cursor::set_cursor_playmode( )
{
	PENTER;
	currentMode = PLAYMODE;
	playTimer.start(40);
}

void Cursor::set_cursor_normalmode( )
{
	PENTER;
	currentMode = NORMALMODE;
	playTimer.stop();
	schedule_for_repaint();
}

void Cursor::set_workcursor_newpos()
{
	m_vp->schedule_for_repaint(this);
}

void Cursor::schedule_for_repaint( )
{
	m_vp->schedule_for_repaint(this);
}

void Cursor::play_cursor_repaint( )
{
	if ( (m_song->get_playing_xpos() + TrackView::CLIPAREABASEX) != playCursorOldPos ) {
/*		playCursorNewPos = m_song->get_playing_xpos() + TrackView::CLIPAREABASEX;
		fCursor->move(playCursorNewPos, 0);*/
		/*double newTime =  get_microseconds();
		double diff = newTime - time;
		printf("Diff is %.3f\n", diff / 1000.0); */
		schedule_for_repaint();
	}
}

FCursor::FCursor( ViewPort * parent )
	: QWidget(parent)
{
	QPalette palette;
	palette.setColor(QPalette::Background, QColor(Qt::black));
	setPalette(palette);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_PaintOnScreen);
	setAutoFillBackground(false);
        
	QPainter painter(&pixmap);
        pixmap.fill(QColor(Qt::black));
	setAttribute(Qt::WA_PaintOnScreen);
}

void FCursor::paintEvent( QPaintEvent * e )
{
// 	printf("entering FCursor paintEvent\n");
        QPainter directpainter(this);
        directpainter.drawPixmap(0, 0, pixmap);
}




HoldCursor::HoldCursor(ViewPort* vp, QPoint pos, const QString& name)
{
	m_vp = vp;
	pixmap = find_pixmap(name);
	m_pos.setX(pos.x() - pixmap.width()/2);
	m_pos.setY(pos.y());
}


QRect HoldCursor::draw( QPainter & painter )
{
// 	backup = m_vp->pixmap->copy(m_pos.x(), m_pos.y(), 32, 32);
	
	painter.drawPixmap(m_pos.x(), m_pos.y(), pixmap);
	return QRect();
}

QRect HoldCursor::get_geometry( )
{
	return QRect(m_pos.x(), m_pos.y(), pixmap.width(), pixmap.height());
}



//eof
