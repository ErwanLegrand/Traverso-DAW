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

#include "Track.h"
#include "Song.h"
#include "AudioClip.h"
#include "AudioClipManager.h"
#include <AudioBus.h>
#include <AudioDevice.h>
#include "PluginChain.h"
#include "Plugin.h"
#include "InputEngine.h"
#include "Information.h"
#include "ProjectManager.h"
#include "ResourcesManager.h"
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
	isSolo = mutedBySolo = isMuted = isArmed = false;
	set_history_stack(m_song->get_history_stack());
	m_pluginChain = new PluginChain(this, m_song);
	m_fader = m_pluginChain->get_fader();
	m_fader->set_gain(1.0);
	m_captureRightChannel = m_captureLeftChannel = true;
}

QDomNode Track::get_state( QDomDocument doc, bool istemplate)
{
	QDomElement node = doc.createElement("Track");
	if (! istemplate ) {
		node.setAttribute("id", m_id);
	}
	node.setAttribute("name", m_name);
	node.setAttribute("pan", m_pan);
	node.setAttribute("mute", isMuted);
	node.setAttribute("solo", isSolo);
	node.setAttribute("mutedbysolo", mutedBySolo);
	node.setAttribute("height", m_height);
	node.setAttribute("sortindex", m_sortIndex);
	node.setAttribute("numtakes", numtakes);
	node.setAttribute("InBus", busIn.data());
	node.setAttribute("OutBus", busOut.data());
	node.setAttribute("CaptureLeftChannel", m_captureLeftChannel);
	node.setAttribute("CaptureRightChannel", m_captureRightChannel);

	if (! istemplate ) {
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
	}
	
	QDomNode m_pluginChainNode = doc.createElement("PluginChain");
	m_pluginChainNode.appendChild(m_pluginChain->get_state(doc));
	node.appendChild(m_pluginChainNode);
	
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
	set_pan( e.attribute( "pan", "" ).toFloat() );
	set_bus_in( e.attribute( "InBus", "" ).toAscii() );
	set_bus_out( e.attribute( "OutBus", "" ).toAscii() );
	m_id = e.attribute("id", "0").toLongLong();
	if (m_id == 0) {
		m_id = create_id();
	}
	numtakes = e.attribute( "numtakes", "").toInt();
	m_captureRightChannel = e.attribute("CaptureRightChannel", "1").toInt();
	m_captureLeftChannel =  e.attribute("CaptureLeftChannel", "1").toInt();
	// never ever allow both to be 0 at the same time!
	if ( ! (m_captureRightChannel || m_captureLeftChannel) ) {
		m_captureRightChannel = m_captureLeftChannel = 1;
	}

	QDomElement ClipsNode = node.firstChildElement("Clips");
	if (!ClipsNode.isNull()) {
		QDomNode clipNode = ClipsNode.firstChild();
		while (!clipNode.isNull()) {
			QDomElement clipElement = clipNode.toElement();
			qint64 id = clipElement.attribute("id", "").toLongLong();
			
			AudioClip* clip = resources_manager()->get_clip(id);
			if (!clip) {
				info().critical(tr("Track: AudioClip with id %1 not \
						found in Resources database!").arg(id));
				break;
			}
			
			clip->set_song(m_song);
			clip->set_track(this);
			clip->set_state(clip->get_dom_node());
			m_song->get_audioclip_manager()->add_clip(clip);
			private_add_clip(clip);
			
			clipNode = clipNode.nextSibling();
		}
	}

	QDomNode m_pluginChainNode = node.firstChildElement("PluginChain");
	if (!m_pluginChainNode.isNull()) {
		m_pluginChain->set_state(m_pluginChainNode);
	}
	
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


Command* Track::remove_clip(AudioClip* clip, bool historable, bool ismove)
{
	PENTER;
	if (! ismove) {
		m_song->get_audioclip_manager()->remove_clip(clip);
	}
	return new AddRemove(this, clip, historable, m_song,
		"private_remove_clip(AudioClip*)", "audioClipRemoved(AudioClip*)",
		"private_add_clip(AudioClip*)", "audioClipAdded(AudioClip*)", 
   		tr("Remove Clip"));
}


Command* Track::add_clip(AudioClip* clip, bool historable, bool ismove)
{
	PENTER;
	clip->set_track(this);
	if (! ismove) {
		m_song->get_audioclip_manager()->add_clip(clip);
	}
	return new AddRemove(this, clip, historable, m_song,
		"private_add_clip(AudioClip*)", "audioClipAdded(AudioClip*)",
		"private_remove_clip(AudioClip*)", "audioClipRemoved(AudioClip*)", 
   		tr("Add Clip"));
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

AudioClip* Track::init_recording()
{
	PENTER2;
	if ( ! isArmed) {
		return 0;
	}
	
	QString name = 	"track-" + QString::number(m_song->get_track_index(m_id)) +
			"_take-" + QString::number(++numtakes);
	
	AudioClip* clip = resources_manager()->new_audio_clip(name);
	clip->set_song(m_song);
	clip->set_track(this);
	clip->set_track_start_frame(m_song->get_transport_frame());
	
	if (clip->init_recording(busIn) < 0) {
		PERROR("Could not create AudioClip to record to!");
		resources_manager()->destroy_clip(clip);
		return 0;
	} else {
		return clip;
	}
	
	return 0;
}


void Track::set_gain(float gain)
{
	if (gain < 0.0)
		gain = 0.0;
	if (gain > 2.0)
		gain = 2.0;
	m_fader->set_gain(gain);
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
	return m_fader->get_gain();
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
	
	// Get the 'render bus' from song, a bit hackish solution, but
	// it avoids to have a dedicated render bus for each Track,
	// or buffers located on the heap...
	AudioBus* bus = m_song->get_render_bus();
	bus->silence_buffers(nframes);
	
	audio_sample_t* mixdown = m_song->mixdown;
	int result;
	float gainFactor, panFactor;

	m_pluginChain->process_pre_fader(bus, nframes);
	
	for (int i=0; i<audioClipList.size(); ++i) {
	
		memset (mixdown, 0, sizeof (audio_sample_t) * nframes);
		
		AudioClip* clip = audioClipList.at(i);
		
		if (isArmed && clip->recording_state() == AudioClip::NO_RECORDING) {
			if (isMuted || mutedBySolo)
				continue;
		}
		
		for (int chan=0; chan<bus->get_channel_count(); ++chan) {
		
			result = clip->process(nframes, mixdown, chan);
			
			if (result <= 0) {
				continue;
			}

			processResult |= result;
			
			gainFactor = get_gain() * clip->get_gain();
			
			if ( (chan == 0) && (m_pan > 0)) {
				panFactor = 1 - m_pan;
				gainFactor *= panFactor;
			}
			
			if ( (chan == 1) && (m_pan < 0)) {
				panFactor = 1 + m_pan;
				gainFactor *= panFactor;
			}
			
			Mixer::mix_buffers_with_gain(bus->get_channel(chan)->get_buffer(nframes), mixdown, nframes, gainFactor);
		}
	}
	
	m_pluginChain->process_post_fader(bus, nframes);
		
	for (int i=0; i<bus->get_channel_count(); ++i) {
		Mixer::mix_buffers_no_gain(m_song->get_master_out()->get_buffer(i, nframes), bus->get_buffer(i, nframes), nframes);
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
	return m_pluginChain->add_plugin(plugin);
}

Command* Track::remove_plugin( Plugin * plugin )
{
	return m_pluginChain->remove_plugin(plugin);
}

void Track::set_sort_index( int index )
{
	m_sortIndex = index;
}

int Track::get_sort_index( ) const
{
	return m_sortIndex;
}

void Track::set_capture_left_channel(bool capture)
{
	m_captureLeftChannel = capture;
	emit inBusChanged();
}

void Track::set_capture_right_channel(bool capture)
{
	m_captureRightChannel = capture;
	emit inBusChanged();
}

// eof

