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
#include <SongView.h>
#include <TrackView.h>
#include <AudioClipView.h>
#include <ViewPort.h>
#include <ClipsViewPort.h>
#include <QScrollBar>


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
		d->xoffset = cv->get_songview()->scalefactor * 3;
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
	d->sv = d->view->get_songview();
	m_song = d->sv->get_song();
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
		delete d;
	}
}

void MoveClip::audioclip_added(AudioClip * clip)
{
	QList<QObject* > items = cpointer().get_context_items();
	
	foreach(QObject* obj, items) {
		AudioClipView* acv = qobject_cast<AudioClipView*>(obj);
		
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
		m_oldOppositeEdge = m_clip->get_track_start_frame() + m_clip->get_length();
	}
	else if (m_actionType == "anchored_right_edge_move") {
		m_oldOppositeEdge = m_clip->get_track_start_frame();
	}

	m_originTrack = m_targetTrack = m_clip->get_track();
	m_originalTrackFirstFrame = m_clip->get_track_start_frame();
	m_posDiff = 0;
	d->origXPos = cpointer().on_first_input_event_x();
	d->origPos = QPoint(d->origXPos, cpointer().on_first_input_event_y());
	d->hScrollbarValue = d->sv->hscrollbar_value();
	d->sv->start_shuttle(true, true);
	d->origTrackStartFrame = m_clip->get_track_start_frame();
	d->origTrackEndFrame = m_clip->get_track_end_frame();
	d->resync = config().get_property("AudioClip", "SyncDuringDrag", false).toBool();
	d->view->set_dragging(true);
	d->bypassjog = false;
	d->origTrackView = d->view->get_trackview();
}


