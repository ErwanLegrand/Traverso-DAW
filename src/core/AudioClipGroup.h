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

#ifndef AUDIO_CLIP_GROUP_H
#define AUDIO_CLIP_GROUP_H

#include "defines.h"

#include "AudioClip.h"

class AudioClipGroup
{
public:
	AudioClipGroup(){};
	AudioClipGroup(QList<AudioClip*> clips);
	
	void add_clip(AudioClip* clip);
	void set_clips(QList<AudioClip*> clips);
	void move_to(int trackIndex, TimeRef location);
	
	void set_snappable(bool snap);
	void set_as_moving(bool move);
	void check_valid_track_index_delta(int& delta);
	
	QList<AudioClip*> copy_clips();
	void add_all_clips_to_tracks();
	void remove_all_clips_from_tracks();
	
	int get_size() const {return m_clips.size();}
	int get_track_index() const {return m_topTrackIndex;}
	
	bool is_locked() const;
	
	TimeRef get_track_start_location() const {return m_trackStartLocation;}
	TimeRef get_track_end_location() const {return m_trackEndLocation;}
	TimeRef get_length() const {return m_trackEndLocation - m_trackStartLocation;}
	
private:
	QList<AudioClip*> m_clips;
	TimeRef m_trackEndLocation;
	TimeRef m_trackStartLocation;
	int	m_topTrackIndex;
	int	m_bottomTrackIndex;
	
	void update_state();
};

#endif
