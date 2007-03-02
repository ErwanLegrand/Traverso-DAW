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

$Id: Track.cpp,v 1.39 2007/03/02 13:36:15 r_sijrier Exp $
*/

#include "Track.h"
#include "Song.h"
#include "AudioClip.h"
#include "AudioClipManager.h"
#include <AudioBus.h>
#include <AudioDevice.h>
#include "PluginChain.h"
#include "Plugin.h"
#include "InputEngine.h"
#include "ProjectManager.h"
#include "AudioSourceManager.h"
#include "Project.h"
#include "Utils.h"

#include <commands.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Track::Track(Song* song, const QString& name, int height )
	: ContextItem(song), 
	  m_song(song), 
	  m_name(name),
	  m_height(height)
{
	PENTERCONS;
	m_pan = numtakes = 0;
	m_gain = 0.5;
	m_sortIndex = -1;
	m_id = create_id();
	
	busIn = "Capture 1";
	busOut = "MasterOut";
	
	init();
}

Track::Track( Song * song, const QDomNode node)
	: ContextItem(song), 
	  m_song(song)
{
	PENTERCONS;
	init();
}


Track::~Track()
{
	PENTERDES;
}

void Track::init()
{
	isSolo = mutedBySolo = isActive = isMuted = isArmed = false;
	set_history_stack(m_song->get_history_stack());
	pluginChain = new PluginChain(this, m_song);
}

QDomNode Track::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("Track");
	node.setAttribute("id", m_id);
	node.setAttribute("name", m_name);
	node.setAttribute("gain", m_gain);
	node.setAttribute("pan", m_pan);
	node.setAttribute("mute", isMuted);
	node.setAttribute("solo", isSolo);
	node.setAttribute("mutedbysolo", mutedBySolo);
	node.setAttribute("height", m_height);
	node.setAttribute("sortindex", m_sortIndex);
	node.setAttribute("numtakes", numtakes);
	node.setAttribute("InBus", busIn.data());
	node.setAttribute("OutBus", busOut.data());

	QDomNode clips = doc.createElement("Clips");
	
	foreach(AudioClip* clip, audioClipList) {
		if (clip->get_length() == 0) {
			PERROR("Clip lenght is 0! This shouldn't happen!!!!");
			continue;
		}
		
		QDomElement clipNode = doc.createElement("Clip");
		clipNode.setAttribute("id", clip->get_id() );
		clips.appendChild(clipNode);
	}
	
	node.appendChild(clips);
	
	QDomNode pluginChainNode = doc.createElement("PluginChain");
	pluginChainNode.appendChild(pluginChain->get_state(doc));
	node.appendChild(pluginChainNode);
	
	return node;
}


int Track::set_state( const QDomNode & node )
{
	QDomElement e = node.toElement();
	
	set_height(e.attribute( "height", "160" ).toInt() );
	m_sortIndex = e.attribute( "sortindex", "-1" ).toInt();
	m_name = e.attribute( "name", "" );
	set_muted(e.attribute( "mute", "" ).toInt());
	if (e.attribute( "solo", "" ).toInt()) {
		solo();
	}
	set_muted_by_solo(e.attribute( "mutedbysolo", "0").toInt());
	m_gain =  e.attribute( "gain", "" ).toFloat();
	set_pan( e.attribute( "pan", "" ).toFloat() );
	set_bus_in( e.attribute( "InBus", "" ).toAscii() );
	set_bus_out( e.attribute( "OutBus", "" ).toAscii() );
	m_id = e.attribute( "id", "").toLongLong();
	numtakes = e.attribute( "numtakes", "").toInt();

	QDomElement ClipsNode = node.firstChildElement("Clips");
	if (!ClipsNode.isNull()) {
		QDomNode clipNode = ClipsNode.firstChild();
		while (!clipNode.isNull()) {
			QDomElement clipElement = clipNode.toElement();
			qint64 id = clipElement.attribute("id", "").toLongLong();
			
			AudioClip* clip = pm().get_project()->get_audiosource_manager()->get_clip(id);
			if (!clip) {
				PERROR("Clip with id %ld not found!", (long)id);
				break;
			}
			
			clip->set_song(m_song);
			clip->set_track(this);
			clip->set_state(clip->m_domNode);
			m_song->get_audioclip_manager()->add_clip(clip);
			private_add_clip(clip);
			
			clipNode = clipNode.nextSibling();
		}
	}

	QDomNode pluginChainNode = node.firstChildElement("PluginChain");
	pluginChain->set_state(pluginChainNode);
	
	return 1;
}


