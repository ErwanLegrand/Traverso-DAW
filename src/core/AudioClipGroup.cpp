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
#include "Command.h"
#include "ProjectManager.h"
#include "ResourcesManager.h"
#include "Sheet.h"
#include "Track.h"

#include "Debugger.h"

static bool smallerClip(const AudioClip* left, const AudioClip* right )
{
	return left->get_track_start_location() < right->get_track_start_location();
}

AudioClipGroup::AudioClipGroup()
{
}

AudioClipGroup::AudioClipGroup(QList< AudioClip * > clips)
{
	m_clips = clips;
	update_track_start_and_end_locations();
}

void AudioClipGroup::add_clip(AudioClip * clip)
{
	m_clips.append(clip);
	update_track_start_and_end_locations();
}

void AudioClipGroup::set_clips(QList< AudioClip * > clips)
{
	m_clips = clips;
	update_track_start_and_end_locations();
}

void AudioClipGroup::move_to(int trackIndex, TimeRef location)
{
	foreach(AudioClip* clip, m_clips) {
		
		if (clip->get_track()->get_sort_index() != trackIndex) {
			Track* track = clip->get_sheet()->get_track_for_index(trackIndex);
			if (track) {
				Command::process_command(clip->get_track()->remove_clip(clip, false, true));
				Command::process_command(track->add_clip(clip, false, true));
			}
		}
		
		TimeRef offset = clip->get_track_start_location() - m_trackStartLocation;
		clip->set_track_start_location(location + offset);
	}
	
	if (m_clips.size()) {
		m_trackStartLocation = m_clips.first()->get_track_start_location();
		m_trackEndLocation = m_clips.last()->get_track_end_location();
	}
}

void AudioClipGroup::update_track_start_and_end_locations()
{
	qSort(m_clips.begin(), m_clips.end(), smallerClip);

	if (m_clips.size()) {
		m_trackStartLocation = m_clips.first()->get_track_start_location();
		m_trackEndLocation = m_clips.last()->get_track_end_location();
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

