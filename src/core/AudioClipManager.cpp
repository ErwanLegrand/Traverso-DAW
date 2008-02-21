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
 
#include "AudioClipManager.h"

#include "Sheet.h"
#include "AudioClip.h"
#include "ResourcesManager.h"
#include "ProjectManager.h"
#include "Track.h"
#include "commands.h"
#include "SnapList.h"
#include "Utils.h"
#include "Debugger.h"

AudioClipManager::AudioClipManager( Sheet* sheet )
	: ContextItem(sheet)
{
	PENTERCONS;
	m_sheet = sheet;
	set_history_stack( m_sheet->get_history_stack() );
	lastLocation = TimeRef();
}

AudioClipManager::~ AudioClipManager( )
{
	PENTERDES;
}

void AudioClipManager::add_clip( AudioClip * clip )
{
	PENTER;
	if (m_clips.contains(clip)) {
		PERROR("Trying to add clip %s, but it's already in my list!!", QS_C(clip->get_name()));
		return;
	}
	
	m_clips.append( clip );
	
	connect(clip, SIGNAL(positionChanged()), this, SLOT(update_last_frame()));
	
	m_sheet->get_snap_list()->mark_dirty();
	update_last_frame();
	resources_manager()->mark_clip_added(clip);
}

void AudioClipManager::remove_clip( AudioClip * clip )
{
	PENTER;
	if (m_clips.removeAll(clip) == 0) {
		PERROR("Clip %s was not in my list, couldn't remove it!!", QS_C(clip->get_name()));
		return;
	}
	
	m_sheet->get_snap_list()->mark_dirty();
	update_last_frame();
	resources_manager()->mark_clip_removed(clip);
}


void AudioClipManager::update_last_frame( )
{
	PENTER;
	
	lastLocation = TimeRef();
	
	foreach(AudioClip* clip, m_clips) {
		if (clip->get_track_end_location() >= lastLocation)
			lastLocation = clip->get_track_end_location();
	}
	
	emit m_sheet->lastFramePositionChanged();
}

const TimeRef& AudioClipManager::get_last_location() const
{
	return lastLocation;
}

void AudioClipManager::get_selected_clips_state( QList< AudioClip * > & list )
{
	foreach(AudioClip* clip, clipselection) {
		list.append(clip);
	}
}

void AudioClipManager::set_selected_clips_state( QList< AudioClip * > & list )
{
	clipselection.clear();
	
	foreach(AudioClip* clip, m_clips) {
		clip->set_selected(false);
	}
	
	foreach(AudioClip* clip, list) {
		add_to_selection( clip );
	}
}

void AudioClipManager::remove_from_selection( AudioClip * clip )
{
	int index = clipselection.indexOf( clip );
	
	if (index >= 0) {
		clipselection.takeAt( index );
		clip->set_selected( false );
	}
}

void AudioClipManager::add_to_selection( AudioClip * clip )
{
	if ( ! clipselection.contains( clip ) ) {
		clipselection.append( clip );
		clip->set_selected( true );
	}
}

void AudioClipManager::toggle_selected( AudioClip * clip )
{
	PENTER;
	
	if (clip->is_selected()) {
		remove_from_selection(clip);
	} else {
		add_to_selection(clip);
	}
}

void AudioClipManager::select_clip(AudioClip* clip)
{
	PENTER;
	
	foreach(AudioClip* c, m_clips) {
		if (c == clip) {
			continue;
		}
		remove_from_selection( c );
	}
	

	if (clip->is_selected()) {
		remove_from_selection(clip);
	} else {
		add_to_selection(clip);
	}
	
}

QList<AudioClip* > AudioClipManager::get_clip_list() const
{
	return m_clips;
}

/****************************** SLOTS ***************************/
/****************************************************************/


Command* AudioClipManager::select_all_clips()
{
	PENTER;
	
	return new ClipSelection(m_clips, this, "add_to_selection", tr("Selection: Add Clip"));
}

Command* AudioClipManager::deselect_all_clips()
{
	PENTER;
	
	return new ClipSelection(m_clips, this, "remove_from_selection", tr("Selection: Remove Clip"));
}

Command* AudioClipManager::invert_clip_selection()
{
	PENTER;
	
	return new ClipSelection(m_clips, this, "toggle_selected", tr("Selection: Invert"));
}

Command* AudioClipManager::delete_selected_clips()
{
	PENTER;
	CommandGroup* group = new CommandGroup(this, tr("Remove Clip(s)"));
	foreach(AudioClip* clip, clipselection) {
		group->add_command(clip->get_track()->remove_clip(clip));
	}
	return group;
}

//eof