AudioClip* Track::get_clip_after(nframes_t framePos)
{
	AudioClip* clip;
	for (int i=0; i < audioClipList.size(); ++i) {
		clip = audioClipList.at(i);
		if (clip->get_track_start_frame() > framePos)
			return clip;
	}
	return (AudioClip*) 0;
}


AudioClip* Track::get_clip_before(nframes_t framePos)
{
	AudioClip* clip;
	for (int i=0; i < audioClipList.size(); ++i) {
		clip = audioClipList.at(i);
		if (clip->get_track_start_frame() < framePos)
			return clip;
	}
	return (AudioClip*) 0;
}


Command* Track::remove_clip(AudioClip* clip, bool historable)
{
	PENTER;
	m_song->get_audioclip_manager()->remove_clip(clip);
	return new AddRemoveItemCommand(this, clip, historable, m_song,
			"private_remove_clip(AudioClip*)", "audioClipRemoved(AudioClip*)",
			"private_add_clip(AudioClip*)", "audioClipAdded(AudioClip*)", tr("Remove Clip"));
}


Command* Track::add_clip(AudioClip* clip, bool historable)
{
	PENTER;
	clip->set_track(this);
	m_song->get_audioclip_manager()->add_clip(clip);
	return new AddRemoveItemCommand(this, clip, historable, m_song,
			"private_add_clip(AudioClip*)", "audioClipAdded(AudioClip*)",
			"private_remove_clip(AudioClip*)", "audioClipRemoved(AudioClip*)", tr("Add Clip"));
}


void Track::activate()
{
	if (!isActive) {
		isActive=true;
	}
}


void Track::deactivate()
{
	if (isActive) {
		isActive=false;
	}
}


int Track::arm()
{
	PENTER;
	set_armed(true);
	AudioBus* bus = audiodevice().get_capture_bus(busIn);
	if (bus) {
		bus->set_monitor_peaks(true);
	}
	return 1;
}


int Track::disarm()
{
	PENTER;
	set_armed(false);
	AudioBus* bus = audiodevice().get_capture_bus(busIn);
	if (bus) {
		bus->set_monitor_peaks(false);
	}
	return 1;
}

void Track::set_bus_in(QByteArray bus)
{
	bool wasArmed=isArmed;
	if (isArmed)
		disarm();
	busIn=bus;
	if (wasArmed) {
		arm();
	}
	
	emit inBusChanged();
}

void Track::set_bus_out(QByteArray bus)
{
	busOut=bus;
	emit outBusChanged();
}

bool Track::is_solo()
{
	return isSolo;
}

bool Track::is_muted()
{
	return isMuted;
}

bool Track::is_muted_by_solo()
{
	return mutedBySolo;
}


bool Track::armed()
{
	return isArmed;
}

bool Track::is_active()
{
	return isActive;
}

void Track::toggle_active()
{
	isActive=!isActive;
	emit isActiveChanged(isActive);
}


Command* Track::init_recording()
{
	PENTER2;
	if (isArmed) {
		QByteArray name = "Audio-" + QByteArray::number(m_song->get_track_index(m_id)) + 
				"-take-" + QByteArray::number(++numtakes);
		
		AudioClip* clip = pm().get_project()->get_audiosource_manager()->new_audio_clip(name);
		clip->set_song(m_song);
		clip->set_track(this);
		clip->set_track_start_frame(m_song->get_transport_frame());
		
		if (clip->init_recording(busIn) < 0) {
			PERROR("Could not create AudioClip to record to!");
			delete clip;
		} else {
			return add_clip( clip );
		}
	}
	
	return 0;
}


void Track::set_gain(float gain)
{
	if (gain < 0.0)
		gain = 0.0;
	if (gain > 2.0)
		gain = 2.0;
	m_gain = gain;
	emit gainChanged();
}


void Track::set_pan(float pan)
{
	if ( pan < -1.0 )
		m_pan=-1.0;
	else
		if ( pan > 1.0 )
			m_pan=1.0;
		else
			m_pan=pan;
	emit panChanged();
}


void Track::set_height(int h)
{
	m_height = h;
	emit heightChanged();
}


int Track::get_total_clips()
{
	return audioClipList.count();
}


float Track::get_gain( ) const
{
	return m_gain;
}

void Track::set_muted_by_solo(bool muted)
{
	PENTER;
	mutedBySolo = muted;
	emit audibleStateChanged();
}

void Track::set_solo(bool solo)
{
	isSolo = solo;
	if (solo)
		mutedBySolo = false;
	emit soloChanged(isSolo);
	emit audibleStateChanged();
}

