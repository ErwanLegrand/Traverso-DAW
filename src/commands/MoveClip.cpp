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
MoveClip::MoveClip(AudioClipView* cv, QVariantList arguments)
	: Command(cv->get_clip(), "")
	, d(new Data)
{
	if (arguments.size()) {
		m_isCopy = arguments.at(0).toBool();
	} else {
		m_isCopy = false;
	}
	
	QString des;
	if (m_isCopy) {
		des = tr("Copy Clip");
		d->xoffset = cv->get_songview()->scalefactor * 3;
	} else {
		des = tr("Move Clip");
	}
	
	setText(des);
	
	d->view = cv;
	d->sv = d->view->get_songview();
	d->song = d->sv->get_song();
	m_targetTrack = 0;
	m_clip = 0;
}


MoveClip::~MoveClip()
{}

void MoveClip::audioclip_added(AudioClip * clip)
{
	QList<QObject* > items = cpointer().get_context_items();
	
	printf("audioclip_added() : Clip has id %lld\n", clip->get_id());
	
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
		
		printf("Found a match!!!!\n");
		return;
	}
	
	printf("MoveClip:: Added new AudioClip, but no AudioClipView available ???\n");
}


void MoveClip::init_data(bool isCopy)
{
	if (isCopy) {
		m_clip = d->newclip;
	} else {
		m_clip = d->view->get_clip();
	}
	
	m_originTrack = m_targetTrack = m_clip->get_track();
	m_originalTrackFirstFrame = m_newInsertFrame = m_clip->get_track_start_frame();
	d->origPos = cpointer().pos();
	d->origXPos = cpointer().x();
	d->hScrollbarValue = d->sv->hscrollbar_value();
	m_clip->set_snappable(false);
	d->sv->start_shuttle(true, true);
	d->origTrackStartFrame = m_clip->get_track_start_frame();
	d->origTrackEndFrame = m_clip->get_track_end_frame();
	d->resync = config().get_property("AudioClip", "SyncDuringDrag", "0").toInt(); 
}


int MoveClip::begin_hold()
{
	if (m_isCopy) {
		d->newclip = resources_manager()->get_clip(d->view->get_clip()->get_id());
		d->newclip->set_song(d->song);
		d->newclip->set_track_start_frame(d->view->get_clip()->get_track_start_frame() + d->xoffset);
		
		printf("Orig Clip has id %lld\n", d->view->get_clip()->get_id());
		printf("Created new Clip with id %lld\n", d->newclip->get_id());
	
		connect(d->view->get_clip()->get_track(), SIGNAL(audioClipAdded(AudioClip*)),
			this, SLOT(audioclip_added(AudioClip*)));
	
		ie().process_command(d->view->get_clip()->get_track()->add_clip(d->newclip, false));
		
		return 1;
	}

	init_data();
	
	return 1;
}


int MoveClip::finish_hold()
{
	m_clip->set_snappable(true);
	d->sv->start_shuttle(false);
	return 1;
}


int MoveClip::prepare_actions()
{
	delete d;
	
	return 1;
}


int MoveClip::do_action()
{
	PENTER;
	if (!m_targetTrack) {
		ie().process_command(m_originTrack->remove_clip(m_clip, false));
		m_targetTrack = (Track*) 0;
	} else {
		ie().process_command(m_originTrack->remove_clip(m_clip, false));
		m_clip->set_track_start_frame(m_newInsertFrame);
		ie().process_command(m_targetTrack->add_clip(m_clip, false));
	}
	
	if (m_isCopy) {
		resources_manager()->undo_remove_clip_from_database(m_clip->get_id());
	}
	
	return 1;
}


int MoveClip::undo_action()
{
	PENTER;
	if (m_targetTrack) {
		ie().process_command(m_targetTrack->remove_clip(m_clip, false));
	}
	
	if (m_isCopy) {
		resources_manager()->remove_clip_from_database(m_clip->get_id());
	} else {
		m_clip->set_track_start_frame(m_originalTrackFirstFrame);
		ie().process_command(m_originTrack->add_clip(m_clip, false));
	}
		
	return 1;
}


int MoveClip::jog()
{
	
	if (! m_clip) {
		return 0;
	}
	
	int scrollbardif = d->hScrollbarValue - d->sv->hscrollbar_value();
	
	QPointF diffPoint(cpointer().pos() - d->origPos);
	QPointF newPos(d->view->pos() + diffPoint);
	
	d->origPos = cpointer().pos();
	
// 	printf("newPos x, y is %f, %f\n", newPos.x(), newPos.y());
	
	TrackView* trackView = d->sv->get_trackview_under(cpointer().scene_pos());
	if (!trackView) {
// 		printf("no trackview returned\n");
	} else if (trackView != d->view->get_trackview()) {
// 		printf("Setting new TrackView!\n");
		d->view->set_trackview(trackView);
		d->view->setParentItem(trackView);
		m_targetTrack = trackView->get_track();
// 		printf("track id is %d\n", m_targetTrack->get_id());
	}
		

	int newXPos = cpointer().x() - scrollbardif;

	SnapList* slist = d->song->get_snap_list();

	// must be signed int because it can be negative
	int diff_f = (cpointer().x() - d->origXPos - scrollbardif) * d->sv->scalefactor;
	long newTrackStartFrame = d->origTrackStartFrame + diff_f;
	nframes_t newTrackEndFrame = d->origTrackEndFrame + diff_f;
// 	printf("newTrackStartFrame is %d\n", newTrackStartFrame);

	// attention: newTrackStartFrame is unsigned, can't check for negative values
	if (newTrackStartFrame < 0) {
		newTrackStartFrame = 0;
	}

	// "nframe_t" domain, but must be signed ints because they can become negative
	int snapStartDiff = 0;
	int snapEndDiff = 0;
	int snapDiff = 0;

	if (d->song->is_snap_on()) {

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
			snapStartDiff = slist->get_snap_diff(newTrackStartFrame) / d->sv->scalefactor;
			snapDiff = snapStartDiff; // in case both ends snapped, change this value later, else leave it
		}

		if (end_snapped) {
			snapEndDiff = slist->get_snap_diff(newTrackEndFrame) / d->sv->scalefactor; 
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

	m_newInsertFrame = newTrackStartFrame - (snapDiff * d->sv->scalefactor);

	// store the new position only if the clip was moved, but not if it stuck to a snap position
	if (d->origTrackStartFrame != m_newInsertFrame) {
		d->origPos.setX(newXPos);
	}

	newPos.setX(m_newInsertFrame / d->sv->scalefactor);	
	newPos.setY(d->view->pos().y());
	
	if (d->resync) {
		m_clip->set_track_start_frame(m_newInsertFrame);
	} else {
		d->view->setPos(newPos);
	}
	
	
	d->sv->update_shuttle_factor();

	return 1;
}


void MoveClip::next_snap_pos(bool autorepeat)
{
	// TODO implement me!
}

void MoveClip::prev_snap_pos(bool autorepeat)
{
	// TODO implement me!
}

// eof
