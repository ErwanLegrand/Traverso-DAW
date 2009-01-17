/*
Copyright (C) 2008 Remon Sijrier 

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

#include "AudioClipGroup.h"

#include "AudioClip.h"
#include "AudioClipManager.h"
#include "Command.h"
#include "ProjectManager.h"
#include "ResourcesManager.h"
#include "Sheet.h"
#include "Track.h"
#include <limits.h>

#include "Debugger.h"

AudioClipGroup::AudioClipGroup(QList< AudioClip * > clips)
{
	m_clips = clips;
	update_state();
}

void AudioClipGroup::add_clip(AudioClip * clip)
{
	m_clips.append(clip);
	update_state();
}

void AudioClipGroup::set_clips(QList< AudioClip * > clips)
{
	m_clips = clips;
	update_state();
}

void AudioClipGroup::move_to(int trackIndex, TimeRef location)
{
	int trackIndexDelta = trackIndex - m_topTrackIndex;
	
	foreach(AudioClip* clip, m_clips) {
		if (trackIndexDelta != 0) {
			Track* track = clip->get_sheet()->get_track_for_index(clip->get_track()->get_sort_index() + trackIndexDelta);
			if (track) {
				Command::process_command(clip->get_track()->remove_clip(clip, false, true));
				Command::process_command(track->add_clip(clip, false, true));
			}
		}
		
		TimeRef offset = clip->get_track_start_location() - m_trackStartLocation;
		clip->set_track_start_location(location + offset);
	}
	
	update_state();
}

void AudioClipGroup::update_state()
{
	if (m_clips.isEmpty()) {
		return;
	}
		
	m_trackStartLocation = LLONG_MAX;
	m_trackEndLocation = TimeRef();
	
	m_topTrackIndex = INT_MAX;
	m_bottomTrackIndex = 0;
	
	foreach(AudioClip* clip, m_clips) {
		int index = clip->get_track()->get_sort_index();
		if (index < m_topTrackIndex) {
			m_topTrackIndex = index;
		}
		if (index > m_bottomTrackIndex) {
			m_bottomTrackIndex = index;
		}
		if (m_trackStartLocation > clip->get_track_start_location()) {
			m_trackStartLocation = clip->get_track_start_location();
		}
		if (m_trackEndLocation < clip->get_track_end_location()) {
			m_trackEndLocation = clip->get_track_end_location();
		}
	}
}

void AudioClipGroup::set_snappable(bool snap)
{
	foreach(AudioClip* clip, m_clips) {
		clip->set_snappable(snap);
	}
}

void AudioClipGroup::set_as_moving(bool move)
{
	foreach(AudioClip* clip, m_clips) {
		clip->set_as_moving(move);
	}
}

QList<AudioClip*> AudioClipGroup::copy_clips()
{
	QList<AudioClip*> newclips;
	
	foreach(AudioClip* clip, m_clips) {
		AudioClip* newclip = resources_manager()->get_clip(clip->get_id());
		newclip->set_sheet(clip->get_sheet());
		newclip->set_track(clip->get_track());
		newclip->set_track_start_location(clip->get_track_start_location());
		newclips.append(newclip);
	}
	
	return newclips;
}

void AudioClipGroup::add_all_clips_to_tracks()
{
	foreach(AudioClip* clip, m_clips) {
		Command::process_command(clip->get_track()->add_clip(clip, false));
	}
}

void AudioClipGroup::remove_all_clips_from_tracks()
{
	foreach(AudioClip* clip, m_clips) {
		Command::process_command(clip->get_track()->remove_clip(clip, false));
	}
}

void AudioClipGroup::check_valid_track_index_delta(int & delta)
{
	if (m_clips.isEmpty()) {
		return;
	}
	
	int allowedDeltaPlus = (m_clips.first()->get_sheet()->get_numtracks() - 1) - m_bottomTrackIndex;
	int allowedDeltaMin  = -m_topTrackIndex;
	
	if (delta > allowedDeltaPlus) {
		delta = allowedDeltaPlus;
	}
	
	if (delta < allowedDeltaMin) {
		delta = allowedDeltaMin;
	}
}

bool AudioClipGroup::is_locked() const
{
	foreach(AudioClip* clip, m_clips) {
		if (clip->is_locked()) {
			return true;
		}
	}
	return false;
}

void AudioClipGroup::select_clips(bool select)
{
	foreach(AudioClip* clip, m_clips) {
		if (select) {
			clip->get_sheet()->get_audioclip_manager()->add_to_selection(clip);
		} else {
			clip->get_sheet()->get_audioclip_manager()->remove_from_selection(clip);
		}
	}
}
