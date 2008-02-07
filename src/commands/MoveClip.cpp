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

#include <libtraversocore.h>

#include "MoveClip.h"
#include "SnapList.h"
#include <SheetView.h>
#include <TrackView.h>
#include <AudioClipView.h>
#include <ViewPort.h>
#include <ClipsViewPort.h>
#include <QScrollBar>
#include "Zoom.h"


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
  *	\class MoveClip
	\brief A Command class for Dragging or Copy-dragging an AudioClip
	 
 */


/**
 * 	Creates  a Move Clip or Copy Clip Command object.
	
	Use the first entry in arguments to set the command to be
	of type MoveClip (arguments.at(0) == false), or of type CopyClip
	argument (arguments.at(0) == true)
	
	arguments is set in the keymap file, example: 
	
	\code 
	<Object objectname="AudioClipView" mousehint="LRUD" pluginname="TraversoCommands" commandname="MoveClip"  arguments="false" />
	\endcode
 
 
 * @param cv The AudioClipView that is to be dragged/copied.
 * @param arguments The first entry in the list is used to detect if it is a copy or drag Command
 */
MoveClip::MoveClip(AudioClipView* cv, QString type)
	: Command(cv->get_clip(), "")
	, d(new Data)
{
	m_actionType = type;
	
	QString des;
	if (m_actionType == "copy") {
		des = tr("Copy Clip");
		d->xoffset = TimeRef(cv->get_sheetview()->timeref_scalefactor * 3);
	} else if (m_actionType == "move") {
		des = tr("Move Clip");
	} else if (m_actionType == "anchored_left_edge_move" ||
		m_actionType == "anchored_right_edge_move") {
		des = tr("Move Anchored Edge");
	} else if (m_actionType == "move_to_start") {
		des = tr("Move Clip To Start");
	} else if (m_actionType == "move_to_end") {
		des = tr("Move Clip To End");
	}
	
	setText(des);
	
	d->view = cv;
	d->sv = d->view->get_sheetview();
	d->zoom = 0;
	m_sheet = d->sv->get_sheet();
	m_targetTrack = 0;

	if (m_actionType == "move_to_start" ||
	    m_actionType == "move_to_end") {
		init_data();
	} else {
		m_clip = 0;
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

void MoveClip::audioclip_added(AudioClip * clip)
{
	Q_UNUSED(clip);
	
	QList<AudioClipView* >* clipviews = d->view->get_trackview()->get_clipviews();
	
	for (int i = 0; i < clipviews->size(); ++i) {
		AudioClipView* acv = clipviews->at(i);
		
		if ( ! acv) {
			continue;
		}
		
		if ( ! (acv->get_clip()->get_id() == d->newclip->get_id()) ) {
			continue;
		}
		
		d->view = acv;
		init_data(true);

		disconnect(d->view->get_clip()->get_track(), SIGNAL(audioClipAdded(AudioClip*)),
			this, SLOT(audioclip_added(AudioClip*)));
		
		return;
	}
}


void MoveClip::init_data(bool isCopy)
{
	if (isCopy) {
		m_clip = d->newclip;
	} else {
		m_clip = d->view->get_clip();
	}

	if (m_actionType == "anchored_left_edge_move") {
		m_oldOppositeEdge = m_clip->get_track_start_location() + m_clip->get_length();
	}
	else if (m_actionType == "anchored_right_edge_move") {
		m_oldOppositeEdge = m_clip->get_track_start_location();
	}

	m_originTrack = m_targetTrack = m_clip->get_track();
	m_originalTrackStartLocation = m_clip->get_track_start_location();
	m_posDiff = TimeRef();
	d->origXPos = cpointer().on_first_input_event_x();
	d->origPos = QPoint(d->origXPos, cpointer().on_first_input_event_y());
	d->hScrollbarValue = d->sv->hscrollbar_value();
	d->sv->start_shuttle(true, true);
	d->origTrackStartLocation = m_clip->get_track_start_location();
	d->origTrackEndLocation = m_clip->get_track_end_location();
	d->resync = config().get_property("AudioClip", "SyncDuringDrag", false).toBool();
	d->view->set_dragging(true);
	d->bypassjog = false;
	d->origTrackView = d->view->get_trackview();
}


int MoveClip::begin_hold()
{
	d->sv->stop_follow_play_head();
	if (m_actionType == "copy") {
		d->newclip = resources_manager()->get_clip(d->view->get_clip()->get_id());
		d->newclip->set_sheet(m_sheet);
		d->newclip->set_track(d->view->get_clip()->get_track());
		d->newclip->set_track_start_location(d->view->get_clip()->get_track_start_location() + d->xoffset);
		
		connect(d->view->get_clip()->get_track(), SIGNAL(audioClipAdded(AudioClip*)),
			this, SLOT(audioclip_added(AudioClip*)));
	
		Command::process_command(d->view->get_clip()->get_track()->add_clip(d->newclip, false));
		d->newclip->set_snappable(false);
		
		return 1;
	}

	init_data();
	m_clip->set_snappable(false);

	return 1;
}


int MoveClip::finish_hold()
{
	m_clip->set_snappable(true);
	d->sv->start_shuttle(false);
	d->view->set_dragging(false);

	return 1;
}


int MoveClip::prepare_actions()
{
	delete d;
	d = 0;
	
	if (m_actionType == "anchored_right_edge_move") {
		m_clip->set_left_edge(m_oldOppositeEdge);
	}
	
	if (m_actionType == "copy") {
		Command::process_command(m_targetTrack->remove_clip(m_clip, false));
		Command::process_command(m_originTrack->remove_clip(m_clip, false));
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
	if (m_actionType == "move_to_start") {
		move_to_start(false);
		return 1;
	}
	else if (m_actionType == "move_to_end") {
		move_to_end(false);
		return 1;
	}

	if (m_actionType == "copy") {
		Command::process_command(m_targetTrack->add_clip(m_clip, false));
		m_clip->set_track_start_location(m_originalTrackStartLocation + m_posDiff);
	} else {
		m_sheet->move_clip(m_originTrack, m_targetTrack, m_clip, m_originalTrackStartLocation + m_posDiff);
	}
	
	if (m_actionType == "anchored_left_edge_move") {
		m_clip->set_right_edge(m_oldOppositeEdge);
	}
	else if (m_actionType == "anchored_right_edge_move") {
		m_clip->set_left_edge(m_oldOppositeEdge);
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

	if (m_actionType == "anchored_left_edge_move") {
		m_clip->set_right_edge(m_oldOppositeEdge);
	}
	else if (m_actionType == "anchored_right_edge_move") {
		m_clip->set_track_start_location(m_oldOppositeEdge - m_posDiff);
		m_clip->set_left_edge(m_oldOppositeEdge);
	}
	
	return 1;
}

void MoveClip::cancel_action()
{
	finish_hold();
	
	if (m_actionType == "copy") {
		Command::process_command(m_originTrack->remove_clip(m_clip, false));
	} else if (m_actionType == "move") {
		if (d->resync) {
			m_clip->set_track_start_location(m_originalTrackStartLocation);
		}
		d->view->set_trackview(d->origTrackView);
		d->view->setPos(QPoint((int)(m_originalTrackStartLocation / d->sv->timeref_scalefactor),
				d->origTrackView->get_childview_y_offset()));
	}
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
	
	int scrollbardif = d->hScrollbarValue - d->sv->hscrollbar_value();
	
	QPointF diffPoint(cpointer().pos() - d->origPos);
	QPointF newPos(d->view->pos() + diffPoint);
	
	d->origPos = cpointer().pos();
	
	if (m_actionType != "anchored_left_edge_move" && m_actionType != "anchored_right_edge_move") {
		TrackView* trackView = d->sv->get_trackview_under(cpointer().scene_pos());
		if (!trackView) {
	// 		printf("no trackview returned\n");
		} else if (trackView != d->view->get_trackview()) {
			d->view->set_trackview(trackView);
			m_targetTrack = trackView->get_track();
		}
	}

	int newXPos = cpointer().x() - scrollbardif;

	TimeRef diff_f = TimeRef((cpointer().x() - d->origXPos - scrollbardif) * d->sv->timeref_scalefactor);
	TimeRef newTrackStartLocation;
	TimeRef newTrackEndLocation = d->origTrackEndLocation + diff_f;

	if (diff_f < TimeRef() && d->origTrackStartLocation < (-1 * diff_f)) {
		newTrackStartLocation = qint64(0);
	} else {
		newTrackStartLocation = d->origTrackStartLocation + diff_f;
	}

	if (m_sheet->is_snap_on()) {
		calculate_snap_diff(newTrackStartLocation, newTrackEndLocation);
	}
	
	m_posDiff = newTrackStartLocation - m_originalTrackStartLocation;

	// store the new position only if the clip was moved, but not if it stuck to a snap position
	if (d->origTrackStartLocation != newTrackStartLocation) {
		d->origPos.setX(newXPos);
	}

	if (m_actionType == "anchored_left_edge_move" && !d->resync) {
			m_clip->set_right_edge(m_oldOppositeEdge - m_posDiff);
	}

	if (m_actionType == "anchored_right_edge_move") {
		m_clip->set_left_edge(m_oldOppositeEdge - m_posDiff);
		newPos.setX(m_originalTrackStartLocation / d->sv->timeref_scalefactor);
		newPos.setY(d->view->pos().y());
		d->view->setPos(newPos);
	} else {
		newPos.setX(newTrackStartLocation / d->sv->timeref_scalefactor);
		newPos.setY(d->view->pos().y());
		if (d->resync) {
			if (m_clip->get_track_start_location() != newTrackStartLocation) {
				m_clip->set_track_start_location(newTrackStartLocation);
			}
			if (m_actionType == "anchored_left_edge_move") {
				m_clip->set_right_edge(m_oldOppositeEdge);
			}
		} else {
			d->view->setPos(newPos);
		}
	}
	
	d->sv->update_shuttle_factor();

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
	if (slist->is_snap_value(leftlocation) && m_actionType != "anchored_right_edge_move") {
		start_snapped = true;
	}
	
	if (slist->is_snap_value(rightlocation) && m_actionType != "anchored_left_edge_move") {
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
	if (!d->zoom) {
		d->zoom = new Zoom(d->sv, QList<QVariant>() << "HJogZoom" << "1.2" << "0.2");
		d->zoom->begin_hold();
		cpointer().get_viewport()->set_holdcursor(":/cursorZoomHorizontal");
		d->sv->start_shuttle(false);
	} else {
		d->zoom->finish_hold();
		delete d->zoom;
		d->zoom = 0;
		cpointer().get_viewport()->set_holdcursor(":/cursorHoldLrud");
		d->sv->start_shuttle(true, true);
	}
}

