/*
Copyright (C) 2005-2006 Remon Sijrier 

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

$Id: RemoveClip.cpp,v 1.2 2006/06/26 23:57:08 r_sijrier Exp $
*/

#include "RemoveClip.h"

#include <libtraversocore.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


RemoveClip::RemoveClip(AudioClip* clip)
		: Command(clip)
{
	m_clips.append(clip);
}


RemoveClip::RemoveClip( QList< AudioClip * > clips, AudioClipManager* manager)
		: Command(manager)
{
	m_clips = clips;
}

RemoveClip::~RemoveClip()
{}

int RemoveClip::prepare_actions()
{
	return 1;
}

int RemoveClip::do_action()
{
	PENTER;
	
	foreach(AudioClip* clip, m_clips) {
		THREAD_SAVE_REMOVE(clip, clip->get_track(), "remove_clip");
	}
	
	return 1;
}

int RemoveClip::undo_action()
{
	PENTER;
	
	foreach(AudioClip* clip, m_clips) {
		THREAD_SAVE_ADD(clip, clip->get_track(), "add_clip");
	}
	
	return 1;
}


// eof


 
