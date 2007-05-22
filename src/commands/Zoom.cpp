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
 
    $Id: Zoom.cpp,v 1.15 2007/05/22 20:54:22 benjie Exp $
*/

#include <libtraversocore.h>

#include "Zoom.h"

#include "SongView.h"
#include "TrackView.h"
#include <QPoint>
#include <ViewPort.h>
#include <ClipsViewPort.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

Zoom::Zoom(SongView* sv)
	: Command("Zoom")
{
        m_sv = sv;
}


int Zoom::prepare_actions()
{
        return -1;
}


int Zoom::begin_hold()
{
        jogZoomTotalX = cpointer().get_viewport()->viewport()->width();
        verticalJogZoomLastY = cpointer().on_first_input_event_y();
        baseJogZoomXFactor = m_sv->get_song()->get_hzoom() - ((int) ( (float) (jogZoomTotalX - cpointer().on_first_input_event_x()) / jogZoomTotalX * 50 ) + 1 );
	
	return 1;
}


int Zoom::finish_hold()
{
	return 1;
}


void Zoom::set_cursor_shape( int useX, int useY )
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	ViewPort* view = cpointer().get_viewport();
	view->viewport()->setCursor(themer()->get_cursor("Zoom"));
	mousePos = QCursor::pos();	
}

int Zoom::jog()
{
        PENTER;
        int x = cpointer().x();
        int y = cpointer().y();
        int jzxfactor = (int) ( (float) (jogZoomTotalX - x) / jogZoomTotalX * 50 ) + 1;
        if (jzxfactor==0)
                jzxfactor=1;

        if (jzxfactor != lastJogZoomXFactor) {
                lastJogZoomXFactor = jzxfactor;
                int newHZoom = jzxfactor + baseJogZoomXFactor;
                if ( newHZoom < 0 )
                        m_sv->get_song()->set_hzoom(0);
                else if ( newHZoom > Peak::ZOOM_LEVELS -1 )
                        m_sv->get_song()->set_hzoom(Peak::ZOOM_LEVELS - 1);
                else
                        m_sv->get_song()->set_hzoom(newHZoom);
                m_sv->center();
        }

        int vzy = y - verticalJogZoomLastY;
        if (vzy>10) {
                m_sv->vzoom_in();
                verticalJogZoomLastY = verticalJogZoomLastY + 10;
        } else if (vzy<-10) {
                m_sv->vzoom_out();
                verticalJogZoomLastY = verticalJogZoomLastY - 10;
        }
        return 1;
}

int Zoom::do_action( )
{
	return -1;
}

int Zoom::undo_action( )
{
	return -1;
}


// eof
