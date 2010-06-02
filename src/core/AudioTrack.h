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


#ifndef AUDIO_TRACK_H
#define AUDIO_TRACK_H

#include <QString>
#include <QDomDocument>
#include <QList>

#include "ContextItem.h"
#include "GainEnvelope.h"
#include "Track.h"

#include "defines.h"


class AudioTrack : public Track
{
        Q_OBJECT
        Q_CLASSINFO("toggle_arm", tr("Record: On/Off"))
        Q_CLASSINFO("silence_others", tr("Silence other tracks"))

public :
        AudioTrack(Sheet* sheet, const QString& name, int height);
        AudioTrack(Sheet* sheet, const QDomNode node);
        ~AudioTrack();

        AudioClip* init_recording();
        Command* add_clip(AudioClip* clip, bool historable=true, bool ismove=false);
        Command* remove_clip(AudioClip* clip, bool historable=true, bool ismove=false);
        AudioClip* get_clip_after(const TimeRef& pos);
        AudioClip* get_clip_before(const TimeRef& pos);
        QDomNode get_state(QDomDocument doc, bool istemplate=false);
        QList<AudioClip*> get_cliplist() const;
        void get_render_range(TimeRef& startlocation, TimeRef& endlocation);
        int get_total_clips();

        int set_state( const QDomNode& node );

        int arm();
        bool armed();
        int disarm();
        int process(nframes_t nframes);

protected:
        void add_input_bus(AudioBus* bus);

private :
        APILinkedList 	m_clips;
        int             m_numtakes;
        bool            m_isArmed;

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

