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

$Id: MoveClip.cpp,v 1.6 2006/07/03 17:51:56 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "TrackView.h"
#include "MoveClip.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MoveClip::MoveClip(Song* song, AudioClip* clip)
		: Command(clip)
{
	m_song = song;
	m_clip = clip;
	targetTrack = 0;
}


MoveClip::~MoveClip()
{}


int MoveClip::begin_hold()
{
	originTrack = targetTrack = m_clip->get_track();
	originalTrackFirstFrame = newInsertFrame = m_clip->get_track_start_frame();
	origXPos = cpointer().x();
	return 1;
}


int MoveClip::finish_hold()
{
	int y = cpointer().y();
	targetTrack = m_song->get_track_under_y(y);
	// m_clip could be moved to another track due jogging
	// so we remove it from there!
	m_clip->get_track()->remove_clip( m_clip );
	return 1;
}


int MoveClip::prepare_actions()
{
	return 1;
}


int MoveClip::do_action()
{
	PENTER;
	if (!targetTrack) {
		PMESG("Deleting clip %p",m_clip);
		originTrack->remove_clip(m_clip);
		targetTrack = (Track*) 0;
	} else {
		originTrack->remove_clip(m_clip);
		m_clip->set_track_start_frame(newInsertFrame);
		targetTrack->add_clip(m_clip);
	}
	return 1;
}


int MoveClip::undo_action()
{
	PENTER;
	if (targetTrack)
		targetTrack->remove_clip(m_clip);
	m_clip->set_track_start_frame(originalTrackFirstFrame);
	originTrack->add_clip(m_clip);
	m_clip->set_track(originTrack);
	return 1;
}


int MoveClip::jog()
{
	int newXPos = cpointer().x();
	
	if ( newXPos < TrackView::CLIPAREABASEX ) {
		newXPos = TrackView::CLIPAREABASEX;
	}
	
	nframes_t diff = (newXPos - origXPos) * Peak::zoomStep[m_song->get_hzoom()];
	nframes_t origTrackStartFrame = m_clip->get_track_start_frame();
	int newTrackStartFrame = origTrackStartFrame + diff;
	
	if (newTrackStartFrame < 0)
		newTrackStartFrame = 0;
	
	newInsertFrame= m_song->xpos_to_frame(m_song->snapped_x(m_song->frame_to_xpos(newTrackStartFrame)));
	m_clip->set_track_start_frame(newInsertFrame);
	
	origXPos = newXPos;
	
	// This potentially leaves a Clip into a Track where it should have been 
	// removed. This happens when a song is playing, and thread save add/remove
	// is used. Disabled until a solid implementation has been made
	// (i.e. moving an audioclip visually without adding/removing it during move to/from Tracks!!!
/*	Track* newTargetTrack = m_song->get_track_under_y( cpointer().y() );
	Track* currentTrack = m_clip->get_track();

	if (newTargetTrack == currentTrack) {
		m_clip->set_track_start_frame(newInsertFrame);
		
	} else {
		currentTrack->remove_clip( m_clip );
		newTargetTrack->add_clip( m_clip );
		m_clip->set_track_start_frame(newInsertFrame);
	}*/
	
	return 1;
}


// eof

