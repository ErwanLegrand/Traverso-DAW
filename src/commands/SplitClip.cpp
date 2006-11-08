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

$Id: SplitClip.cpp,v 1.10 2006/11/08 14:52:11 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "SplitClip.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

SplitClip::SplitClip(Song* song, AudioClip* clip)
	: Command(clip, QObject::tr("Split Clip"))
{
	m_clip = clip;
	m_song = song;
	m_track = clip->get_track();
}


SplitClip::~SplitClip()
{}


int SplitClip::prepare_actions()
{
	splitPoint = m_song->xpos_to_frame( cpointer().x());

	AudioSourceManager* manager = pm().get_project()->get_audiosource_manager();
	
	leftClip = manager->get_clip(m_clip->get_id());
	rightClip = manager->get_clip(m_clip->get_id());

	leftClip->set_track_start_frame( m_clip->get_track_start_frame() );
	leftClip->set_right_edge(splitPoint);
	
	rightClip->set_left_edge(splitPoint);
	rightClip->set_track_start_frame( splitPoint);

	return 1;
}


int SplitClip::do_action()
{
	PENTER;
	ie().process_command(m_track->remove_clip(m_clip, false));

	ie().process_command(m_track->add_clip(leftClip, false));
	ie().process_command(m_track->add_clip(rightClip, false));
	
	return 1;
}

int SplitClip::undo_action()
{
	PENTER;
	ie().process_command(m_track->add_clip(m_clip, false));
	
	ie().process_command(m_track->remove_clip(leftClip, false));
	ie().process_command(m_track->remove_clip(rightClip, false));
	
	
	return 1;
}

// eof

