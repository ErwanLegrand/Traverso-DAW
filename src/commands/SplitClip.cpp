/*
Copyright (C) 2005-2007 Remon Sijrier 

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

#include "SplitClip.h"
		
#include <libtraversocore.h>
#include <SongView.h>
#include <AudioClipView.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

SplitClip::SplitClip(AudioClipView* view)
	: Command(view->get_clip(), tr("Split Clip"))
{
	m_clip = view->get_clip();
	m_sv = view->get_songview();
	m_track = m_clip->get_track();
	Q_ASSERT(m_clip->get_song());
}


int SplitClip::prepare_actions()
{
	nframes_t splitPoint = cpointer().scene_x() * m_sv->scalefactor;

	leftClip = resources_manager()->get_clip(m_clip->get_id());
	rightClip = resources_manager()->get_clip(m_clip->get_id());
	
	leftClip->set_song(m_clip->get_song());
	leftClip->set_track_start_frame( m_clip->get_track_start_frame() );
	leftClip->set_right_edge(splitPoint);
	
	rightClip->set_song(m_clip->get_song());
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
	
	resources_manager()->undo_remove_clip_from_database(leftClip->get_id());
	resources_manager()->undo_remove_clip_from_database(rightClip->get_id());
	
	return 1;
}

int SplitClip::undo_action()
{
	PENTER;
	ie().process_command(m_track->remove_clip(leftClip, false));
	ie().process_command(m_track->remove_clip(rightClip, false));
	
	ie().process_command(m_track->add_clip(m_clip, false));
	
	resources_manager()->remove_clip_from_database(leftClip->get_id());
	resources_manager()->remove_clip_from_database(rightClip->get_id());
	
	return 1;
}

// eof

