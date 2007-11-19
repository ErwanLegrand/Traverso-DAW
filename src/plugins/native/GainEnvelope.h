/*
Copyright (C) 2007 Remon Sijrier

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


#ifndef GAIN_ENVELOPE_H
#define GAIN_ENVELOPE_H

#include "Plugin.h"
#include "Curve.h"
#include "Mixer.h"

class Song;

class GainEnvelope : public Plugin
{
	Q_OBJECT
	
public:
	GainEnvelope(Song* song);
	~GainEnvelope(){};

	QDomNode get_state(QDomDocument doc);
	int set_state(const QDomNode & node );
	void process(AudioBus* bus, unsigned long nframes);
	void process_gain(audio_sample_t** buffer, const TimeRef& startlocation, const TimeRef& endlocation, nframes_t nframes, uint channels);
	
	void set_song(Song* song);
	void set_gain(float gain) {m_gain = gain;}
	
	float get_gain() const {return m_gain;}
	Curve* get_curve() const;
	QString get_name();
	
private:
	float m_gain;
};


inline void GainEnvelope::process_gain(audio_sample_t** buffer, const TimeRef& startlocation, const TimeRef& endlocation, nframes_t nframes, uint channels)
{
	PluginControlPort* port = m_controlPorts.at(0);
	
	if (port->use_automation()) {
		port->get_curve()->process(buffer, startlocation, endlocation, nframes, channels, m_gain);
	} else {
		for (uint chan=0; chan<channels; ++chan) {
			Mixer::apply_gain_to_buffer(buffer[chan], nframes, m_gain);
		}
	}
}


#endif

