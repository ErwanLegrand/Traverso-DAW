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

$Id: SplitClip.cpp,v 1.5 2006/06/26 23:57:08 r_sijrier Exp $
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

	leftClip->set_track_start_frame( m_clip->get_track_start_frame() );
	leftClip->set_right_edge(splitPoint);
	
	rightClip->set_left_edge(splitPoint);
	rightClip->set_track_start_frame( splitPoint );

	return 1;
}


int SplitClip::do_action()
{
	PENTER;
	THREAD_SAVE_REMOVE(m_clip, m_track, "remove_clip");

	THREAD_SAVE_ADD(leftClip, leftClip->get_track(), "add_clip");
	THREAD_SAVE_ADD(rightClip, rightClip->get_track(), "add_clip");
	
	return 1;
}

int SplitClip::undo_action()
{
	PENTER;
	THREAD_SAVE_ADD(m_clip, m_track, "add_clip");
	
	THREAD_SAVE_REMOVE(leftClip, leftClip->get_track(), "remove_clip");
	THREAD_SAVE_REMOVE(rightClip, rightClip->get_track(), "remove_clip");
	
	return 1;
}

// eof

