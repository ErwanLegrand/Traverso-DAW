/*
Copyright (C) 2006 Remon Sijrier

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


#ifndef PLUGIN_CHAIN_H
#define PLUGIN_CHAIN_H

#include <ContextItem.h>
#include <QList>
#include <QDomNode>
#include "Plugin.h"
#include "GainEnvelope.h"

class Song;
class AudioBus;

class PluginChain : public ContextItem
{
	Q_OBJECT
	
public:
	PluginChain(ContextItem* parent);
	PluginChain(ContextItem* parent, Song* song);
	~PluginChain();
	
	QDomNode get_state(QDomDocument doc);
	int set_state(const QDomNode & node );
	
	Command* add_plugin(Plugin* plugin, bool historable=true);
	Command* remove_plugin(Plugin* plugin, bool historable=true);
	void process_pre_fader(AudioBus* bus, unsigned long nframes);
	void process_post_fader(AudioBus* bus, unsigned long nframes);
	void process_fader(audio_sample_t* buffer, nframes_t pos, nframes_t nframes) {m_fader->process_gain(buffer, pos, nframes);}
	
	void set_song(Song* song);
	
	QList<Plugin* > get_plugin_list() {return m_pluginList;}
	GainEnvelope* get_fader() const {return m_fader;}
	
private:
	QList<Plugin* >	m_pluginList;
	GainEnvelope*	m_fader;
	Song*		m_song;
	
signals:
	void pluginAdded(Plugin* plugin);
	void pluginRemoved(Plugin* plugin);

private slots:
	void private_add_plugin(Plugin* plugin);
	void private_remove_plugin(Plugin* plugin);


};

inline void PluginChain::process_pre_fader(AudioBus * bus, unsigned long nframes)
{
	return;
	for (int i=0; i<m_pluginList.size(); ++i) {
		Plugin* plugin = m_pluginList.at(i);
		if (plugin == m_fader) return;
		plugin->process(bus, nframes);
	}
}

inline void PluginChain::process_post_fader(AudioBus * bus, unsigned long nframes)
{
	for (int i=0; i<m_pluginList.size(); ++i) {
		Plugin* plugin = m_pluginList.at(i);
// 		if (plugin == m_fader) continue;
		plugin->process(bus, nframes);
	}
}

#endif

//eof
