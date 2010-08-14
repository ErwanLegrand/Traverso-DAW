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
	m_lastLocation = TimeRef();
}

AudioClipManager::~ AudioClipManager( )
{
	PENTERDES;
}


QDomNode AudioClipManager::get_state(QDomDocument doc, bool istemplate)
{
	QDomElement managerNode = doc.createElement("ClipManager");
	QDomElement globalSelection = doc.createElement("GlobalSelection");
	
	QStringList selectedClips;
	foreach(AudioClip* clip, m_clipselection) {
		selectedClips << QString::number(clip->get_id());
	}
	
	globalSelection.setAttribute("clips",  selectedClips.join(";"));
	
	managerNode.appendChild(globalSelection);
	
	return managerNode;
}

int AudioClipManager::set_state(const QDomNode & node)
{
	QDomElement e = node.firstChildElement("GlobalSelection");
	
	QStringList selectionList = e.attribute("clips", "").split(";");
	
	for (int i=0; i<selectionList.size(); ++i) {
		qint64 id = selectionList.at(i).toLongLong();
		foreach(AudioClip* clip, m_clips) {
			if (clip->get_id() == id) {
				add_to_selection(clip);
			}
		}
	}
	
	return 1;

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
	
	remove_from_selection(clip);
	
	m_sheet->get_snap_list()->mark_dirty();
	update_last_frame();
	resources_manager()->mark_clip_removed(clip);
}


void AudioClipManager::update_last_frame( )
{
        PENTER3;
	
	m_lastLocation = TimeRef();
	
	foreach(AudioClip* clip, m_clips) {
		if (clip->get_track_end_location() >= m_lastLocation)
			m_lastLocation = clip->get_track_end_location();
	}
	
	emit m_sheet->lastFramePositionChanged();
}

TimeRef AudioClipManager::get_last_location() const
{
	return m_lastLocation;
}

void AudioClipManager::get_selected_clips( QList< AudioClip * > & list )
{
	foreach(AudioClip* clip, m_clipselection) {
		list.append(clip);
	}
}

void AudioClipManager::remove_from_selection( AudioClip * clip )
{
	if (m_clipselection.contains(clip)) {
		m_clipselection.removeAll(clip);
		clip->set_selected( false );
	}
}

void AudioClipManager::add_to_selection( AudioClip * clip )
{
	if ( ! m_clipselection.contains( clip ) ) {
		m_clipselection.append( clip );
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
		remove_from_selection(c);
	}
	
	add_to_selection(clip);
}

QList<AudioClip* > AudioClipManager::get_clip_list() const
{
	return m_clips;
}

bool AudioClipManager::is_clip_in_selection(AudioClip* clip)
{
	return m_clipselection.contains(clip);
}


/****************************** SLOTS ***************************/
/****************************************************************/


TCommand* AudioClipManager::select_all_clips()
{
	PENTER;
	
	if (m_clipselection.size()) {
		return new ClipSelection(m_clips, this, "remove_from_selection", tr("Selection: Remove Clip"));
	}
		
	return new ClipSelection(m_clips, this, "add_to_selection", tr("Selection: Add Clip"));
}

TCommand* AudioClipManager::invert_clip_selection()
{
	PENTER;
	
	return new ClipSelection(m_clips, this, "toggle_selected", tr("Selection: Invert"));
}

