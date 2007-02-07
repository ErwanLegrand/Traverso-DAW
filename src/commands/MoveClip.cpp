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

$Id: MoveClip.cpp,v 1.19 2007/02/07 23:22:20 r_sijrier Exp $
*/

#include <libtraversocore.h>

#include "MoveClip.h"
#include "SnapList.h"
#include <SongView.h>
#include <TrackView.h>
#include <AudioClipView.h>
#include <ViewPort.h>
#include <ClipsViewPort.h>
#include <QScrollBar>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MoveClip::MoveClip(SongView* sv, AudioClipView* cv, AudioClip* clip)
	: Command(clip, QObject::tr("Move Clip"))
{
	m_sv = sv;
	m_cv = cv;
	m_song = clip->get_song();
	m_clip = clip;
	m_targetTrack = 0;
}


MoveClip::~MoveClip()
{}


int MoveClip::begin_hold()
{
	m_originTrack = m_targetTrack = m_clip->get_track();
	m_originalTrackFirstFrame = m_newInsertFrame = m_clip->get_track_start_frame();
	m_origPos = cpointer().pos();
	m_origXPos = cpointer().x();
	m_horizontalScrollBarValue = m_sv->get_clips_viewport()->horizontalScrollBar()->value();
	m_clip->set_snappable(false);
	m_sv->start_shuttle(true, true);
	return 1;
}


int MoveClip::finish_hold()
{
	m_clip->set_snappable(true);
	m_sv->start_shuttle(false);
		return 1;
}


int MoveClip::prepare_actions()
{
	return 1;
}


int MoveClip::do_action()
{
	PENTER;
	if (!m_targetTrack) {
		PMESG("Deleting clip %p",m_clip);
		ie().process_command(m_originTrack->remove_clip(m_clip, false));
		m_targetTrack = (Track*) 0;
	} else {
		ie().process_command(m_originTrack->remove_clip(m_clip, false));
		m_clip->set_track_start_frame(m_newInsertFrame);
		ie().process_command(m_targetTrack->add_clip(m_clip, false));
	}
	return 1;
}


int MoveClip::undo_action()
{
	PENTER;
	if (m_targetTrack)
		ie().process_command(m_targetTrack->remove_clip(m_clip, false));
	m_clip->set_track_start_frame(m_originalTrackFirstFrame);
	ie().process_command(m_originTrack->add_clip(m_clip, false));
	return 1;
}


int MoveClip::jog()
{
// 	printf("jog\n");
	int scrollbardif = m_horizontalScrollBarValue - m_sv->get_clips_viewport()->horizontalScrollBar()->value();
	
	QPointF diffPoint(cpointer().pos() - m_origPos);
	QPointF newPos(m_cv->pos() + diffPoint);
	
	m_origPos = cpointer().pos();
	
// 	printf("newPos x, y is %f, %f\n", newPos.x(), newPos.y());
	
	TrackView* trackView = m_sv->get_trackview_under(cpointer().scene_pos());
	if (!trackView) {
// 		printf("no trackview returned\n");
	} else if (trackView != m_cv->get_trackview()) {
// 		printf("Setting new TrackView!\n");
		m_cv->set_trackview(trackView);
		m_cv->setParentItem(trackView);
		m_targetTrack = trackView->get_track();
// 		printf("track id is %d\n", m_targetTrack->get_id());
	}
		

	int newXPos = cpointer().x() - scrollbardif;

	SnapList* slist = m_song->get_snap_list();

	// must be signed int because it can be negative
	int diff_f = (cpointer().x() - m_origXPos - scrollbardif) * m_sv->scalefactor;
	nframes_t origTrackStartFrame = m_clip->get_track_start_frame();
	nframes_t origTrackEndFrame = m_clip->get_track_end_frame();
	long newTrackStartFrame = origTrackStartFrame + diff_f;
	nframes_t newTrackEndFrame = origTrackEndFrame + diff_f;
// 	printf("newTrackStartFrame is %d\n", newTrackStartFrame);

	// attention: newTrackStartFrame is unsigned, can't check for negative values
	if (newTrackStartFrame < 0) {
		newTrackStartFrame = 0;
	}

	// "nframe_t" domain, but must be signed ints because they can become negative
	int snapStartDiff = 0;
	int snapEndDiff = 0;
	int snapDiff = 0;

	if (m_song->is_snap_on()) {

		// check if there is anything to snap
		bool start_snapped = false;
		bool end_snapped = false;
		if (slist->is_snap_value(newTrackStartFrame)) {
			start_snapped = true;
		}
		if (slist->is_snap_value(newTrackEndFrame)) {
			end_snapped = true;
		}

		if (start_snapped) {
			snapStartDiff = slist->get_snap_diff(newTrackStartFrame) / m_sv->scalefactor;
			snapDiff = snapStartDiff; // in case both ends snapped, change this value later, else leave it
		}

		if (end_snapped) {
			snapEndDiff = slist->get_snap_diff(newTrackEndFrame) / m_sv->scalefactor; 
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

	m_newInsertFrame = newTrackStartFrame - (snapDiff * m_sv->scalefactor);

	// store the new position only if the clip was moved, but not if it stuck to a snap position
	if (origTrackStartFrame != m_newInsertFrame) {
		m_origPos.setX(newXPos);
	}

	newPos.setX(m_newInsertFrame / m_sv->scalefactor);	
	newPos.setY(m_cv->pos().y());
	m_cv->setPos(newPos);
	
	m_sv->update_shuttle_factor();

	return 1;
}


// eof

