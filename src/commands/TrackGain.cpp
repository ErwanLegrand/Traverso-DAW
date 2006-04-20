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
 
    $Id: TrackGain.cpp,v 1.1 2006/04/20 14:51:13 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "TrackGain.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


TrackGain::TrackGain(Track* track, Song* song)
                : Command(track)
{
        m_track = track;
        m_song = song;
}


TrackGain::~TrackGain()
{}

int TrackGain::prepare_actions()
{
        return 1;
}

int TrackGain::begin_hold()
{
        int trackNumber;

        // Override m_track in case there was a number collection!
        if ( (trackNumber = ie().collected_number()) > 0) {
                Track* track = m_song->get_track(trackNumber);
                if (track) {
                        m_track = track;
                        PMESG("Starting jog gain for m_track %d", trackNumber);
                } else {
                        info().information(tr("TrackNumber %1 does not exist!").arg(trackNumber));
                        return 0;
                }
        }

        origY = cpointer().y();
        origGain = newGain = m_track->get_gain();

        return 1;
}

int TrackGain::finish_hold()
{
        return 1;
}

int TrackGain::do_action()
{
        PENTER;
        m_track->set_gain( newGain );
        return 1;
}

int TrackGain::undo_action()
{
        PENTER;
        m_track->set_gain( origGain );
        return 1;
}


int TrackGain::jog()
{
        int ofy = origY - cpointer().y();
        newGain  = (0.003f *  ofy)  + origGain;
        m_track->set_gain( newGain );
        return 1;
}
// eof

