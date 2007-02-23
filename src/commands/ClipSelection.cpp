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

$Id: ClipSelection.cpp,v 1.4 2007/02/23 14:00:35 r_sijrier Exp $
*/


#include "ClipSelection.h"
#include "AudioClipManager.h"
#include <AudioClip.h>
#include <Song.h>

#include "Debugger.h"


ClipSelection::ClipSelection(AudioClip* clip, const char* slot, const QString& des)
	: Command(clip, des)
{
	m_clips.append( clip );
	m_slot = slot;
	m_acmanager = clip->get_song()->get_audioclip_manager();
}

ClipSelection::ClipSelection( QList< AudioClip * > clips, AudioClipManager * manager, const char * slot, const QString& des )
		: Command(manager, des)
{
	m_clips = clips;
	m_slot = slot;
	m_acmanager = manager;
}

ClipSelection::~ClipSelection()
{}

int ClipSelection::prepare_actions()
{
	m_acmanager->get_selected_clips_state( selectedClips );
	
	return 1;
}

int ClipSelection::do_action()
{
	foreach(AudioClip* clip, m_clips) {
		QMetaObject::invokeMethod(m_acmanager, m_slot, Q_ARG(AudioClip*, clip));
	}
	
	return 1;
}

int ClipSelection::undo_action()
{
	m_acmanager->set_selected_clips_state( selectedClips );
	
	return 1;
}

// eof

