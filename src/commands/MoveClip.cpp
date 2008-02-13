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

#include "MoveClip.h"

#include "AudioClip.h"
#include "ContextPointer.h"
#include "ProjectManager.h"
#include "ResourcesManager.h"
#include "SnapList.h"
#include "Sheet.h"
#include "Track.h"

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
 *	
 * @param cv The AudioClipView that is to be dragged/copied.
 * @param arguments Can be either one of the following: move, copy, move_to_end, move_to_start
 */
MoveClip::MoveClip(AudioClipView* cv, QVariantList args)
	: Command(cv->get_clip(), "")
	, d(new Data)
{
	m_actionType = "move"; // default action!
	
	if (args.size() > 0) {
		m_actionType = args.at(0).toString();
	}
	if (args.size() > 1) {
		d->verticalOnly = args.at(1).toBool();
	} else {
		d->verticalOnly = false;
	}
	
	d->view = cv;
	d->sv = d->view->get_sheetview();
	d->zoom = 0;
	m_sheet = d->sv->get_sheet();
	m_targetTrack = 0;
	
	QString des;
	if (m_actionType == "copy") {
		des = tr("Copy Clip");
	} else if (m_actionType == "move") {
		des = tr("Move Clip");
	} else if (m_actionType == "move_to_start") {
		des = tr("Move Clip To Start");
	} else if (m_actionType == "move_to_end") {
		des = tr("Move Clip To End");
	}
	
	setText(des);
	
	if (m_actionType == "move_to_start" || m_actionType == "move_to_end") {
		init_data();
	} else {
		m_clip = d->view->get_clip();
	}
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

void MoveClip::init_data()
{
	if (m_actionType == "copy") {
		m_clip = resources_manager()->get_clip(m_clip->get_id());
		m_clip->set_sheet(m_sheet);
		m_clip->set_track(d->view->get_clip()->get_track());
		m_clip->set_track_start_location(m_clip->get_track_start_location() + TimeRef(d->sv->timeref_scalefactor * 3));
	
		Command::process_command(m_clip->get_track()->add_clip(m_clip, false));
	}

	m_clip->set_snappable(false);
	m_originTrack = m_targetTrack = m_clip->get_track();
	m_originalTrackStartLocation = m_clip->get_track_start_location();
	d->origTrackEndLocation = m_clip->get_track_end_location();
	d->origXPos = cpointer().on_first_input_event_scene_x();
	d->origPos = QPointF(d->origXPos, cpointer().on_first_input_event_scene_y());
	d->sv->start_shuttle(true, true);
	d->bypassjog = false;
}


int MoveClip::begin_hold()
{
	d->sv->stop_follow_play_head();

	init_data();
	
	m_clip->set_as_moving(true);

	return 1;
}


int MoveClip::finish_hold()
{
	m_clip->set_snappable(true);
	m_clip->set_as_moving(false);
	
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
	
	if (m_actionType == "copy") {
		Command::process_command(m_targetTrack->remove_clip(m_clip, false));
		Command::process_command(m_originTrack->remove_clip(m_clip, false));
	} else {
		m_sheet->move_clip(m_targetTrack, m_originTrack, m_clip, m_originalTrackStartLocation);
	}
	
	if (m_originTrack == m_targetTrack &&  m_posDiff == qint64(0) && 
		   ! (m_actionType == "copy" || m_actionType == "move_to_start" || m_actionType == "move_to_end") ) {
		return -1;
	}
	
	return 1;
}


int MoveClip::do_action()
{
	PENTER;
	if (m_actionType == "move") {
		m_sheet->move_clip(m_originTrack, m_targetTrack, m_clip, m_originalTrackStartLocation + m_posDiff);
	}
	else if (m_actionType == "copy") {
		Command::process_command(m_targetTrack->add_clip(m_clip, false));
		m_clip->set_track_start_location(m_originalTrackStartLocation + m_posDiff);
	}
	else if (m_actionType == "move_to_start") {
		move_to_start(false);
	}
	else if (m_actionType == "move_to_end") {
		move_to_end(false);
	}
	
	return 1;
}


int MoveClip::undo_action()
{
	PENTER;
	
	if (m_actionType == "copy") {
		Command::process_command(m_targetTrack->remove_clip(m_clip, false));
	} else {
		m_sheet->move_clip(m_targetTrack, m_originTrack, m_clip, m_originalTrackStartLocation);
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
	if (!m_clip) {
		return 0;
	}
	
	if (d->bypassjog) {
		QPoint diff = d->jogBypassPos - cpointer().pos();
		if (diff.manhattanLength() > 35) {
			d->bypassjog = false;
		} else {
			return 0;
		}
	}
	
	if (d->zoom) {
		d->zoom->jog();
		return 0;
	}
	
	
	d->jogBypassPos = cpointer().pos();
	
	QPointF diffPoint(cpointer().scene_pos() - d->origPos);
	
	d->origPos = cpointer().scene_pos();
	
	TrackView* trackView = d->sv->get_trackview_under(cpointer().scene_pos());
	if (trackView) {
		m_targetTrack = trackView->get_track();
	}

	int newXPos = cpointer().scene_x();

	TimeRef diff_f = TimeRef((cpointer().scene_x() - d->origXPos) * d->sv->timeref_scalefactor);
	if (d->verticalOnly) {
		diff_f = TimeRef();
	}
	
	TimeRef newTrackStartLocation;
	TimeRef newTrackEndLocation = d->origTrackEndLocation + diff_f;

	if (diff_f < TimeRef() && m_originalTrackStartLocation < (-1 * diff_f)) {
		newTrackStartLocation = qint64(0);
	} else {
		newTrackStartLocation = m_originalTrackStartLocation + diff_f;
	}

	if (m_sheet->is_snap_on() && !d->verticalOnly) {
		calculate_snap_diff(newTrackStartLocation, newTrackEndLocation);
	}
	
	m_posDiff = newTrackStartLocation - m_originalTrackStartLocation;

	// store the new position only if the clip was moved, but not if it stuck to a snap position
	if (m_originalTrackStartLocation != newTrackStartLocation) {
		d->origPos.setX(newXPos);
	}

	m_sheet->move_clip(m_clip->get_track(), m_targetTrack, m_clip, newTrackStartLocation);
	
	d->sv->update_shuttle_factor();
	
	cpointer().get_viewport()->set_holdcursor_pos(d->sv->get_clips_viewport()->mapToScene(cpointer().pos()).toPoint());
	cpointer().get_viewport()->set_holdcursor_text(timeref_to_text(newTrackStartLocation, d->sv->timeref_scalefactor));


	return 1;
}


void MoveClip::next_snap_pos(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	d->bypassjog = true;
	TimeRef trackStartLocation = m_sheet->get_snap_list()->next_snap_pos(m_clip->get_track_start_location());
	TimeRef trackEndLocation = m_sheet->get_snap_list()->next_snap_pos(m_clip->get_track_end_location());
	qint64 startdiff = (trackStartLocation - m_clip->get_track_start_location()).universal_frame();
	qint64 enddiff = (trackEndLocation - m_clip->get_track_end_location()).universal_frame();
	qint64 diff = (abs(startdiff) < abs(enddiff)) ? startdiff : enddiff;
	trackStartLocation = m_clip->get_track_start_location() + diff;
	m_posDiff = trackStartLocation - m_originalTrackStartLocation;
	m_clip->set_track_start_location(trackStartLocation);
}

void MoveClip::prev_snap_pos(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	d->bypassjog = true;
	TimeRef trackStartLocation = m_sheet->get_snap_list()->prev_snap_pos(m_clip->get_track_start_location());
	TimeRef trackEndLocation = m_sheet->get_snap_list()->prev_snap_pos(m_clip->get_track_end_location());
	qint64 startdiff = (trackStartLocation - m_clip->get_track_start_location()).universal_frame();
	qint64 enddiff = (trackEndLocation - m_clip->get_track_end_location()).universal_frame();
	qint64 diff = (abs(startdiff) < abs(enddiff)) ? startdiff : enddiff;
	trackStartLocation = m_clip->get_track_start_location() + diff;
	m_posDiff = trackStartLocation - m_originalTrackStartLocation;
	m_clip->set_track_start_location(trackStartLocation);
}

void MoveClip::move_to_start(bool autorepeat)
{
	Q_UNUSED(autorepeat)
	TimeRef location;
	m_clip->set_track_start_location(location);
}

void MoveClip::move_to_end(bool autorepeat)
{
	Q_UNUSED(autorepeat)
	Track *track = m_clip->get_track();
	
	Command::process_command(track->remove_clip(m_clip, false));
	m_clip->set_track_start_location(m_clip->get_sheet()->get_last_location());
	Command::process_command(track->add_clip(m_clip, false));
}

void MoveClip::calculate_snap_diff(TimeRef& leftlocation, TimeRef rightlocation)
{
	// "nframe_t" domain, but must be signed ints because they can become negative
	qint64 snapStartDiff = 0;
	qint64 snapEndDiff = 0;
	qint64 snapDiff = 0;
	
	SnapList* slist = m_sheet->get_snap_list();

	// check if there is anything to snap
	bool start_snapped = false;
	bool end_snapped = false;
	if (slist->is_snap_value(leftlocation)) {
		start_snapped = true;
	}
	
	if (slist->is_snap_value(rightlocation)) {
		end_snapped = true;
	}

	if (start_snapped) {
		snapStartDiff = slist->get_snap_diff(leftlocation);
		snapDiff = snapStartDiff; // in case both ends snapped, change this value later, else leave it
	}

	if (end_snapped) {
		snapEndDiff = slist->get_snap_diff(rightlocation); 
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
	
	leftlocation -= snapDiff;
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
		delete d->zoom;
		d->zoom = 0;
		cpointer().get_viewport()->set_holdcursor(":/cursorHoldLrud");
		d->origXPos -= int((d->origPos - cpointer().scene_pos()).x());
		d->origPos = cpointer().scene_pos();
		d->sv->start_shuttle(true, true);
	}
}

void MoveClip::set_cursor_shape(int useX, int useY)
{
	if (useX && useY) {
		cpointer().get_viewport()->set_holdcursor(":/cursorHoldLrud");
	} else {
		cpointer().get_viewport()->set_holdcursor(":/cursorHoldUd");
	}
}

