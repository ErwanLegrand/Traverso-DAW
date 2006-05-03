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

$Id: AudioClip.cpp,v 1.8 2006/05/03 11:59:39 r_sijrier Exp $
*/

#include "ContextItem.h"
#include "AudioClip.h"
#include "AudioSource.h"
#include "ReadSource.h"
#include "WriteSource.h"
#include "ColorManager.h"
#include "Song.h"
#include "Track.h"
#include "AudioChannel.h"
#include "Mixer.h"
#include "DiskIO.h"
#include "Export.h"
#include "AudioClipManager.h"
#include "AudioSourceManager.h"

#include <commands.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AudioClip::AudioClip(Track* track, nframes_t pTrackInsertBlock, QString name)
		: ContextItem((ContextItem*) 0, track) , m_track(track), m_name(name), trackStartFrame(pTrackInsertBlock)
{
	PENTERCONS;
	m_gain = 1.0;
	m_length = fadeOutBlocks = fadeInBlocks = sourceStartFrame = m_channels = sourceEndFrame = trackEndFrame = 0;
	isMuted=false;
	m_song = m_track->get_song();
	bitDepth = m_song->get_bitdepth();
	init();
}

AudioClip::AudioClip(Track* track, const QDomNode& node)
		: ContextItem((ContextItem*) 0, track), m_track(track)
{
	m_song = m_track->get_song();
	init();
	set_state( node );
}

AudioClip::~AudioClip()
{
	PENTERDES;
	
	foreach(ReadSource* source, readSources)
		delete source;
}

void AudioClip::init()
{
	connect(m_track, SIGNAL(muteChanged(bool )), this, SLOT(track_mute_changed( bool )));
	set_history_stack(m_track->get_history_stack());
	m_song->get_audioclip_manager()->add_clip( this );
	isRecording = false;
	isSelected = false;
}

int AudioClip::set_state(const QDomNode& node )
{
	QDomElement e = node.toElement();
	trackStartFrame = e.attribute( "trackstart", "" ).toUInt();
	sourceStartFrame = e.attribute( "sourcestart", "" ).toUInt();
	m_name = e.attribute( "clipname", "" ) ;
	m_length = e.attribute( "length", "0" ).toUInt();
	isTake = e.attribute( "take", "").toInt();
	uint channels = e.attribute( "channels", "0").toInt();
	bitDepth = e.attribute("origbitdepth", "0").toInt();
	set_fade_in( e.attribute( "fadeIn", "" ).toInt() );
	set_fade_out( e.attribute( "fadeOut", "").toInt() );
	set_gain( e.attribute( "gain", "" ).toFloat() );
	isMuted =  e.attribute( "mute", "" ).toInt();
	
	sourceEndFrame = sourceStartFrame + m_length;
	set_track_end_frame(trackStartFrame + m_length);
	
	if ( e.attribute("selected", "0").toInt() ) {
		m_song->get_audioclip_manager()->add_to_selection( this );
	}

	m_channels = 0;
	
	for (uint i=0; i<channels; i++) {
		QString sourceName = "source-" + QByteArray::number(i);
		qint64 id = e.attribute(sourceName, "").toLongLong();
		
		ReadSource* rs = pm().get_project()->get_audiosource_manager()->get_readsource( id );
		
		if ( ! rs ) {
			PWARN("no audio source returned!");
			m_channels = 0;
			return -1;
		} else {
			add_audio_source(rs, i);
		}
	}

	return 1;
}

QDomNode AudioClip::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("Clip");
	node.setAttribute("trackstart", trackStartFrame);
	node.setAttribute("sourcestart", sourceStartFrame);
	node.setAttribute("length", m_length);
	node.setAttribute("gain", m_gain);
	node.setAttribute("mute", isMuted);
	node.setAttribute("fadeIn", fadeInBlocks);
	node.setAttribute("fadeOut", fadeOutBlocks);
	node.setAttribute("take", isTake);
	node.setAttribute("channels", m_channels);
	node.setAttribute("clipname", m_name );
	node.setAttribute("selected", isSelected );
	node.setAttribute("origbitdepth", bitDepth );

	for (uint i=0; i<m_channels; i++) {
		QString sourceName = "source-" + QByteArray::number(i);
		node.setAttribute(sourceName, readSources.at(i)->get_id());
	}

	return node;
}

