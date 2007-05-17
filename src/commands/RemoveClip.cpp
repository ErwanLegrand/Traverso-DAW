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

#include <AudioClip.h>
#include <Track.h>
#include "ResourcesManager.h"
#include "ProjectManager.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"
 

AddRemoveClip::AddRemoveClip(AudioClip* clip, int type)
	: Command(clip, tr("Remove Clip"))
{
	m_clip = clip;
	m_track = m_clip->get_track();
	m_type = type;
	m_removeFromDataBase = false;
}


int AddRemoveClip::prepare_actions()
{
	return 1;
}


int AddRemoveClip::do_action()
{
	PENTER;
	if (m_type == REMOVE) {
		Command::process_command(m_track->remove_clip(m_clip, false));
	}
	
	if (m_type == ADD) {
		Command::process_command(m_track->add_clip(m_clip, false));
	}
	
	return 1;
}

int AddRemoveClip::undo_action()
{
	PENTER;

	if (m_type == REMOVE) {
		Command::process_command(m_track->add_clip(m_clip, false));
	}
	
	if (m_type == ADD) {
		Command::process_command(m_track->remove_clip(m_clip, false));
	}
	
	return 1;
}

// eof

