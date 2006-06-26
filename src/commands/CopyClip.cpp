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
 
    $Id: CopyClip.cpp,v 1.3 2006/06/26 23:57:08 r_sijrier Exp $
*/

#include "CopyClip.h"

#include <libtraversocore.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


CopyClip::CopyClip(Song* song, AudioClip* clip)
                : Command(clip)
{
        m_song = song;
        m_clip = clip;
        newCreatedClip = (AudioClip*) 0;
}


CopyClip::~CopyClip()
{}


int CopyClip::begin_hold()
{
        return 1;
}


int CopyClip::finish_hold()
{
        int x = m_song->snapped_x(cpointer().clip_area_x());
        int y = cpointer().y();
        newInsertBlock = m_song->xpos_to_frame( x );
        targetTrack = m_song->get_track_under_y(y);
        return 1;
}


int CopyClip::prepare_actions()
{
        newCreatedClip = m_clip->create_copy();
        return 1;
}

int CopyClip::do_action()
{
        PENTER;
	THREAD_SAVE_ADD(newCreatedClip, targetTrack, "add_clip");
        return 1;
}


int CopyClip::undo_action()
{
        PENTER;
        targetTrack->remove_clip(newCreatedClip);
	THREAD_SAVE_REMOVE(newCreatedClip, targetTrack, "remove_clip");
        return 1;
}


int CopyClip::jog()
{
        return 1;
}


// eof