void AudioClip::toggle_mute()
{
	PENTER;
	isMuted=!isMuted;
	set_sources_active_state();	
	emit muteChanged(isMuted);
}

void AudioClip::track_mute_changed( bool )
{
	set_sources_active_state();	
}

void AudioClip::set_sources_active_state()
{
	if ( m_track->is_muted() || m_track->is_muted_by_solo() || is_muted() ) {
		foreach(ReadSource* source, readSources)
			source->set_inactive();
	} else {
		foreach(ReadSource* source, readSources)
			source->set_active();
	}	
		
}

void AudioClip::set_left_edge(nframes_t newFrame)
{
	
	if (newFrame < trackStartFrame) {
		
		int availableFramesLeft = sourceStartFrame;
		
		int movingToLeft = trackStartFrame - newFrame;
		
		if (movingToLeft > availableFramesLeft) {
			movingToLeft = availableFramesLeft;
		}
		
		trackStartFrame -= movingToLeft;
		set_source_start_frame( sourceStartFrame - movingToLeft );
	
	} else if (newFrame > trackStartFrame) {
		
		int availableFramesRight = m_length;
		
		int movingToRight = newFrame - trackStartFrame;
		
		if (movingToRight > availableFramesRight) {
			movingToRight = availableFramesRight;
		}
		
		trackStartFrame += movingToRight;
		set_source_start_frame( sourceStartFrame + movingToRight );
		
	} else {
		return;
	}

	emit positionChanged();
}

void AudioClip::set_right_edge(nframes_t newFrame)
{
	PENTER;
	if (newFrame > trackEndFrame) {
		
		int availableFramesRight = sourceLength - sourceEndFrame;
		
		int movingToRight = newFrame - trackEndFrame;
		
		if (movingToRight > availableFramesRight) {
			movingToRight = availableFramesRight;
		}
		
		set_track_end_frame( trackEndFrame + movingToRight );
		set_source_end_frame( sourceEndFrame + movingToRight );
	
	} else if (newFrame < trackEndFrame) {
		
		int availableFramesLeft = m_length;
		
		int movingToLeft = trackEndFrame - newFrame;
		
		if (movingToLeft > availableFramesLeft) {
			movingToLeft = availableFramesLeft;
		}
		
		set_track_end_frame( trackEndFrame - movingToLeft );
		set_source_end_frame( sourceEndFrame - movingToLeft);
	
	} else {
		return;
	}
	
	emit positionChanged();
}

void AudioClip::set_source_start_frame(nframes_t frame)
{
	sourceStartFrame = frame;
	m_length = sourceEndFrame - sourceStartFrame;
}

void AudioClip::set_source_end_frame(nframes_t frame)
{
	sourceEndFrame = frame;
	m_length = sourceEndFrame - sourceStartFrame;
}

void AudioClip::set_track_start_frame(nframes_t newTrackStartFrame)
{
	trackStartFrame = newTrackStartFrame;

	set_track_end_frame(trackStartFrame + m_length);

	emit positionChanged();
}

void AudioClip::set_track_end_frame( nframes_t endFrame )
{
	trackEndFrame = endFrame;
	emit trackEndFrameChanged();
}

void AudioClip::set_blur(bool )
{
	/*	ColorManager::set_blur(ColorManager::CLIP_BG_DEFAULT            , stat );
		ColorManager::set_blur(ColorManager::CLIP_BG_SELECTED           , stat );
		ColorManager::set_blur(ColorManager::CLIP_BG_MUTED              , stat );
		ColorManager::set_blur(ColorManager::CLIP_INFO_AREA_BG          , stat );
		ColorManager::set_blur(ColorManager::CLIP_INFO_AREA_BG_ACTIVE   , stat );
		ColorManager::set_blur(ColorManager::CLIP_PEAK_MICROVIEW        , stat );
		ColorManager::set_blur(ColorManager::CLIP_PEAK_MACROVIEW        , stat );
		ColorManager::set_blur(ColorManager::CLIP_PEAK_OVERLOADED_SAMPLE, stat );
		ColorManager::set_blur(ColorManager::DARK_TEXT                  , stat );*/
	emit stateChanged();
}

