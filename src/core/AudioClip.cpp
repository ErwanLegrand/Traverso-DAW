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

$Id: AudioClip.cpp,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
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

#include <commands.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AudioClip::AudioClip(Track* track, nframes_t pTrackInsertBlock, QString name)
		: ContextItem() , m_track(track), m_name(name), trackStartFrame(pTrackInsertBlock)
{
	PENTERCONS;
	m_gain = 1.0;
	m_length = fadeOutBlocks = fadeInBlocks = sourceStartFrame = m_channels = 0;
	isMuted=false;
	isSelected=false;
	set_blur(false);
	m_song = m_track->get_parent_song();
	bitDepth = m_song->get_bitdepth();
	init();
}

AudioClip::AudioClip(Track* track, const QDomNode& node)
		: ContextItem(), m_track(track)
{
	m_song = m_track->get_parent_song();
	set_state( node );
	init();
}

AudioClip::~AudioClip()
{
	PENTERDES;
	delete [] mixdown;
	
	foreach(ReadSource* source, readSources)
		delete source;
}

void AudioClip::init()
{
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(resize_buffer()), Qt::DirectConnection);
	connect(m_track, SIGNAL(muteChanged(bool )), this, SLOT(track_mute_changed( bool )));
	set_history_stack(m_track->get_history_stack());
	mixdown = new audio_sample_t[audiodevice().get_buffer_size()];
	sourceEndFrame = sourceStartFrame + m_length;
	set_track_end_frame(trackStartFrame + m_length);
	isRecording = false;
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
	set_fade_in( e.attribute( "fadeIn", "" ).toInt() );
	set_fade_out( e.attribute( "fadeOut", "").toInt() );
	set_gain( e.attribute( "gain", "" ).toFloat() );
	set_muted( e.attribute( "mute", "" ).toInt() );
	isSelected = e.attribute("selected", "0").toInt();
	bitDepth = e.attribute("origbitdepth", "0").toInt();

	m_channels = 0;
	
	for (uint i=0; i<channels; i++) {
		QString sourceName = "source-" + QByteArray::number(i);
		qint64 id = e.attribute(sourceName, "").toLongLong();
		ReadSource* rs = (ReadSource*) m_song->get_source(id);
		if ( ! rs ) {
			PWARN("no audio source returned!");
			m_channels = 0;
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

// FIXME Bogus!
AudioSource* AudioClip::get_audio_source()
{
	return audioSource;
}

void AudioClip::set_track_first_block(nframes_t newTrackFirstBlock)
{
	trackStartFrame=newTrackFirstBlock;

	foreach(ReadSource* source, readSources)
	source->set_source_start_offset(trackStartFrame + sourceStartFrame);

	set_track_end_frame(trackStartFrame + m_length);
	emit trackStartFrameChanged();
}

void AudioClip::set_muted(bool b)
{
	PENTER;
	isMuted=b;
	set_sources_active_state();	
	emit muteChanged(isMuted);
}

void AudioClip::track_mute_changed( bool )
{
	set_sources_active_state();	
}

void AudioClip::set_sources_active_state()
{
	if ( m_track->is_muted() || is_muted() ) {
		foreach(ReadSource* source, readSources)
			source->set_inactive();
	} else {
		foreach(ReadSource* source, readSources)
			source->set_active();
	}	
		
}

void AudioClip::set_left_edge(nframes_t newFrame)
{
	nframes_t olen = sourceEndFrame - sourceStartFrame;
	nframes_t otff = trackStartFrame;
	nframes_t origTrackStartFrame = trackStartFrame;

	trackStartFrame = newFrame;

	if (trackStartFrame > trackEndFrame)
		trackStartFrame = trackEndFrame;

	m_length = olen + otff - newFrame;

	int newSourceStartFrame = (int) (sourceEndFrame - m_length);

	if (newSourceStartFrame < 0) {
		newSourceStartFrame = 0;
		trackStartFrame = origTrackStartFrame;
		m_length = sourceLength;
	}

	sourceStartFrame = (nframes_t) newSourceStartFrame;

	if (sourceStartFrame > sourceLength)
		sourceStartFrame = sourceLength;

	foreach(ReadSource* source, readSources)
		source->set_source_start_offset(trackStartFrame + sourceStartFrame);

	emit edgePositionChanged();
}

void AudioClip::set_right_edge(nframes_t newFrame)
{
	sourceEndFrame = (newFrame - trackStartFrame);

	if (sourceEndFrame > sourceLength)
		sourceEndFrame = sourceLength;

	if (sourceEndFrame < sourceStartFrame)
		sourceEndFrame = sourceStartFrame;

	m_length = sourceEndFrame - sourceStartFrame;

	set_track_end_frame(trackStartFrame + m_length);

	emit edgePositionChanged();
}

void AudioClip::set_first_source_block(nframes_t block)
{
	sourceStartFrame = block;
	m_length = sourceEndFrame - sourceStartFrame;
}

void AudioClip::set_last_source_block(nframes_t block)
{
	sourceEndFrame = block;
	m_length = sourceEndFrame - sourceStartFrame;
	set_track_end_frame(trackStartFrame + m_length);
	emit stateChanged();
}

void AudioClip::set_track_end_frame( nframes_t endFrame )
{
	trackEndFrame = endFrame;
	m_song->update_last_block();
}

nframes_t AudioClip::get_length()
{
	return m_length;
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

int AudioClip::get_baseY()
{
	return m_track->real_baseY() + 3;
}

int AudioClip::get_width()
{
	nframes_t blockWidth = sourceEndFrame - sourceStartFrame;
	int xwidth = (int) ( blockWidth / Peak::zoomStep[m_song->get_hzoom()] );
	return xwidth;
}

int AudioClip::get_height()
{
	return m_track->get_height() - 8;
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

int AudioClip::set_selected()
{
	isSelected = !isSelected;
	emit stateChanged();
	return 1;
}

int AudioClip::process(nframes_t nframes)
{
	if (isRecording) {
		process_capture(nframes);
		return 0;
	}

	nframes_t read_frames =0;
	nframes_t mix_pos;
	float gainFactor = m_gain * m_track->get_gain();

	if (isMuted || (gainFactor == 0.0) || (m_channels == 0)) {
		return 0;
	}

	if ( (trackStartFrame + sourceStartFrame) < m_song->get_transfer_frame()) {
		mix_pos = m_song->get_transfer_frame() - trackStartFrame;
		if (mix_pos > m_length)
			return 0;
	} else {
		return 0;
	}


	AudioBus* bus = m_song->get_master_out();
	int channels = m_channels;
	if (channels > bus->get_channel_count())
		channels = bus->get_channel_count();

	for (int channel=0; channel<channels; channel++) {
		if (m_song->realime_path())
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
		nframes_t written = source->rb_write(captureBus->get_buffer(channel, nframes), m_song->get_transfer_frame(), nframes);
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

	writeSources.clear();

	for (int chan=0; chan<captureBus->get_channel_count(); chan++) {
		ExportSpecification*  spec = new ExportSpecification;

		spec->exportdir = pm().get_project()->get_root_dir() + "/audiosources";
		spec->format = SF_FORMAT_WAV;
		spec->data_width = 1;	// 1 means float
		spec->format |= SF_FORMAT_FLOAT;
		spec->channels = 1;
		spec->sample_rate = audiodevice().get_sample_rate();
		spec->src_quality = SRC_SINC_MEDIUM_QUALITY;
		spec->name = m_name + "-" + QByteArray::number(chan) + ".wav";

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

void AudioClip::resize_buffer( )
{
	delete mixdown;
	mixdown = new audio_sample_t[audiodevice().get_buffer_size()];
}

Command* AudioClip::deselect()
{
	isSelected=false;
	emit stateChanged();
	return (Command*) 0;
}

Command* AudioClip::mute()
{
	return new MuteClip(this);
}

Command* AudioClip::reset_gain()
{
	set_gain(1.0);
	return (Command*) 0;
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

Command* AudioClip::select()
{
	m_song->select_clip(this);
	return (Command*) 0;
}

Command* AudioClip::drag()
{
	return new MoveClip(m_song, this);
}

Command* AudioClip::drag_edge()
{
	int x = cpointer().clip_area_x();
	int cxm = m_song->block_to_xpos( trackStartFrame + ( m_length / 2 ) );


	if (x < cxm)
		return  new  MoveEdge(this, "set_left_edge");
	else
		return new MoveEdge(this, "set_right_edge");

	return (Command*) 0;
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

Command * AudioClip::add_to_selection()
{
	set_selected();
	return (Command*) 0;
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

Peak* AudioClip::get_peak_for_channel( int chan )
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
	set_last_source_block( rs->get_nframes() );
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
			
			add_audio_source(rs, i);
			pm().get_project()->add_audio_source(rs, ws->get_channel());
			
			delete ws;
			ws = 0;
		}
	}
}

void AudioClip::finish_recording()
{
	PENTER;
	
	PWARN("AudioClip :: entering finish_recording()");
	
	foreach(WriteSource* ws, writeSources) {
		PWARN("Setting writesource isRecording to false");
		ws->set_recording(false);
	}

	disconnect(m_song, SIGNAL(transferStopped()), this, SLOT(finish_recording()));
	isRecording = false;
}

int AudioClip::get_channels( )
{
	return m_channels;
}
// eof


