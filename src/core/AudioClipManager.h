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

#ifndef AUDIOCLIPMANAGER_H
#define AUDIOCLIPMANAGER_H

#include "ContextItem.h"
#include "defines.h"

#include <QDomNode>
#include <QList>

class AudioClip;
class Sheet;

class AudioClipManager : public ContextItem
{
	Q_OBJECT

public:
	AudioClipManager(Sheet* sheet);
	~AudioClipManager();

	QDomNode get_state(QDomDocument doc, bool istemplate=false);
	int set_state( const QDomNode & node );

	
	bool is_clip_in_selection(AudioClip* clip);
	
	void get_selected_clips(QList<AudioClip*> & list);

	QList<AudioClip* > get_clip_list() const;

	TimeRef get_last_location() const;

private:
	QList<AudioClip* >		m_clips;
	QList<AudioClip* >		m_clipselection;
	Sheet*				m_sheet;
	
	TimeRef 			m_lastLocation;

public slots:
	void add_clip(AudioClip* clip);
	void remove_clip(AudioClip* clip);
	void select_clip(AudioClip* clip);
	void toggle_selected(AudioClip* clip);
	
	void remove_from_selection(AudioClip* clip);
	void add_to_selection(AudioClip* clip);
	void update_last_frame();

	TCommand* select_all_clips();
	TCommand* invert_clip_selection();
};

#endif

//eof
