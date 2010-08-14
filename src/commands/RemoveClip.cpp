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

#include "RemoveClip.h"

#include "AudioClip.h"
#include "AudioClipManager.h"
#include "Sheet.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"
 

AddRemoveClip::AddRemoveClip(AudioClip* clip, int type)
	: TCommand(clip, tr("Remove Clip"))
{
	if (clip->is_selected()) {
		QList<AudioClip*> selectedClips;
		clip->get_sheet()->get_audioclip_manager()->get_selected_clips(selectedClips);
		m_group.set_clips(selectedClips);
		setText(tr("Remove Selected Clips"));
	} else {
		m_group.add_clip(clip);
	}
	m_type = type;
}


int AddRemoveClip::prepare_actions()
{
	return 1;
}


int AddRemoveClip::do_action()
{
	PENTER;
	if (m_type == REMOVE) {
		m_group.remove_all_clips_from_tracks();
	}
	
	if (m_type == ADD) {
		m_group.add_all_clips_to_tracks();
	}
	
	return 1;
}

int AddRemoveClip::undo_action()
{
	PENTER;

	if (m_type == REMOVE) {
		m_group.add_all_clips_to_tracks();
	}
	
	if (m_type == ADD) {
		m_group.remove_all_clips_from_tracks();
	}
	
	return 1;
}

// eof