int MoveClip::begin_hold()
{
	if (m_actionType == "copy") {
		d->newclip = resources_manager()->get_clip(d->view->get_clip()->get_id());
		d->newclip->set_song(m_song);
		d->newclip->set_track(d->view->get_clip()->get_track());
		d->newclip->set_track_start_frame(d->view->get_clip()->get_track_start_frame() + d->xoffset);
		
		printf("Orig Clip has id %lld\n", d->view->get_clip()->get_id());
		printf("Created new Clip with id %lld\n", d->newclip->get_id());
	
		connect(d->view->get_clip()->get_track(), SIGNAL(audioClipAdded(AudioClip*)),
			this, SLOT(audioclip_added(AudioClip*)));
	
		Command::process_command(d->view->get_clip()->get_track()->add_clip(d->newclip, false));
		
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

	if (!m_targetTrack) {
		Command::process_command(m_originTrack->remove_clip(m_clip, false));
	} else {
		m_song->move_clip(m_originTrack, m_targetTrack, m_clip, m_originalTrackFirstFrame + m_posDiff);
	}
	
	if (m_actionType == "copy") {
		resources_manager()->undo_remove_clip_from_database(m_clip->get_id());
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
	if (m_targetTrack) {
		Command::process_command(m_targetTrack->remove_clip(m_clip, false));
	}
	
	if (m_actionType == "copy") {
		resources_manager()->remove_clip_from_database(m_clip->get_id());
	} else {
		m_clip->set_track_start_frame(m_originalTrackFirstFrame);
		Command::process_command(m_originTrack->add_clip(m_clip, false));
	}

	if (m_actionType == "anchored_left_edge_move") {
		m_clip->set_right_edge(m_oldOppositeEdge);
	}
	else if (m_actionType == "anchored_right_edge_move") {
		m_clip->set_track_start_frame(m_oldOppositeEdge - m_posDiff);
		m_clip->set_left_edge(m_oldOppositeEdge);
	}
	
	return 1;
}

void MoveClip::cancel_action()
{
	finish_hold();
	
	if (m_actionType == "copy") {
		Command::process_command(m_originTrack->remove_clip(m_clip, false));
		resources_manager()->remove_clip_from_database(m_clip->get_id());
	} else if (m_actionType == "move") {
		if (d->resync) {
			m_clip->set_track_start_frame(m_originalTrackFirstFrame);
		}
		d->view->set_trackview(d->origTrackView);
		d->view->setParentItem(d->origTrackView);
		d->view->setPos(QPoint(m_originalTrackFirstFrame / d->sv->scalefactor,
				d->origTrackView->get_childview_y_offset()));
	}
}

int MoveClip::jog()
{
	if (! m_clip || d->bypassjog) {
		return 0;
	}
	
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
			d->view->setParentItem(trackView);
			m_targetTrack = trackView->get_track();
		}
	}

	int newXPos = cpointer().x() - scrollbardif;

	// must be signed int because it can be negative
	int diff_f = (cpointer().x() - d->origXPos - scrollbardif) * d->sv->scalefactor;
	long newTrackStartFrame = d->origTrackStartFrame + diff_f;
	nframes_t newTrackEndFrame = d->origTrackEndFrame + diff_f;

	if (newTrackStartFrame < 0) {
		newTrackStartFrame = 0;
	}

	if (m_song->is_snap_on()) {
		calculate_snap_diff(newTrackStartFrame, newTrackEndFrame);
	}
	
	m_posDiff = newTrackStartFrame - m_originalTrackFirstFrame;

	// store the new position only if the clip was moved, but not if it stuck to a snap position
	if (d->origTrackStartFrame != newTrackStartFrame) {
		d->origPos.setX(newXPos);
	}

	if (m_actionType == "anchored_left_edge_move" && !d->resync) {
			m_clip->set_right_edge(m_oldOppositeEdge - m_posDiff);
	}

	if (m_actionType == "anchored_right_edge_move") {
		m_clip->set_left_edge(m_oldOppositeEdge - m_posDiff);
		newPos.setX(m_originalTrackFirstFrame / d->sv->scalefactor);
		newPos.setY(d->view->pos().y());
		d->view->setPos(newPos);
	} else {
		newPos.setX(newTrackStartFrame / d->sv->scalefactor);
		newPos.setY(d->view->pos().y());
		if (d->resync) {
			if (m_clip->get_track_start_frame() != newTrackStartFrame) {
				m_clip->set_track_start_frame(newTrackStartFrame);
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
	long trackStartFrame = m_song->get_snap_list()->next_snap_pos(m_clip->get_track_start_frame());
	nframes_t trackEndFrame = m_song->get_snap_list()->next_snap_pos(m_clip->get_track_end_frame());
	int startdiff = trackStartFrame - m_clip->get_track_start_frame();
	int enddiff = trackEndFrame - m_clip->get_track_end_frame();
	int diff = (abs(startdiff) < abs(enddiff)) ? startdiff : enddiff;
	trackStartFrame = m_clip->get_track_start_frame() + diff;
	m_posDiff = trackStartFrame - m_originalTrackFirstFrame;
	m_clip->set_track_start_frame(trackStartFrame);
}

void MoveClip::prev_snap_pos(bool autorepeat)
{
	Q_UNUSED(autorepeat);
	d->bypassjog = true;
	long trackStartFrame = m_song->get_snap_list()->prev_snap_pos(m_clip->get_track_start_frame());
	nframes_t trackEndFrame = m_song->get_snap_list()->prev_snap_pos(m_clip->get_track_end_frame());
	int startdiff = trackStartFrame - m_clip->get_track_start_frame();
	int enddiff = trackEndFrame - m_clip->get_track_end_frame();
	int diff = (abs(startdiff) < abs(enddiff)) ? startdiff : enddiff;
	trackStartFrame = m_clip->get_track_start_frame() + diff;
	m_posDiff = trackStartFrame - m_originalTrackFirstFrame;
	m_clip->set_track_start_frame(trackStartFrame);
}

void MoveClip::move_to_start(bool autorepeat)
{
	Q_UNUSED(autorepeat)
	m_clip->set_track_start_frame(0);
}

void MoveClip::move_to_end(bool autorepeat)
{
	Q_UNUSED(autorepeat)
	Track *track = m_clip->get_track();
	
	Command::process_command(track->remove_clip(m_clip, false));
	m_clip->set_track_start_frame(m_clip->get_song()->get_last_frame());
	Command::process_command(track->add_clip(m_clip, false));
}

void MoveClip::calculate_snap_diff(long& leftframe, nframes_t rightframe)
{
	// "nframe_t" domain, but must be signed ints because they can become negative
	int snapStartDiff = 0;
	int snapEndDiff = 0;
	int snapDiff = 0;
	
	SnapList* slist = m_song->get_snap_list();

	// check if there is anything to snap
	bool start_snapped = false;
	bool end_snapped = false;
	if (slist->is_snap_value(leftframe) && m_actionType != "anchored_right_edge_move") {
		start_snapped = true;
	}
	
	if (slist->is_snap_value(rightframe) && m_actionType != "anchored_left_edge_move") {
		end_snapped = true;
	}

	if (start_snapped) {
		snapStartDiff = slist->get_snap_diff(leftframe) / d->sv->scalefactor;
		snapDiff = snapStartDiff; // in case both ends snapped, change this value later, else leave it
	}

	if (end_snapped) {
		snapEndDiff = slist->get_snap_diff(rightframe) / d->sv->scalefactor; 
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
	
	leftframe -= snapDiff * d->sv->scalefactor;
}

// eof

