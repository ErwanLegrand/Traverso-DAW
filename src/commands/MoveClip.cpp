/*
Copyright (C) 2005-2008 Remon Sijrier 

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

#include "MoveClip.h"

#include "AudioClip.h"
#include "AudioClipManager.h"
#include "ContextPointer.h"
#include "InputEngine.h"
#include "SnapList.h"
#include "Sheet.h"
#include "Track.h"
#include "TimeLine.h"

#include "ClipsViewPort.h"
#include "SheetView.h"
#include "TrackView.h"
#include "AudioClipView.h"

#include "Zoom.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
  *	\class MoveClip
	\brief A Command class for Dragging or Copy-dragging an AudioClip
	 
	\sa TraversoCommands
 */


/**
 *	Creates  a Move Clip or Copy Clip Command object.
 */
MoveClip::MoveClip(ViewItem* view, QVariantList args)
	: Command(view->get_context(), "")
	, d(new Data)
{
	QString action = "move"; // default action!
	
	if (args.size() > 0) {
		action = args.at(0).toString();
	}
	if (args.size() > 1) {
		d->verticalOnly = args.at(1).toBool();
	} else {
		d->verticalOnly = false;
	}
	
	QString des;
	if (action == "copy") {
		des = tr("Copy Clip");
		m_actionType = COPY;
	} else if (action == "move") {
		des = tr("Move Clip");
		m_actionType = MOVE;
	} else if (action == "move_to_start") {
		des = tr("Move Clip To Start");
		m_actionType = MOVE_TO_START;
	} else if (action == "move_to_end") {
		des = tr("Move Clip To End");
		m_actionType = MOVE_TO_END;
	} else if (action == "fold_sheet") {
		des = tr("Fold Sheet");
		m_actionType = FOLD_SHEET;
	} else if (action == "fold_track") {
		des = tr("Fold Track");
		m_actionType = FOLD_TRACK;
	} else if (action == "fold_markers") {
		des = tr("Fold Markers");
		m_actionType = FOLD_MARKERS;
	} else {
		PERROR("MoveClip: Unknown action type: %s", QS_C(action));
	}
	
	setText(des);
	
	if (m_actionType == FOLD_SHEET || m_actionType == FOLD_TRACK || m_actionType == FOLD_MARKERS) {
		
		QList<AudioClip*> movingClips;
		QList<Track*> tracks;
		
		if (m_actionType == FOLD_TRACK) {
			TrackView* tv = qobject_cast<TrackView*>(view);
			Q_ASSERT(tv);
			d->sv= tv->get_sheetview();
			tracks.append(tv->get_track());
		} else if (m_actionType == FOLD_SHEET) {
			d->sv = qobject_cast<SheetView*>(view);
			Q_ASSERT(d->sv);
			tracks = d->sv->get_sheet()->get_tracks();
		} else {
			d->sv = qobject_cast<SheetView*>(view->get_sheetview());
			Q_ASSERT(d->sv);
		}
		
		TimeRef currentLocation = TimeRef(cpointer().on_first_input_event_scene_x() * d->sv->timeref_scalefactor);
		
		if (d->sv->get_trackview_under(cpointer().scene_pos())) {
			d->pointedTrackIndex = d->sv->get_trackview_under(cpointer().scene_pos())->get_track()->get_sort_index();
		} else {
			d->pointedTrackIndex = 0;
		}
		
		if (m_actionType == FOLD_SHEET || m_actionType == FOLD_MARKERS) {
			QList<Marker*> movingMarkers = d->sv->get_sheet()->get_timeline()->get_markers();
			foreach(Marker* marker, movingMarkers) {
				if (marker->get_when() > currentLocation) {
					MarkerAndOrigin markerAndOrigin;
					markerAndOrigin.marker = marker;
					markerAndOrigin.origin = marker->get_when();
					m_markers.append(markerAndOrigin);
				}
			}
		}
		
		if (m_actionType == FOLD_SHEET || m_actionType == FOLD_TRACK) {
			foreach(Track* track, tracks) {
				QList<AudioClip*> clips = track->get_cliplist();
				foreach(AudioClip* clip, clips) {
					if (clip->get_track_end_location() > currentLocation) {
						movingClips.append(clip);
					}
				}
			}
		}
		
		m_group.set_clips(movingClips);
		
	} else {
		AudioClipView* cv = qobject_cast<AudioClipView*>(view);
		Q_ASSERT(cv);
		d->sv = cv->get_sheetview();
		AudioClip* clip  = cv->get_clip();
		if (clip->is_selected()) {
			QList<AudioClip*> selected;
			clip->get_sheet()->get_audioclip_manager()->get_selected_clips_state(selected);
			m_group.set_clips(selected);
		} else {
			m_group.add_clip(clip);
		}
		d->pointedTrackIndex = clip->get_track()->get_sort_index();
	}
	
	m_origTrackIndex = m_newTrackIndex = m_group.get_track_index();
	if (m_group.get_size() == 0 && m_markers.count() > 0) {
		m_trackStartLocation = m_markers[0].origin;
	} else {
		m_trackStartLocation = m_group.get_track_start_location();
	}
	m_sheet = d->sv->get_sheet();
	d->zoom = 0;
}