void AudioClip::set_fade_in(nframes_t b)
{
	fadeInBlocks=b;
	emit stateChanged();
}

void AudioClip::set_fade_out(nframes_t b)
{
	fadeOutBlocks=b;
	emit stateChanged();
}

void AudioClip::set_gain(float g) 
{
	PENTER3;
	g = g < 0.0 ? 0.0 : g;
	g = g > 1.0 ? 1.0 : g;
	m_gain = g;
	emit stateChanged();
}

int AudioClip::set_selected(bool selected)
{
	if (isSelected != selected) {
		isSelected = selected;
		emit stateChanged();
	}
	return 1;
}

int AudioClip::process(nframes_t nframes)
{
	if (isRecording) {
		process_capture(nframes);
		return 0;
	}

	nframes_t mix_pos;
	float gainFactor = m_gain * m_track->get_gain();

	if (isMuted || (gainFactor == 0.0) || (m_channels == 0)) {
		return 0;
	}

	if ( (trackStartFrame < m_song->get_transport_frame()) && (trackEndFrame > m_song->get_transport_frame()) ) {

		mix_pos = m_song->get_transport_frame() - trackStartFrame + sourceStartFrame;

	} else {
		return 0;
	}


	AudioBus* bus = m_song->get_master_out();
	audio_sample_t* mixdown = m_song->mixdown;
	int channels = m_channels;
	nframes_t read_frames = 0;
	
	if (channels > bus->get_channel_count())
		channels = bus->get_channel_count();

	for (int channel=0; channel<channels; channel++) {
		if (m_song->realtime_path())
			read_frames = readSources.at(channel)->rb_read(mixdown, mix_pos, nframes);
		else
			read_frames = readSources.at(channel)->file_read(mixdown, mix_pos, nframes);

		if (read_frames == 0)
			continue;

		gainFactor = m_gain * m_track->get_gain();
		float panFactor = 1;

		if ( (channel == 0) && (m_track->get_pan() > 0)) {
			panFactor = 1 - m_track->get_pan();
			gainFactor *= panFactor;
		}
		if ( (channel == 1) && (m_track->get_pan() < 0)) {
			panFactor = 1 + m_track->get_pan();
			gainFactor *= panFactor;
		}

		if (read_frames == nframes) {
			// If gainFactor == 1.0 , then there's no need to apply it.
			if (gainFactor == 1.0)
				Mixer::mix_buffers_no_gain(bus->get_buffer(channel, nframes), mixdown, nframes);
			else
				Mixer::mix_buffers_with_gain(bus->get_buffer(channel, nframes), mixdown, nframes, gainFactor);
		}
	}

	return 1;
}

void AudioClip::process_capture( nframes_t nframes )
{
	for (int channel=0; channel < writeSources.size(); channel++) {
		WriteSource* source = writeSources.at(channel);
		nframes_t written = source->rb_write(captureBus->get_buffer(channel, nframes), m_song->get_transport_frame(), nframes);
		if (written != nframes) {
			PWARN("couldn't write nframes to buffer, only %d", written);
		}
	}
	captureBus->monitor_peaks();
}

