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

$Id: MoveClip.cpp,v 1.4 2006/05/03 11:59:39 r_sijrier Exp $
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
	originalTrackFirstBlock = newInsertBlock = m_clip->get_track_start_frame();
	origXPos = cpointer().x();
	return 1;
}


int MoveClip::finish_hold()
{
	int y = cpointer().y();
	targetTrack = m_song->get_track_under_y(y);
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
		m_clip->set_track_start_frame(newInsertBlock);
		targetTrack->add_clip(m_clip);
	}
	return 1;
}


int MoveClip::undo_action()
{
	PENTER;
	if (targetTrack)
		targetTrack->remove_clip(m_clip);
	m_clip->set_track_start_frame(originalTrackFirstBlock);
	originTrack->add_clip(m_clip);
	m_clip->set_track(originTrack);
	return 1;
}


int MoveClip::jog()
{
	int newXPos = cpointer().x();
	if ( newXPos < TrackView::CLIPAREABASEX )
		return 1;
	
	nframes_t diff = (newXPos - origXPos) * Peak::zoomStep[m_song->get_hzoom()];
	nframes_t origTrackStartFrame = m_clip->get_track_start_frame();
	int newTrackStartFrame = origTrackStartFrame + diff;
	
	if (newTrackStartFrame < 0)
		newTrackStartFrame = 0;
	
	newInsertBlock= m_song->xpos_to_frame(m_song->snapped_x(m_song->frame_to_xpos(newTrackStartFrame)));
	
	origXPos = newXPos;
	
	Track* newTargetTrack = m_song->get_track_under_y( cpointer().y() );
	
	if (!newTargetTrack)
		return 1;
	
	if (targetTrack && (newTargetTrack != originTrack)) {
		originTrack = targetTrack;
		targetTrack = newTargetTrack;
		do_action();
	} else {
		m_clip->set_track_start_frame(newInsertBlock);
	}
	return 1;
}


// eof

