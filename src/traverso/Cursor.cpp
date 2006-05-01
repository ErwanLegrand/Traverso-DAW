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
 
    $Id: Cursor.cpp,v 1.2 2006/05/01 21:31:58 r_sijrier Exp $
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


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Cursor::Cursor(SongView* sv, ViewPort* vp, Song* song)
                : ViewItem(vp, (ViewItem*) 0), m_sv(sv), m_vp(vp), m_song(song)
{
        m_vp->register_postdraw_item(this);
        m_vp->register_predraw_item(this);

        currentMode = NORMALMODE;

        floatCursorOldPos = workCursorOldPos = playCursorOldPos = 0;
        floatCursorBackgroundBackup = QPixmap(0,0);
        workCursorBackgroundBackup = QPixmap(0,0);
        playCursorBackgroundBackup = QPixmap(0,0);

        connect(m_song, SIGNAL(transferStarted( )), this, SLOT(set_cursor_playmode( )) );
        connect(m_song, SIGNAL(transferStopped( )), this, SLOT(set_cursor_normalmode( )) );
        connect(m_song, SIGNAL(workingPosChanged(int )), this, SLOT(set_workcursor_newpos(int )));

        connect(&playTimer, SIGNAL(timeout( )), this, SLOT(play_cursor_repaint()));


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
        p.drawPixmap(workCursorOldPos, 0, workCursorBackgroundBackup);

        // Restore the background of the playcursor old pos if necessary
        p.drawPixmap(playCursorOldPos, 0, playCursorBackgroundBackup);

        // 	return QRect(cursorOldPos  , 0, FLOATCURSORWIDTH, m_vp->height());
        return QRect();
}


QRect Cursor::postdraw(QPainter& p)
{


        floatCursorNewPos = cpointer().x();
        if (floatCursorNewPos < TrackView::CLIPAREABASEX)
                floatCursorNewPos = TrackView::CLIPAREABASEX;

        playCursorNewPos = m_song->get_playing_xpos() + TrackView::CLIPAREABASEX;
        if ( ( playCursorNewPos > (m_sv->cliparea_width() + TrackView::CLIPAREABASEX) ) || (playCursorNewPos < TrackView::CLIPAREABASEX) ) {
                playCursorNewPos = TrackView::CLIPAREABASEX;
                if (currentMode == PLAYMODE)
                        m_song->set_first_block(m_song->get_transfer_frame());
        }


        floatCursorBackgroundBackup = m_vp->pixmap.copy(floatCursorNewPos, 0, FLOATCURSORWIDTH, m_vp->height());
        workCursorBackgroundBackup = m_vp->pixmap.copy(workCursorPos, 0, WORKCURSORWIDTH, m_vp->height());
        playCursorBackgroundBackup = m_vp->pixmap.copy(playCursorNewPos, 0, PLAYCURSORWIDTH, m_vp->height());


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

        // Paint the work cursor
        p.drawPixmap( workCursorPos, 0, workCursor);

        workCursorOldPos = workCursorPos;
        playCursorOldPos = playCursorNewPos;
        floatCursorOldPos  = floatCursorNewPos;

        time = get_microseconds();

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

void Cursor::set_workcursor_newpos( int newPos )
{
        workCursorPos = newPos + TrackView::CLIPAREABASEX;
        m_vp->schedule_for_repaint(this);
}

void Cursor::schedule_for_repaint( )
{
        m_vp->schedule_for_repaint(this);
}

void Cursor::play_cursor_repaint( )
{
        if ( (m_song->get_playing_xpos() + TrackView::CLIPAREABASEX) != playCursorOldPos ) {
                /*		double newTime =  get_microseconds();
                		double diff = newTime - time;
                		printf("Diff is %.3f\n", diff / 1000.0); */
                schedule_for_repaint();
        }
}


//eof