int AudioClip::init_recording( QByteArray name )
{
	captureBus = audiodevice().get_capture_bus(name);

	if (!captureBus) {
		info().warning( tr("Unable to Record to Track") );
		info().information( tr("AudioDevice doesn't have this Capture Bus: %1 (Track %2)").
				arg(name.data()).arg(m_track->get_id()) );
		return -1;
	}


	for (int chan=0; chan<captureBus->get_channel_count(); chan++) {
		ExportSpecification*  spec = new ExportSpecification;

		spec->exportdir = pm().get_project()->get_root_dir() + "/audiosources";
		spec->format = SF_FORMAT_WAV;
		spec->data_width = 1;	// 1 means float
		spec->format |= SF_FORMAT_FLOAT;
		spec->channels = 1;
		spec->sample_rate = audiodevice().get_sample_rate();
		spec->src_quality = SRC_SINC_MEDIUM_QUALITY;
		
		QString songid = QString::number(m_song->get_id())  + "_";
		if (m_song->get_id() < 10)
			songid.prepend("0");
		songid.prepend( "Song" );
		
		spec->name = songid + m_name + "-" + QByteArray::number(chan) + ".wav";

		spec->dataF = captureBus->get_buffer( chan, audiodevice().get_buffer_size());

		WriteSource* ws = new WriteSource(spec, 0);
		ws->set_process_peaks( true );
		ws->set_recording( true );
		
		connect(ws, SIGNAL(exportFinished( WriteSource* )), this, SLOT(finish_write_source( WriteSource* )));

		writeSources.insert(chan, ws);
		m_song->get_diskio()->register_write_source( ws );
	}

	sourceStartFrame = 0;
	isTake = 1;
	isRecording = true;
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(finish_recording()));

	return 1;
}

Command* AudioClip::remove_from_selection()
{
	return new ClipSelection(this, "remove_from_selection");
}

Command * AudioClip::add_to_selection()
{
	return new ClipSelection(this, "add_to_selection");
}

Command* AudioClip::select()
{
	return new ClipSelection(this, "select_clip");
}

Command* AudioClip::mute()
{
	return new PCommand(this, "toggle_mute");
}

Command* AudioClip::reset_gain()
{
	return new ClipGain(this, 1.000);
}

Command* AudioClip::reset_fade_in()
{
	set_fade_in(0);
	return (Command*) 0;
}

Command* AudioClip::reset_fade_out()
{
	set_fade_out(0);
	return (Command*) 0;
}

Command* AudioClip::reset_fade_both()
{
	set_fade_in(0);
	set_fade_out(0);
	return (Command*) 0;
}

Command* AudioClip::drag()
{
	return new MoveClip(m_song, this);
}

Command* AudioClip::drag_edge()
{
	int x = cpointer().clip_area_x();
	int cxm = m_song->frame_to_xpos( trackStartFrame + ( m_length / 2 ) );

	MoveEdge* me;

	if (x < cxm)
		me =   new  MoveEdge(this, "set_left_edge");
	else
		me = new MoveEdge(this, "set_right_edge");

	return me;
}

Command* AudioClip::gain()
{
	return new ClipGain(this);
}

Command* AudioClip::split()
{
	return new SplitClip(m_song, this);
}

Command* AudioClip::copy()
{
	return new CopyClip(m_song, this);
}

AudioClip * AudioClip::prev_clip( )
{
	return m_track->get_cliplist().prev(this);
}

AudioClip * AudioClip::next_clip( )
{
	return m_track->get_cliplist().next(this);
}

AudioClip* AudioClip::create_copy( )
{
	QDomDocument doc("AudioClip");
	QDomNode clipState = get_state(doc);
	AudioClip* clip = new AudioClip(m_track, clipState);
	clip->set_name( clip->get_name().prepend(tr("Copy of - ")) );
	return clip;
}

Peak* AudioClip::get_peak_for_channel( int chan ) const
{
	ReadSource* source = readSources.at(chan);
	
	if (source)
		return source->get_peak();
	
	return (Peak*) 0;
}

void AudioClip::add_audio_source( ReadSource* rs, int channel )
{
	PENTER;
	
	readSources.insert(channel, rs);
	sourceLength = rs->get_nframes();
	set_source_end_frame( rs->get_nframes() );
	m_channels = readSources.size();
	m_song->get_diskio()->register_read_source( rs );
	rate = rs->get_rate();
	
	set_sources_active_state();
}