MoveClip::~MoveClip()
{
	if (d) {
		if (d->zoom) {
			delete d->zoom;
		}
		delete d;
	}
}

int MoveClip::begin_hold()
{
	if ((!m_group.get_size() || m_group.is_locked()) && !m_markers.count()) {
		return -1;
	}
	
	if (m_actionType == COPY) {
		// FIXME Memory leak here!
		QList<AudioClip*> newclips = m_group.copy_clips();
		m_group.set_clips(newclips);
		m_group.add_all_clips_to_tracks();
		m_group.move_to(m_origTrackIndex, m_trackStartLocation + TimeRef(d->sv->timeref_scalefactor * 3));
	}
	
	m_group.set_as_moving(true);
	
	d->sv->stop_follow_play_head();
	d->sv->start_shuttle(true, true);
	d->sceneXStartPos = cpointer().on_first_input_event_scene_x();
	
	return 1;
}


int MoveClip::finish_hold()
{
	m_group.set_as_moving(false);
	
	d->sv->start_shuttle(false);

	return 1;
}


int MoveClip::prepare_actions()
{
	if (d->zoom) {
		delete d->zoom;
	}
	delete d;
	d = 0;
	
	if (m_actionType == COPY) {
		m_group.remove_all_clips_from_tracks();
	}
	
	if (m_origTrackIndex == m_newTrackIndex &&  m_posDiff == TimeRef() && 
	    ! (m_actionType == COPY || m_actionType == MOVE_TO_START || m_actionType == MOVE_TO_END) ) {
		return -1;
	}
	
	return 1;
}


int MoveClip::do_action()
{
	PENTER;
	if (m_actionType == MOVE || m_actionType == FOLD_SHEET || m_actionType == FOLD_TRACK) {
		m_group.move_to(m_newTrackIndex, m_trackStartLocation + m_posDiff);
	}
	else if (m_actionType == COPY) {
		m_group.add_all_clips_to_tracks();
		m_group.move_to(m_newTrackIndex, m_trackStartLocation + m_posDiff);
	}
	else if (m_actionType == MOVE_TO_START) {
		move_to_start(false);
	}
	else if (m_actionType == MOVE_TO_END) {
		move_to_end(false);
	}
	
	foreach(MarkerAndOrigin markerAndOrigin, m_markers) {
		markerAndOrigin.marker->set_when(markerAndOrigin.origin + m_posDiff);
	}
	
	return 1;
}


int MoveClip::undo_action()
{
	PENTER;
	
	if (m_actionType == COPY) {
		m_group.remove_all_clips_from_tracks();
	} else {
		m_group.move_to(m_origTrackIndex, m_trackStartLocation);
	}

	foreach(MarkerAndOrigin markerAndOrigin, m_markers) {
		markerAndOrigin.marker->set_when(markerAndOrigin.origin);
	}
	
	return 1;
}

void MoveClip::cancel_action()
{
	finish_hold();
	undo_action();
}

int MoveClip::jog()
{
	if (d->zoom) {
		d->zoom->jog();
		return 0;
	}
	
	TrackView* trackView = d->sv->get_trackview_under(cpointer().scene_pos());
	int deltaTrackIndex = 0;
	if (trackView/* && !(m_actionType == FOLD_SHEET)*/) {
		deltaTrackIndex = trackView->get_track()->get_sort_index() - d->pointedTrackIndex;
		m_group.check_valid_track_index_delta(deltaTrackIndex);
		m_newTrackIndex = m_newTrackIndex + deltaTrackIndex;
		d->pointedTrackIndex = trackView->get_track()->get_sort_index();
	}

	// Calculate the distance moved based on the current scene x pos and the initial one.
	// Only assign if we the movements is allowed in horizontal direction
	TimeRef diff_f;
	if (!d->verticalOnly) {
		diff_f = (cpointer().scene_x() - d->sceneXStartPos) * d->sv->timeref_scalefactor;
	}
	
	// If the moved distance (diff_f) makes as go beyond the left most position (== 0, or TimeRef())
	// set the newTrackStartLocation to 0. Else calculate it based on the original track start location
	// and the distance moved.
	TimeRef newTrackStartLocation;
	if (diff_f < TimeRef() && m_trackStartLocation < (-1 * diff_f)) {
		newTrackStartLocation = qint64(0);
	} else {
		newTrackStartLocation = m_trackStartLocation + diff_f;
	}

	// substract the snap distance, if snap is turned on.
	if (m_sheet->is_snap_on() && !d->verticalOnly) {
		newTrackStartLocation -= m_sheet->get_snap_list()->calculate_snap_diff(newTrackStartLocation, newTrackStartLocation + m_group.get_length());
	}
	
	// Now that the new track start location is known, the position diff can be calculated
	m_posDiff = newTrackStartLocation - m_trackStartLocation;
	
	// and used to move the group to it's new location.
	m_group.move_to(m_newTrackIndex, m_trackStartLocation + m_posDiff);
	
	// and used to move the markers
	foreach(MarkerAndOrigin markerAndOrigin, m_markers) {
		markerAndOrigin.marker->set_when(markerAndOrigin.origin + m_posDiff);
	}
	
	d->sv->update_shuttle_factor();
	
	cpointer().get_viewport()->set_holdcursor_pos(d->sv->get_clips_viewport()->mapToScene(cpointer().pos()).toPoint());
	cpointer().get_viewport()->set_holdcursor_text(timeref_to_text(newTrackStartLocation, d->sv->timeref_scalefactor));


	return 1;
}


