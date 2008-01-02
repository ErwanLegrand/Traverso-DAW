/*
    Copyright (C) 2007 Ben Levitt
 
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

#include "MultiMove.h"
#include <ViewPort.h>
#include <SongView.h>
#include <AudioClip.h>
#include <Marker.h>
#include "APILinkedList.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


MultiMove::MultiMove(SongView* sv, bool allTracks)
	: Command(sv->get_song(), (allTracks) ? tr("Fold Sheet") : tr("Fold Track"))
{
	m_sv = sv;
	m_song = sv->get_song();
	m_allTracks = allTracks;
	m_track = 0;
	
	if (!allTracks) {
		QList<QObject* >objs = cpointer().get_context_items();
		foreach(QObject* obj, objs) {
			Track* t = dynamic_cast<Track*>(obj);
			if (t) {
				m_track = t;
				break;
			}
		}
	}
}


MultiMove::~MultiMove()
{
}


int MultiMove::prepare_actions()
{
        PENTER;
	
	if (m_newPos == m_originalPos) {
		// Nothing happened!
		return -1;
	}
	
	
	if (m_markers.size() == 0 && m_clips.size() == 0) {
		// If no markers or clips will be moved, don't start the command
		return -1;
	}
	
        return 1;
}


int MultiMove::begin_hold()
{
	PENTER;
	
	m_newPos = m_lastPos = m_originalPos =
		TimeRef(cpointer().on_first_input_event_scene_x() * m_sv->timeref_scalefactor);
	
	m_sv->stop_follow_play_head();
	m_sv->start_shuttle(true, true);
	
	if (!m_allTracks && m_track == 0) {
		// No track selected
		// Should probably return -1, but that currently throws an error, and
		// returning here means that no markers or clips will get moved, which
		// is correct.
		return 1;
	}
	
	m_selectionStartPos = m_song->get_last_location();
	
	if (!m_track) {
		QList<Marker*>markers = m_song->get_timeline()->get_markers();
		for (int i = 0; i < markers.size(); i++) {
			if (markers[i]->get_when() > m_originalPos) {
				m_markers.append(markers[i]);
				
				if (markers[i]->get_when() < m_selectionStartPos) {
					m_selectionStartPos = markers[i]->get_when();
				}
			}
		}
	}
	
	APILinkedList tracks = m_song->get_tracks();
	
	if (m_track) {
		tracks.clear();
		tracks.append(m_track);
	}
	
	apill_foreach(Track* track, Track, tracks) {
		APILinkedList clips = track->get_cliplist();
		apill_foreach(AudioClip* clip, AudioClip, clips) {
			if (clip->get_track_end_location() > m_originalPos) {
				m_clips.append(clip);
				
				if (clip->get_track_start_location() < m_selectionStartPos) {
					m_selectionStartPos = clip->get_track_start_location();
				}
			}
		}
	}
	
	return 1;
}


int MultiMove::finish_hold()
{
	m_sv->start_shuttle(false);
	undo_action();
	
        return 1;
}


void MultiMove::cancel_action()
{
	finish_hold();
}


int MultiMove::do_action()
{
	for (int i=0; i < m_clips.size(); i++) {
		TimeRef newTrackStartLocation = m_clips[i]->get_track_start_location() + (m_newPos - m_originalPos);
		m_clips[i]->set_track_start_location(newTrackStartLocation);
	}
	for (int i=0; i < m_markers.size(); i++) {
		TimeRef newStartLocation = m_markers[i]->get_when() + (m_newPos - m_originalPos);
		m_markers[i]->set_when(newStartLocation);
	}
	return 1;
}


int MultiMove::undo_action()
{
	for (int i=0; i < m_clips.size(); i++) {
		TimeRef newTrackStartLocation = m_clips[i]->get_track_start_location() - (m_newPos - m_originalPos);
		m_clips[i]->set_track_start_location(newTrackStartLocation);
	}
	for (int i=0; i < m_markers.size(); i++) {
		TimeRef newStartLocation = m_markers[i]->get_when() - (m_newPos - m_originalPos);
		m_markers[i]->set_when(newStartLocation);
	}
	return 1;
}


int MultiMove::jog()
{
	m_newPos = TimeRef(cpointer().scene_x() * m_sv->timeref_scalefactor);
	
	if (m_originalPos > m_selectionStartPos + m_newPos) {
		m_newPos = m_originalPos - m_selectionStartPos;
	}
	
	for (int i=0; i < m_clips.size(); i++) {
		TimeRef newTrackStartLocation = m_clips[i]->get_track_start_location() + (m_newPos - m_lastPos);
		m_clips[i]->set_track_start_location(newTrackStartLocation);
	}
	for (int i=0; i < m_markers.size(); i++) {
		TimeRef newStartLocation = m_markers[i]->get_when() + (m_newPos - m_lastPos);
		m_markers[i]->set_when(newStartLocation);
	}
	
	m_lastPos = m_newPos;
	m_sv->update_shuttle_factor();
	
	return 1;
}

// eof

