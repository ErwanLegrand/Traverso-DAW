/*
    Copyright (C) 2005-2008 Remon Sijrier 
 
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
 
*/

#include "TrackPan.h"

#include "ViewPort.h"

#include "ContextPointer.h"
#include "Track.h"
#include "Mixer.h"
#include "TInputEventDispatcher.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
 *	\class TrackPan
	\brief Change (jog) the Panorama of a Track, or set to a pre-defined value
	
	\sa TraversoCommands
 */


TrackPan::TrackPan(Track* track, QVariantList args)
	: TCommand(track, "")
	, d(new Data)
{
        m_track = track;
	
	QString des;
	
	if (args.size() > 0) {
		m_newPan = args.at(0).toDouble();
		des = tr("Track Pan: %1").arg("Reset");
		m_origPan = m_track->get_pan();
	} else {
		des = tr("Track Pan");
	}
	
	setText(des);
}


int TrackPan::prepare_actions()
{
	delete d;
        return 1;
}


int TrackPan::begin_hold()
{
        d->origX = cpointer().x();
        m_origPan = m_newPan = m_track->get_pan();

        return 1;
}


int TrackPan::finish_hold()
{
	QCursor::setPos(d->mousePos);
	return 1;
}


int TrackPan::do_action()
{
        m_track->set_pan(m_newPan);
        return 1;
}


int TrackPan::undo_action()
{
        m_track->set_pan(m_origPan);
        return 1;
}

void TrackPan::cancel_action()
{
	finish_hold();
	undo_action();
}

void TrackPan::set_cursor_shape(int useX, int useY)
{
	Q_UNUSED(useX);
	Q_UNUSED(useY);
	
	d->mousePos = QCursor::pos();
	if (useX) {
		cpointer().setCursor(":/cursorHoldLr");
	} else {
		cpointer().setCursor("");
	}
}

int TrackPan::jog()
{
        float w = 600.0;
        float ofx = (float) d->origX - cpointer().x();
        float p = -2.0f *  (ofx) / w ;

        if (p > 0.0f && p < 0.01f) {
                p = 0.01;
        }
        if (p < 0.0f && p > -0.01f) {
                p = -0.01f;
        }

	m_newPan = p + m_newPan;
	
        if (m_newPan < -1.0f)
                m_newPan = -1.0f;
	if (m_newPan > 1.0) 
                m_newPan = 1.0f;
	
        if (fabs(m_newPan) < 0.01f) {
                m_newPan = 0.0f;
        }

        m_track->set_pan(m_newPan);
	
	QCursor::setPos(d->mousePos);

	cpointer().setCursorText(QByteArray::number(m_newPan, 'f', 2));
	return 1;
}

void TrackPan::pan_left(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	
        m_newPan -= 0.05f;
	if (m_newPan < -1.0) 
                m_newPan = -1.0f;
	m_track->set_pan(m_newPan);
	
	cpointer().setCursorText(QByteArray::number(m_newPan, 'f', 2));
}

void TrackPan::pan_right(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	
        m_newPan += 0.05f;
        if (m_newPan > 1.0f)
                m_newPan = 1.0f;
	m_track->set_pan(m_newPan);

	cpointer().setCursorText(QByteArray::number(m_newPan, 'f', 2));
}

void TrackPan::reset_pan(bool autorepeat)
{
	if (autorepeat) {
		return;
	}

	m_newPan = 0.0f;
	do_action();
	ied().bypass_jog_until_mouse_movements_exceeded_manhattenlength();
}
