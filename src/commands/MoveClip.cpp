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

$Id: MoveClip.cpp,v 1.8 2006/07/31 15:34:28 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "TrackView.h"
#include "MoveClip.h"
#include "SnapList.h"

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
	originTrack = targetTrack = currentTrack = m_clip->get_track();
	originalTrackFirstFrame = newInsertFrame = m_clip->get_track_start_frame();
	origXPos = cpointer().x();
	m_song->update_snaplist(m_clip);
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

	SnapList *slist = m_song->get_snap_list();

	// must be signed int because it can be negative
	int diff_f = (newXPos - origXPos) * Peak::zoomStep[m_song->get_hzoom()];
	nframes_t origTrackStartFrame = m_clip->get_track_start_frame();
	nframes_t origTrackEndFrame = m_clip->get_track_end_frame();
	nframes_t newTrackStartFrame = origTrackStartFrame + diff_f;
	nframes_t newTrackEndFrame = origTrackEndFrame + diff_f;

	// attention: newTrackStartFrame is unsigned, can't check for negative values
	if (-diff_f >= (int)newTrackStartFrame)
		newTrackStartFrame = 0;

	// "nframe_t" domain, but must be signed ints because they can become negative
	int snapStartDiff = 0;
	int snapEndDiff = 0;
	int snapDiff = 0;

	if (m_song->is_snap_on()) {

		// check if there is anything to snap
		bool start_snapped = false;
		bool end_snapped = false;
		if (slist->is_snap_value(m_song->frame_to_xpos(newTrackStartFrame))) start_snapped = true;
		if (slist->is_snap_value(m_song->frame_to_xpos(newTrackEndFrame))) end_snapped = true;

		if (start_snapped) {
			snapStartDiff = slist->get_snap_diff(m_song->frame_to_xpos(newTrackStartFrame)) 
				* Peak::zoomStep[m_song->get_hzoom()];
			snapDiff = snapStartDiff; // in case both ends snapped, change this value later, else leave it
		}

		if (end_snapped) {
			snapEndDiff = slist->get_snap_diff(m_song->frame_to_xpos(newTrackEndFrame)) 
				* Peak::zoomStep[m_song->get_hzoom()];
			snapDiff = snapEndDiff; // in case both ends snapped, change this value later, else leave it
		}

		// If both snapped, check which one is closer. Do not apply this check if one of the
		// ends hasn't snapped, because it's diff value will be 0 by default and will always
		// be smaller than the actually snapped value.
		if (start_snapped && end_snapped) {
			if (abs(snapEndDiff) > abs(snapStartDiff))
				snapDiff = snapStartDiff;
			else
				snapDiff = snapEndDiff;
		}
	}

	newInsertFrame = newTrackStartFrame - snapDiff;

	// store the new position only if the clip was moved, but not if it stuck to a snap position
	if (origTrackStartFrame != newInsertFrame)
		origXPos = newXPos;

	Track* newTargetTrack = m_song->get_track_under_y( cpointer().y() );

	if (newTargetTrack == currentTrack) {
		m_clip->set_track_start_frame(newInsertFrame);
		
	} else {
		if (currentTrack) {
			currentTrack->remove_clip( m_clip );
		}
		if (newTargetTrack) {
			newTargetTrack->add_clip( m_clip );
		}
		m_clip->set_track_start_frame(newInsertFrame);
		currentTrack = newTargetTrack;
	}
	
	return 1;
}


// eof

