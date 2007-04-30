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

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"
 

RemoveClip::RemoveClip(AudioClip* clip)
	: Command(clip, tr("Remove Clip"))
{
	m_clip = clip;
	m_track = m_clip->get_track();
}


int RemoveClip::prepare_actions()
{
	return 1;
}


int RemoveClip::do_action()
{
	PENTER;
	Command::process_command(m_track->remove_clip(m_clip, false));
	return 1;
}

int RemoveClip::undo_action()
{
	PENTER;

	Command::process_command(m_track->add_clip(m_clip, false));
	return 1;
}

// eof