void AudioClip::finish_write_source( WriteSource * ws )
{
	PENTER;
 	
 	printf("AudioClip::finish_write_source :  thread id is: %ld\n", QThread::currentThreadId ());
	
	for (int i=0; i<writeSources.size(); ++i) {
		if (ws == writeSources.at(i)) {
			ReadSource* rs = new ReadSource(ws);
			
			rs->set_created_by_song( m_song->get_id() );
			rs->set_original_bit_depth( audiodevice().get_bit_depth() );
			
			add_audio_source(rs, i);
			pm().get_project()->get_audiosource_manager()->add(rs);
			
			delete ws;
			ws = 0;
		}
	}
}

void AudioClip::finish_recording()
{
	PENTER;
	
	foreach(WriteSource* ws, writeSources) {
		ws->set_recording(false);
	}

	disconnect(m_song, SIGNAL(transferStopped()), this, SLOT(finish_recording()));
	
	isRecording = false;
}

int AudioClip::get_channels( ) const
{
	return m_channels;
}

Song * AudioClip::get_song( ) const
{
	return m_song;
}

Track * AudioClip::get_track( ) const
{
	return m_track;
}

void AudioClip::set_track( Track * t )
{
	m_track = t;
}

void AudioClip::set_name( QString name )
{
	m_name = name;
}

float AudioClip::get_gain( ) const
{
	return m_gain;
}

bool AudioClip::is_selected( ) const
{
	return isSelected;
}

bool AudioClip::is_take( ) const
{
	return isTake;
}

bool AudioClip::is_muted( ) const
{
	return isMuted;
}

QString AudioClip::get_name( ) const
{
	return m_name;
}

int AudioClip::get_bitdepth( ) const
{
	return bitDepth;
}

int AudioClip::get_rate( ) const
{
	return rate;
}

nframes_t AudioClip::get_source_length( ) const
{
	return sourceLength;
}

nframes_t AudioClip::get_length() const
{
	return m_length;
}

int AudioClip::get_baseY() const
{
	return m_track->real_baseY() + 3;
}

int AudioClip::get_width() const
{
	nframes_t nframes = sourceEndFrame - sourceStartFrame;
	int xwidth = (int) ( nframes / Peak::zoomStep[m_song->get_hzoom()] );
	return xwidth;
}

int AudioClip::get_height() const
{
	return m_track->get_height() - 8;
}

bool AudioClip::is_recording( ) const
{
	return isRecording;
}

nframes_t AudioClip::get_fade_out_frames( ) const
{
	return fadeOutBlocks;
}

nframes_t AudioClip::get_fade_in_frames( ) const
{
	return fadeInBlocks;
}

nframes_t AudioClip::get_source_end_frame( ) const
{
	return sourceEndFrame;
}

nframes_t AudioClip::get_source_start_frame( ) const
{
	return sourceStartFrame;
}

nframes_t AudioClip::get_track_end_frame( ) const
{
	return trackEndFrame;
}

nframes_t AudioClip::get_track_start_frame( ) const
{
	return trackStartFrame;
}

float AudioClip::get_fade_factor_for( nframes_t pos )
{
	float fi,fo;
	if (fadeInBlocks==0)
		fi = 1.0;
	else
	{
		if (pos < trackStartFrame) // that happens in the edge, the current playing pos , when rounded to locate the clip, may lead to a position "considered" inside the clip, but it is not.
			fi = 0.0;
		else if (pos > trackStartFrame + fadeInBlocks)
			fi = 1.0;
		else
			fi = (float)((double)(pos - trackStartFrame)/fadeInBlocks);
	}
	if (fadeOutBlocks==0)
		fo = 1.0;
	else
	{
		if (pos> trackEndFrame)
			fo = 0.0;
		else if ( pos < trackEndFrame - fadeOutBlocks)
			fo = 1.0;
		else
			fo = (float)((double)(trackEndFrame-pos)/fadeOutBlocks);
	}
	// TODO calculate cross-fades-gains
	return fi*fo;
}

// eof


