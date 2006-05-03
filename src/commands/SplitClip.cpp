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
 
    $Id: SplitClip.cpp,v 1.3 2006/05/03 11:59:39 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "SplitClip.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

SplitClip::SplitClip(Song* song, AudioClip* clip)
                : Command(clip)
{
        m_clip = clip;
        m_song = song;
        m_track = clip->get_track();
}


SplitClip::~SplitClip()
{}


int SplitClip::prepare_actions()
{
        splitPoint = m_song->xpos_to_frame( cpointer().clip_area_x());

        leftClip = m_clip->create_copy();
        rightClip = m_clip->create_copy();

        leftClip->set_right_edge(splitPoint);
        rightClip->set_left_edge(splitPoint);

        return 1;
}


int SplitClip::do_action()
{
        PENTER;
        m_track->remove_clip(m_clip);

        leftClip->get_track()->add_clip(leftClip);
        rightClip->get_track()->add_clip(rightClip);

        return 1;
}

int SplitClip::undo_action()
{
        PENTER;
        m_track->add_clip(m_clip);

        leftClip->get_track()->remove_clip(leftClip);
        rightClip->get_track()->remove_clip(rightClip);

        return 1;
}

// eof

