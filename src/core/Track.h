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


#ifndef TRACK_H
#define TRACK_H

#include <QString>
#include <QDomDocument>
#include <QList>

#include "ContextItem.h"
#include "GainEnvelope.h"
#include "ProcessingData.h"

#include "defines.h"


class Track : public ProcessingData
{
	Q_OBJECT
	Q_CLASSINFO("mute", tr("Mute"))
	Q_CLASSINFO("toggle_arm", tr("Record: On/Off"))
	Q_CLASSINFO("solo", tr("Solo"))
	Q_CLASSINFO("silence_others", tr("Silence other tracks"))

public :
	Track(Sheet* sheet, const QString& name, int height);
	Track(Sheet* sheet, const QDomNode node);
	~Track();

	static const int INITIAL_HEIGHT = 100;

	
	AudioClip* init_recording();
        Command* add_clip(AudioClip* clip, bool historable=true, bool ismove=false);
        Command* remove_clip(AudioClip* clip, bool historable=true, bool ismove=false);
        AudioClip* get_clip_after(const TimeRef& pos);
        AudioClip* get_clip_before(const TimeRef& pos);

	int arm();
	int disarm();

	// Get functions:
	void get_render_range(TimeRef& startlocation, TimeRef& endlocation);
	
	int get_total_clips();
	QDomNode get_state(QDomDocument doc, bool istemplate=false);
	QList<AudioClip*> get_cliplist() const;

	

	// Set functions:
        int set_state( const QDomNode& node );


	bool armed();


	int process(nframes_t nframes);

private :
        APILinkedList 	m_clips;
        int numtakes;

	bool isArmed;

	void set_armed(bool armed);
	void init();

signals:
        void audioClipAdded(AudioClip* clip);
        void audioClipRemoved(AudioClip* clip);
        
        void armedChanged(bool isArmed);

public slots:
	void set_gain(float gain);
	void clip_position_changed(AudioClip* clip);
	
	Command* toggle_arm();
        Command* silence_others();

private slots:
        void private_add_clip(AudioClip* clip);
        void private_remove_clip(AudioClip* clip);

};

#endif

