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

$Id: Track.cpp,v 1.24 2006/08/31 17:55:38 r_sijrier Exp $
*/

#include "Track.h"
#include "Song.h"
#include "AudioClip.h"
#include "Tsar.h"
#include "PluginChain.h"
#include "Plugin.h"

#include <commands.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Track::Track(Song* song, int pID, QString pName, int pBaseY, int pHeight )
		: ContextItem((ContextItem*) 0, song), m_song(song), ID(pID), m_name(pName),
		baseY(pBaseY), height(pHeight)
{
	PENTERCONS;
	m_pan = numtakes = 0;
	m_gain = 0.5;
	
	busIn = "Capture 1";
	busOut = "MasterOut";
	
	init();
}

Track::Track( Song * song, const QDomNode )
		: ContextItem( (ContextItem*) 0, song ), m_song(song)
{
	PENTERCONS;
	// These are used by TrackView _before_ they are initialized by set_state !!
	baseY = height = 0;
	
	// ALWAYS call init() before new member objects are created!
	init();
}


Track::~Track()
{
	PENTERDES;
 	delete pluginChain;
}

void Track::init()
{
	isSolo = mutedBySolo = isActive = isMuted = isArmed = false;
	set_history_stack(m_song->get_history_stack());
	pluginChain = new PluginChain;
	
	connect(m_song, SIGNAL( transferStarted() ), this, SLOT (init_recording() ));
}

QDomNode Track::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("Track");
	node.setAttribute("id", ID);
	node.setAttribute("name", m_name);
	node.setAttribute("gain", m_gain);
	node.setAttribute("pan", m_pan);
	node.setAttribute("mute", isMuted);
	node.setAttribute("solo", isSolo);
	node.setAttribute("height", height);
	node.setAttribute("numtakes", numtakes);
	node.setAttribute("InBus", busIn.data());
	node.setAttribute("OutBus", busOut.data());

	QDomNode clips = doc.createElement("Clips");
	foreach(AudioClip* clip, audioClipList) {
		if (clip->get_length() == 0) {
			PERROR("Clip lenght is 0! This shouldn't happen!!!!");
			continue;
		}
		clips.appendChild(clip->get_state(doc));
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
	m_name = e.attribute( "name", "" );
	set_muted(e.attribute( "mute", "" ).toInt());
	if (e.attribute( "solo", "" ).toInt()) {
		solo();
	}
	m_gain =  e.attribute( "gain", "" ).toFloat();
	set_pan( e.attribute( "pan", "" ).toFloat() );
	set_bus_in( e.attribute( "InBus", "" ).toAscii() );
	set_bus_out( e.attribute( "OutBus", "" ).toAscii() );
	set_height( e.attribute( "height", "160" ).toInt() );
	ID = e.attribute( "id", "").toInt();
	numtakes = e.attribute( "numtakes", "").toInt();

	QDomElement ClipsNode = node.firstChildElement("Clips");
	if (!ClipsNode.isNull()) {
		QDomNode clipNode = ClipsNode.firstChild();
		while (!clipNode.isNull()) {
			AudioClip* clip = new AudioClip(this, clipNode);
			// First add the clip, this will emit the clipAdded Signal!
			add_clip( clip );
			// Now set the clips state, which will eventually generate
			// other signals, so the GUI can act on it!
			clip->set_state(clipNode);
			clipNode = clipNode.nextSibling();
		}
	}

	QDomNode pluginChainNode = node.firstChildElement("PluginChain");
	pluginChain->set_state(pluginChainNode);
	
	return 1;
}


bool Track::is_pointed(int y)
{
	return ( (y >= real_baseY()) && (y <= real_baseY() + height) );
}


QList<AudioClip* > Track::split_clip(nframes_t splitPoint)
{
	PENTER2;

	QList<AudioClip* > clipPair;

	PMESG("Trying to find clip under %ld", (long)splitPoint);
	AudioClip* c = get_clip_under(splitPoint);

	if (c)
		clipPair = split_clip(c, splitPoint);
	else
		PMESG("I'm uppon no clip :-(");

	return clipPair;
}


QList<AudioClip* > Track::split_clip(AudioClip* clip, nframes_t splitPoint)
{
	PENTER2;
	QList<AudioClip* > clipPair;

	nframes_t leftLenght = splitPoint - (clip->get_track_start_frame());
	nframes_t rightSourceFirstBlock = clip->get_source_start_frame() + leftLenght;

	AudioClip* leftClip = clip->create_copy();
	leftClip->set_source_end_frame(rightSourceFirstBlock);
	clipPair.append(leftClip);

	AudioClip* rightClip = clip->create_copy();
	rightClip->set_source_start_frame( rightSourceFirstBlock );
	rightClip->set_track_start_frame( splitPoint );
	clipPair.append(rightClip);

	return clipPair;
}