void Track::set_muted( bool muted )
{
	isMuted = muted;
	emit muteChanged(isMuted);
	emit audibleStateChanged();
}

void Track::set_armed( bool armed )
{
	isArmed = armed;
	emit armedChanged(isArmed);
}


//
//  Function called in RealTime AudioThread processing path
//
int Track::process( nframes_t nframes )
{
	int processResult = 0;
	
	if ( (isMuted || mutedBySolo) && ( ! isArmed) ) {
		return 0;
	}
		
	AudioBus* bus = m_song->get_master_out();
	
	if (!bus) {
		return 0;
	}
	
	audio_sample_t* channelBuffer;
	audio_sample_t* mixdown = m_song->mixdown;
	int result;
	float gainFactor, panFactor;
	
	
	for (int i=0; i<audioClipList.size(); ++i) {
	
		memset (mixdown, 0, sizeof (audio_sample_t) * nframes);
		
		AudioClip* clip = audioClipList.at(i);
		
		
		for (int chan=0; chan<bus->get_channel_count(); ++chan) {
		
			channelBuffer = bus->get_buffer(chan, nframes);
			
			result = clip->process(nframes, mixdown, chan);
			
			if (result == 0) {
				continue;
			}

			if (result != -1) { // No such channel !!
				processResult |= result;
			}
			
			gainFactor = m_gain * clip->get_gain() * clip->get_norm_factor();
			
			if ( (chan == 0) && (m_pan > 0)) {
				panFactor = 1 - m_pan;
				gainFactor *= panFactor;
			}
			
			if ( (chan == 1) && (m_pan < 0)) {
				panFactor = 1 + m_pan;
				gainFactor *= panFactor;
			}
			
			Mixer::mix_buffers_with_gain(channelBuffer, mixdown, nframes, gainFactor);
		}
	}
	
	QList<Plugin* >* pluginList = pluginChain->get_plugin_list();
	
	
	for (int i=0; i<pluginList->size(); ++i) {
		pluginList->at(i)->process(bus, nframes);
	}
		
	return processResult;
}

Command* Track::mute()
{
	PENTER;
	set_muted(!isMuted);
	
	return (Command*) 0;
}

Command* Track::toggle_arm()
{
	if (isArmed)
		disarm();
	else
		arm();
	return (Command*) 0;
}

Command* Track::solo(  )
{
	m_song->solo_track(this);
	return (Command*) 0;
}

Command* Track::gain()
{
	return new Gain(this, tr("Track %1 Gain").arg(m_song->get_track_index(m_id)));
}

Command* Track::reset_gain()
{
	float newGain = 0.5;
	if (m_gain == 0.5) {
		newGain = 1.0;
	}
	
	return new Gain(this, tr("Track %1 Gain").arg(m_song->get_track_index(m_id)), newGain);
}

Command* Track::pan()
{
	return new TrackPan(this, m_song);
}

Command* Track::import_audiosource()
{
	return new Import(this);
}

Command* Track::silence_others( )
{
	return new PCommand(this, "solo", tr("Silence Others"));
}

void Track::set_name( const QString & name )
{
	m_name = name;
	emit stateChanged();
}

void Track::get_render_range(nframes_t& startframe, nframes_t& endframe )
{
	if (audioClipList.size() == 0)
		return;
		
	endframe = 0;
	startframe = INT_MAX;
	
	AudioClip* clip;
	
	for(int i=0; i < audioClipList.size(); ++i) {
		clip = audioClipList.at( i );
		
		if (! clip->is_muted() ) {
			if (clip->get_track_end_frame() > endframe) {
				endframe = clip->get_track_end_frame();
			}
			
			if (clip->get_track_start_frame() < startframe) {
				startframe = clip->get_track_start_frame();
			}
		}
	}
	
}

void Track::private_add_clip(AudioClip* clip)
{
	audioClipList.add_clip(clip);
}

void Track::private_remove_clip(AudioClip* clip)
{
	audioClipList.remove_clip(clip);
}

Command* Track::add_plugin( Plugin * plugin )
{
	return pluginChain->add_plugin(plugin);
}

Command* Track::remove_plugin( Plugin * plugin )
{
	return pluginChain->remove_plugin(plugin);
}

void Track::set_sort_index( int index )
{
	m_sortIndex = index;
}

int Track::get_sort_index( ) const
{
	return m_sortIndex;
}


Command * Track::remove_myself( )
{
	return m_song->remove_track(this);
}


// eof