void MoveClip::next_snap_pos(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	do_prev_next_snap(m_sheet->get_snap_list()->next_snap_pos(m_group.get_track_start_location()),
			  m_sheet->get_snap_list()->next_snap_pos(m_group.get_track_end_location()));
}

void MoveClip::prev_snap_pos(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	do_prev_next_snap(m_sheet->get_snap_list()->prev_snap_pos(m_group.get_track_start_location()),
			m_sheet->get_snap_list()->prev_snap_pos(m_group.get_track_end_location()));
}

void MoveClip::do_prev_next_snap(TimeRef trackStartLocation, TimeRef trackEndLocation)
{
	if (d->verticalOnly) return;
	ie().bypass_jog_until_mouse_movements_exceeded_manhattenlength();
	trackStartLocation -= m_sheet->get_snap_list()->calculate_snap_diff(trackStartLocation, trackEndLocation);
	m_posDiff = trackStartLocation - m_trackStartLocation;
	do_move();
}

void MoveClip::move_to_start(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	m_group.move_to(m_group.get_track_index(), TimeRef());
}

void MoveClip::move_to_end(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	m_group.move_to(m_group.get_track_index(), m_sheet->get_last_location());
}

void MoveClip::move_up(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	ie().bypass_jog_until_mouse_movements_exceeded_manhattenlength();
	int deltaTrackIndex = -1;
	m_group.check_valid_track_index_delta(deltaTrackIndex);
	m_newTrackIndex = m_newTrackIndex + deltaTrackIndex;
	do_move();
}

void MoveClip::move_down(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	ie().bypass_jog_until_mouse_movements_exceeded_manhattenlength();
	int deltaTrackIndex = 1;
	m_group.check_valid_track_index_delta(deltaTrackIndex);
	m_newTrackIndex = m_newTrackIndex + deltaTrackIndex;
	do_move();
}

void MoveClip::move_left(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	if (d->verticalOnly) return;
	ie().bypass_jog_until_mouse_movements_exceeded_manhattenlength();
	m_posDiff -= d->sv->timeref_scalefactor;
	do_move();
}

void MoveClip::move_right(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	if (d->verticalOnly) return;
	ie().bypass_jog_until_mouse_movements_exceeded_manhattenlength();
	m_posDiff += d->sv->timeref_scalefactor;
	do_move();
}

void MoveClip::start_zoom(bool autorepeat)
{
	if (autorepeat) return;
	
	if (!d->zoom) {
		d->zoom = new Zoom(d->sv, QList<QVariant>() << "HJogZoom" << "1.2" << "0.2");
		d->zoom->begin_hold();
		cpointer().get_viewport()->set_holdcursor(":/cursorZoomHorizontal");
		d->sv->start_shuttle(false);
	} else {
		cpointer().get_viewport()->set_holdcursor(":/cursorHoldLrud");
		d->sv->start_shuttle(true, true);
	}
}

void MoveClip::set_cursor_shape(int useX, int useY)
{
	if (useX && useY) {
		cpointer().get_viewport()->set_holdcursor(":/cursorHoldLrud");
	} else if (useX) {
		cpointer().get_viewport()->set_holdcursor(":/cursorHoldLr");
	} else {
		cpointer().get_viewport()->set_holdcursor(":/cursorHoldUd");
	}
}

void MoveClip::toggle_vertical_only(bool autorepeat)
{
	d->verticalOnly = !d->verticalOnly;
	if (d->verticalOnly) {
		set_cursor_shape(0, 1);
	} else {
		set_cursor_shape(1, 1);
	}
}

void MoveClip::do_move()
{
	m_group.move_to(m_newTrackIndex, m_trackStartLocation + m_posDiff);
	if (d) {
		cpointer().get_viewport()->set_holdcursor_text(timeref_to_text(m_trackStartLocation + m_posDiff, d->sv->timeref_scalefactor));
	}
}