AudioClip* Track::get_clip_under(nframes_t framePos)
{
	AudioClip* clip = (AudioClip*) 0;
	for (int i=0; i < audioClipList.size(); ++i) {
		clip = audioClipList.at(i);
		if ((clip->get_track_start_frame() <= framePos) && (clip->get_track_end_frame() >= framePos))
			return clip;
		if (clip->get_track_start_frame() > framePos)
			break;
	}
	return clip;
}


AudioClip* Track::get_clip_between(nframes_t , nframes_t )
{
	PENTER4;
	AudioClip* theClip = (AudioClip*) 0;
	return theClip;
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



//FIXME needs to be reviewed. NOT USED ANYMORE...
// int Track::delete_clip(AudioClip* clip, bool permanently)
// {
// 	PENTER;
// 	if (!clip) {
// 		PERROR("Trying to delete invalid Clip");
// 		return -1;
// 	}
// 	if(permanently && (m_song->get_clips_count_for_audio(clip->get_audio_source()) == 1))//FIXME this clip is the only one with this audioSource,
// 		clip->get_audio_source()->get_peak()->free_buffer_memory();					//so we have to unbuild the peak RAMBuffers... or not? Waste of memory usage if we keep it there...
// 
// 
// 	return 1;
// }


int Track::remove_clip(AudioClip* clip)
{
	PENTER;
	if ( ! m_song->is_transporting() ) {
		private_remove_clip(clip);
		emit audioClipRemoved(clip);
// 		printf("Song not running, removing Clip now\n");
		return 1;
	} else {
		THREAD_SAVE_CALL_EMIT_SIGNAL(this, clip, private_remove_clip(AudioClip*), audioClipRemoved(AudioClip*));
	}
	
	return 1;
}


void Track::add_clip(AudioClip* clip)
{
	PENTER;
	if ( ! m_song->is_transporting() ) {
		private_add_clip(clip);
		emit audioClipAdded(clip);
// 		printf("Song not running, adding Clip now\n");
	} else {
		THREAD_SAVE_CALL_EMIT_SIGNAL(this, clip, private_add_clip(AudioClip*), audioClipAdded(AudioClip*));
	}
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


void Track::set_blur(bool )
{
}


int Track::arm()
{
	PENTER;
	set_armed(true);
	return 1;
}


int Track::disarm()
{
	PENTER;
	set_armed(false);
	return 1;
}

void Track::set_bus_in(QByteArray bus)
{
	bool wasArmed=isArmed;
	if (isArmed)
		disarm();
	busIn=bus;
	if (wasArmed)
		arm();
	emit stateChanged();
}

void Track::set_bus_out(QByteArray bus)
{
	busOut=bus;
	emit stateChanged();
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


void Track::init_recording()
{
	PENTER2;
	if (isArmed) {
		QByteArray name = "Audio_" + QByteArray::number(ID) + "." + QByteArray::number(++numtakes);
		AudioClip* clip = new AudioClip(this, m_song->get_transport_frame(), name);
		if (clip->init_recording(busIn) < 0) {
			PERROR("Could not create AudioClip to record to!");
			delete clip;
		} else {
			add_clip( clip );
		}
	}
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


void Track::vzoom_in(int newBaseY)
{
	baseY = newBaseY;
	if (1.1*height<500.0)
		set_height((int) (1.10*height));
}


void Track::vzoom_out(int newBaseY)
{
	baseY = newBaseY;
	if ( 0.9 * height > MINIMUM_HEIGHT )
		set_height((int) (0.90 * height));
}


void Track::set_baseY(int b)
{
	baseY = b;

	// I know, this is not true, but who cares. It causes everything to be repainted!
	emit heightChanged();
}


void Track::set_height(int h)
{
	height = h;
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

int Track::real_baseY( ) const
{
	//FIXME!!!!
	return baseY;// + m_song->mtaBaseY;
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
	
	QList<Plugin* >	pluginList = pluginChain->get_plugin_list();
	
	foreach(Plugin* plugin, pluginList) {
		plugin->process(bus, nframes);
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
	return new Gain(this);
}

Command* Track::pan()
{
	return new TrackPan(this, m_song);
}

Command * Track::import_audiosource(  )
{
	return new Import(this);
}

Command* Track::silence_others( )
{
	return new PCommand(this, "solo");
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
	clip->set_track(this);
	audioClipList.add_clip(clip);
}

void Track::private_remove_clip(AudioClip* clip)
{
	audioClipList.remove_clip(clip);
}

void Track::set_id( int id )
{
	ID = id;
}

void Track::add_plugin( Plugin * plugin )
{
	pluginChain->insert_plugin(plugin);
}

void Track::remove_plugin( Plugin * plugin )
{
	pluginChain->remove_plugin(plugin);
}



// eof
