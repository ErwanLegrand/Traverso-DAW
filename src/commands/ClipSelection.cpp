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


#include "ClipSelection.h"
#include "AudioClipManager.h"
#include <AudioClip.h>
#include "ContextPointer.h"
#include <Sheet.h>
#include <Utils.h>

#include "Debugger.h"

ClipSelection::ClipSelection(AudioClip* clip, QVariantList args)
	: Command("")
{
	QString slot = args.at(0).toString();
	
	if (slot == "remove_from_selection") {
		 setText(tr("Selection: Remove Clip"));
	} else if (slot == "add_to_selection") {
		 setText(tr("Selection: Add Clip"));
	} else if (slot == "select_clip") {
		 setText(tr("Select Clip"));
	}
	
	m_clips.append( clip );
	m_slot = qstrdup(QS_C(slot));
	m_acmanager = clip->get_sheet()->get_audioclip_manager();
}

ClipSelection::ClipSelection( QList< AudioClip * > clips, AudioClipManager * manager, const char * slot, const QString& des )
	: Command("")
{
	m_clips = clips;
	m_slot = slot;
	m_acmanager = manager;
}

ClipSelection::~ClipSelection()
{}

int ClipSelection::begin_hold()
{
	return -1;
}

int ClipSelection::finish_hold()
{
	return -1;
}

int ClipSelection::prepare_actions()
{
	return 1;
}

int ClipSelection::do_action()
{
	foreach(AudioClip* clip, m_clips) {
		if ( ! QMetaObject::invokeMethod(m_acmanager, m_slot, Q_ARG(AudioClip*, clip))) {
			PERROR("AudioClip::%s failed for %s", m_slot, QS_C(clip->get_name()));
		}
	}
	
	return 1;
}

int ClipSelection::undo_action()
{
	return 1;
}

void ClipSelection::cancel_action()
{
	undo_action();
}

int ClipSelection::jog()
{
	return -1;
}
