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

$Id: AudioClip.cpp,v 1.54 2006/12/04 19:24:54 r_sijrier Exp $
*/

#include <cfloat>
#include <QInputDialog>

#include "ContextItem.h"
#include "AudioClip.h"
#include "AudioSource.h"
#include "ReadSource.h"
#include "WriteSource.h"
#include "Song.h"
#include "SnapList.h"
#include "Track.h"
#include "AudioChannel.h"
#include <AudioBus.h>
#include <AudioDevice.h>
#include "Mixer.h"
#include "DiskIO.h"
#include "Export.h"
#include "AudioClipManager.h"
#include "AudioSourceManager.h"
#include "Curve.h"
#include "FadeCurve.h"
#include "Tsar.h"
#include "ProjectManager.h"
#include "Peak.h"
#include "ContextPointer.h"
#include "Project.h"
#include "Utils.h"
#include "Information.h"

#include <commands.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AudioClip::AudioClip(const QString& name)
	: ContextItem(),
	  m_name(name)
{
	PENTERCONS;
	m_gain = m_normfactor = 1.0;
	m_length = sourceStartFrame = sourceEndFrame = trackEndFrame = 0;
	isMuted=false;
	m_id = create_id();
	m_readSourceId = 0;
	init();
}


AudioClip::AudioClip(const QDomNode& node)
	: ContextItem()
{
	PENTERCONS;
	QDomNode clipNode = node.firstChild();
	
	// It makes sense to set these values at this time allready
	// they are for example used by the AudioSourcesManager!
	QDomElement e = node.toElement();
	m_id = e.attribute("id", "").toLongLong();
	m_readSourceId = e.attribute("source", "").toLongLong();
	m_name = e.attribute( "clipname", "" ) ;
	isMuted =  e.attribute( "mute", "" ).toInt();
	m_domNode = node;
	init();
}

AudioClip::~AudioClip()
{
	PENTERDES;
	if (m_readSource) {
		m_song->get_diskio()->unregister_read_source(m_readSource);
		delete m_readSource;
	}
}

void AudioClip::init()
{
	m_song = 0;
	m_track = 0;
	m_readSource = 0;
	isRecording = false;
	isSelected = false;
	fadeIn = 0;
	fadeOut = 0;
	m_refcount = 0;
	m_isSnappable = true;
	gainEnvelope = new Curve(this);
}

int AudioClip::set_state(const QDomNode& node)
{
	PENTER;
	
	QDomElement e = node.toElement();

	isTake = e.attribute( "take", "").toInt();
	set_gain( e.attribute( "gain", "" ).toFloat() );
	m_normfactor =  e.attribute( "normfactor", "1.0" ).toFloat();

	isSelected = e.attribute("selected", "0").toInt(); 

	sourceStartFrame = e.attribute( "sourcestart", "" ).toUInt();
	m_length = e.attribute( "length", "0" ).toUInt();
	sourceEndFrame = sourceStartFrame + m_length;
	set_track_start_frame( e.attribute( "trackstart", "" ).toUInt());

	QDomElement curvesNode = node.firstChildElement("Curves");
	if (!curvesNode.isNull()) {
		QDomElement fadeInNode = curvesNode.firstChildElement("FadeIn");
		if (!fadeInNode.isNull()) {
			fadeIn = new FadeCurve(this, "FadeIn");
			fadeIn->set_state( fadeInNode );
			private_add_fade(fadeIn);
		}

		QDomElement fadeOutNode = curvesNode.firstChildElement("FadeOut");
		if (!fadeOutNode.isNull()) {
			fadeOut = new FadeCurve(this, "FadeOut");
			fadeOut->set_state( fadeOutNode );
			private_add_fade(fadeOut);
		}
		
		QDomElement gainEnvelopeNode = curvesNode.firstChildElement("Curve");
		if (!gainEnvelopeNode.isNull()) {
			gainEnvelope->set_state( gainEnvelopeNode );
		}
	}

	return 1;
}

QDomNode AudioClip::get_state( QDomDocument doc )
{
	Q_ASSERT(m_readSource);
	
	QDomElement node = doc.createElement("Clip");
	node.setAttribute("trackstart", trackStartFrame);
	node.setAttribute("sourcestart", sourceStartFrame);
	node.setAttribute("length", m_length);
	node.setAttribute("gain", m_gain);
	node.setAttribute("normfactor", m_normfactor);
	node.setAttribute("mute", isMuted);
	node.setAttribute("take", isTake);
	node.setAttribute("clipname", m_name );
	node.setAttribute("selected", isSelected );
	node.setAttribute("id", m_id );

	node.setAttribute("source", m_readSource->get_id());

	QDomNode curves = doc.createElement("Curves");

	if (fadeIn) {
		curves.appendChild(fadeIn->get_state(doc));
	}
	if (fadeOut) {
		curves.appendChild(fadeOut->get_state(doc));
	}
	curves.appendChild(gainEnvelope->get_state(doc));

	node.appendChild(curves);

	return node;
}

void AudioClip::toggle_mute()
{
	PENTER;
	isMuted=!isMuted;
	set_sources_active_state();
	emit muteChanged();
}

void AudioClip::track_audible_state_changed()
{
	set_sources_active_state();
}

void AudioClip::set_sources_active_state()
{
	if (! m_track) {
		return;
	}
	
	if (! m_readSource) {
		return;
	}
	
	if ( m_track->is_muted() || m_track->is_muted_by_solo() || is_muted() ) {
			m_readSource->set_inactive();
	} else {
			m_readSource->set_active();
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
// 	PWARN("trackEndFrame is %d", endFrame);
	trackEndFrame = endFrame;
	emit trackEndFrameChanged();
}

void AudioClip::set_blur(bool )
{
	emit stateChanged();
}

void AudioClip::set_fade_in(nframes_t b)
{
	if (!fadeIn) {
		create_fade_in();
	}
	fadeIn->set_range( b );
}

void AudioClip::set_fade_out(nframes_t b)
{
	if (!fadeOut) {
		create_fade_out();
	}
	fadeOut->set_range( b );
}

void AudioClip::set_gain(float gain)
{
	PENTER3;
	if (gain < 0.0)
		gain = 0.0;
	if (gain > 32.0)
		gain = 32.0;
	m_gain = gain;
	emit gainChanged();
}

int AudioClip::set_selected(bool selected)
{
	if (isSelected != selected) {
		isSelected = selected;
		emit stateChanged();
	}
	return 1;
}

//
//  Function called in RealTime AudioThread processing path
//
int AudioClip::process(nframes_t nframes, audio_sample_t* mixdown, uint channel)
{
	Q_ASSERT(m_song);
	
	if (isRecording) {
		process_capture(nframes, channel);
		return 0;
	}


	if (channel >= m_readSource->get_channel_count()) {
		return -1;	// Channel doesn't exist!!
	}

	bool showdebug = false;

	if ( ((m_song->get_transport_frame() < (548864 + 3000))) && (channel == 0)) {
// 		showdebug = true;
	}

	if (showdebug) {
		printf("%s\n", m_name.toAscii().data());
		printf("trackStartFrame is %d\n", trackStartFrame);
		printf("trackEndFrame is %d\n", trackEndFrame);
		printf("song->transport is %d\n", m_song->get_transport_frame());
		printf("diff trackEndFrame, song->transport is %d\n", (int)trackEndFrame - m_song->get_transport_frame());
		printf("diff trackStartFrame, song->transport is %d\n", (int)trackStartFrame - m_song->get_transport_frame());
	}

	nframes_t mix_pos;

	if (showdebug) printf("clip trackEndFrame - song->transport_frame is %d\n", trackEndFrame - m_song->get_transport_frame());

	if ( (trackStartFrame <= (m_song->get_transport_frame())) && (trackEndFrame > (m_song->get_transport_frame())) ) {
		mix_pos = m_song->get_transport_frame() - trackStartFrame + sourceStartFrame;
		if (showdebug) {
			printf("mix_pos is %d\n", mix_pos);
		}
	} else {
		if (showdebug) {
			printf("Not processing this Clip\n\n");
			printf("END %s\n\n", m_name.toAscii().data());
		}

		return 0;
	}


	if (isMuted || ( (m_gain * m_normfactor) == 0.0f) ) {
		return 0;
	}


	nframes_t read_frames = 0;


	if (m_song->realtime_path()) {
		read_frames = m_readSource->rb_read(channel, mixdown, mix_pos, nframes);
	} else {
		read_frames = m_readSource->file_read(channel, mixdown, mix_pos, nframes);
	}

	if (showdebug) {
		printf("read frames is %d\n", read_frames);
		printf("END %s\n\n", m_name.toAscii().data());
	}


	if (read_frames == 0) {
		return 0;
	}


	for (int i=0; i<m_fades.size(); ++i) {
		m_fades.at(i)->process(mixdown, read_frames);
	}
	
	nframes_t gainEnvelopeMixPos = m_song->get_transport_frame() - trackStartFrame;
	gainEnvelope->get_vector(gainEnvelopeMixPos, gainEnvelopeMixPos + read_frames, m_song->gainbuffer, read_frames);
	
	for (nframes_t n = 0; n < read_frames; ++n) {
		mixdown[n] *= m_song->gainbuffer[n];
	}


	return 1;
}

//
//  Function called in RealTime AudioThread processing path
//
void AudioClip::process_capture( nframes_t nframes, uint channel )
{
	WriteSource* source = writeSources.at(channel);

	nframes_t written = source->rb_write(captureBus->get_buffer(channel, nframes), nframes);

	if (written != nframes) {
		PWARN("couldn't write nframes to buffer, only %d", written);
	}
}

int AudioClip::init_recording( QByteArray name )
{
	Q_ASSERT(m_song);
	Q_ASSERT(m_track);
	
	captureBus = audiodevice().get_capture_bus(name);

	if (!captureBus) {
		info().warning( tr("Unable to Record to Track") );
		info().information( tr("AudioDevice doesn't have this Capture Bus: %1 (Track %2)").
				arg(name.data()).arg(m_track->get_id()) );
		return -1;
	}


	for (int chan=0; chan<captureBus->get_channel_count(); chan++) {
		ExportSpecification*  spec = new ExportSpecification;

		spec->exportdir = pm().get_project()->get_root_dir() + "/audiosources/";
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

		spec->name = songid + m_name;

		spec->dataF = captureBus->get_buffer( chan, audiodevice().get_buffer_size());

		WriteSource* ws = new WriteSource(spec, chan, captureBus->get_channel_count());
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
	return new ClipSelection(this, "remove_from_selection", tr("Selection: Remove Clip"));
}

Command * AudioClip::add_to_selection()
{
	return new ClipSelection(this, "add_to_selection", tr ("Selection: Add Clip"));
}

Command* AudioClip::select()
{
	return new ClipSelection(this, "select_clip", tr("Select Clip"));
}

Command* AudioClip::mute()
{
	return new PCommand(this, "toggle_mute", tr("Toggle Mute"));
}

Command* AudioClip::reset_gain()
{
	return new Gain(this, tr("Clip Gain"), 1.0);
}

Command* AudioClip::reset_fade_in()
{
	set_fade_in(1);
	return (Command*) 0;
}

Command* AudioClip::reset_fade_out()
{
	set_fade_out(1);
	return (Command*) 0;
}

Command* AudioClip::reset_fade_both()
{
	set_fade_in(1);
	set_fade_out(1);
	return (Command*) 0;
}

Command* AudioClip::gain()
{
	return new Gain(this, tr("Clip Gain"));
}

Command* AudioClip::copy()
{
	Q_ASSERT(m_song);
	return new CopyClip(m_song, this);
}

AudioClip * AudioClip::prev_clip( )
{
	Q_ASSERT(m_track);
	return m_track->get_cliplist().prev(this);
}

AudioClip * AudioClip::next_clip( )
{
	Q_ASSERT(m_track);
	return m_track->get_cliplist().next(this);
}

AudioClip* AudioClip::create_copy( )
{
	PENTER;
	Q_ASSERT(m_song);
	Q_ASSERT(m_track);
	QDomDocument doc("AudioClip");
	QDomNode clipState = get_state(doc);
	AudioClip* clip = new AudioClip(clipState);
	clip->set_song(m_song);
	clip->set_name( clip->get_name().prepend(tr("Copy of - ")) );
	clip->set_state(clipState);
	return clip;
}

Peak* AudioClip::get_peak_for_channel( int chan ) const
{
	PENTER2;
	Q_ASSERT(m_readSource);
	return m_readSource->get_peak(chan);
}

void AudioClip::set_audio_source(ReadSource* rs)
{
	PENTER;
	
	m_readSource = rs;
	sourceLength = rs->get_nframes();

	// If m_length isn't set yet, it means we are importing stuff instead of reloading from project file.
	// it's a bit weak this way, hopefull I'll get up something better in the future.
	// The positioning-length-offset and such stuff is still a bit weak :(
	if (m_length == 0) {
		sourceEndFrame = rs->get_nframes();
		m_length = sourceEndFrame;
	}

	set_track_end_frame( trackStartFrame + sourceLength - sourceStartFrame);

	set_sources_active_state();

	rs->set_audio_clip(this);

	emit stateChanged();
}

void AudioClip::finish_write_source( WriteSource * ws )
{
	PENTER;

//  	printf("AudioClip::finish_write_source :  thread id is: %ld\n", QThread::currentThreadId ());

	QString dir;
	QString name;
	
	for (int i=0; i<writeSources.size(); ++i) {
		if (ws == writeSources.at(i)) {
			writeSources.removeAt(i);
			dir = ws->get_dir();
			name = ws->get_name();
			delete ws;
		}
	}
	
	if (writeSources.isEmpty()) {
		int channelCount, fileCount;
		channelCount = fileCount = captureBus->get_channel_count(); 
		
		AudioSourceManager* asmanager = pm().get_project()->get_audiosource_manager();
		ReadSource* rs = asmanager->new_readsource(dir,
							name,
							channelCount,
							fileCount,
							m_song->get_id(), 
							audiodevice().get_bit_depth(), 
							audiodevice().get_sample_rate() );
		if (rs) {
			set_audio_source(rs);
			m_song->get_diskio()->register_read_source( m_readSource );
		} else {
			PERROR("No ReadSource returned from asm after recording");
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
	if (m_readSource) {
		return m_readSource->get_channel_count();
	} else {
		if (writeSources.size() > 0) {
			writeSources.at(0)->get_channel_count();
		}
	}
	
	return 0;
}

Song* AudioClip::get_song( ) const
{
	Q_ASSERT(m_song);
	return m_song;
}

Track* AudioClip::get_track( ) const
{
	Q_ASSERT(m_track);
	return m_track;
}

void AudioClip::set_song( Song * song )
{
	m_song = song;
	if (m_readSource) {
		m_song->get_diskio()->register_read_source( m_readSource );
	}
	set_history_stack(m_song->get_history_stack());
	gainEnvelope->set_history_stack(get_history_stack());

	
	if (isSelected) {
		m_song->get_audioclip_manager()->add_to_selection( this );
	}
}


void AudioClip::set_track( Track * track )
{
	if (m_track) {
		disconnect(m_track, SIGNAL(audibleStateChanged()), this, SLOT(track_audible_state_changed()));
	}
	
	m_track = track;
	
	connect(m_track, SIGNAL(audibleStateChanged()), this, SLOT(track_audible_state_changed()));
	set_sources_active_state();
}

void AudioClip::set_name( const QString& name )
{
	m_name = name;
}

float AudioClip::get_gain( ) const
{
	return m_gain;
}

float AudioClip::get_norm_factor( ) const
{
	return m_normfactor;
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
	if (m_readSource) {
		return m_readSource->get_bit_depth();
	}
	
	return audiodevice().get_bit_depth();

}

int AudioClip::get_rate( ) const
{
	if (m_readSource) {
		return m_readSource->get_rate();
	}
	
	return audiodevice().get_sample_rate();
}

nframes_t AudioClip::get_source_length( ) const
{
	return sourceLength;
}

nframes_t AudioClip::get_length() const
{
	return m_length;
}

bool AudioClip::is_recording( ) const
{
	return isRecording;
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


Command * AudioClip::clip_fade_in( )
{
	int direction = 1;
	if (!fadeIn) {
		create_fade_in();
	}
	return new Fade(this, fadeIn, direction);
}

Command * AudioClip::clip_fade_out( )
{
	int direction = -1;
	if (!fadeOut) {
		create_fade_out();
	}
	return new Fade(this, fadeOut, direction);
}

Command * AudioClip::normalize( )
{
        bool ok;
        double d = QInputDialog::getDouble(0, tr("Normalization"),
                                           tr("Set Normalization level:"), 0.0, -120, 0, 1, &ok);
        if (ok)
		calculate_normalization_factor(d);

	// Hmm, this is not entirely true, but "almost" ;-)
	emit gainChanged();

	return (Command*) 0;
}

Command * AudioClip::denormalize( )
{
	m_normfactor = 1.0;
	// Hmm, this is not entirely true, but "almost" ;-)
	emit gainChanged();

	return (Command*) 0;
}

void AudioClip::calculate_normalization_factor(float targetdB)
{
	double maxamp = 0;

	float target = dB_to_scale_factor (targetdB);

	if (target == 1.0f) {
		/* do not normalize to precisely 1.0 (0 dBFS), to avoid making it appear
		   that we may have clipped.
		*/
		target -= FLT_EPSILON;
	}

	for (uint i=0; i<m_readSource->get_channel_count(); ++i) {
		maxamp = f_max(m_readSource->get_peak(i)->get_max_amplitude(sourceStartFrame, sourceEndFrame), maxamp);
	}

	if (maxamp == 0.0f) {
		/* don't even try */
		return;
	}

	if (maxamp == target) {
		/* we can't do anything useful */
		return;
	}

	/* compute scale factor */
	m_normfactor = target/maxamp;
}

FadeCurve * AudioClip::get_fade_in( )
{
	return fadeIn;
}

FadeCurve * AudioClip::get_fade_out( )
{
	return fadeOut;
}

void AudioClip::private_add_fade( FadeCurve* fade )
{
	m_fades.append(fade);
}

void AudioClip::private_remove_fade( FadeCurve * fade )
{
	m_fades.append(fade);
}

qint64 AudioClip::get_id( ) const
{
	return m_id;
}


int AudioClip::ref( )
{
	return m_refcount++;
}

int AudioClip::get_ref_count( ) const
{
	return m_refcount;
}

void AudioClip::create_fade_in( )
{
	fadeIn = new FadeCurve(this, "FadeIn");
	fadeIn->set_shape("Linear");
	THREAD_SAVE_CALL_EMIT_SIGNAL(this, fadeIn, private_add_fade(FadeCurve*), fadeAdded(FadeCurve*));
}

void AudioClip::create_fade_out( )
{
	fadeOut = new FadeCurve(this, "FadeOut");
	fadeOut->set_shape("Linear");
	THREAD_SAVE_CALL_EMIT_SIGNAL(this, fadeOut, private_add_fade(FadeCurve*), fadeAdded(FadeCurve*));
}

void AudioClip::set_snappable( bool snap )
{
	m_isSnappable = snap;
	m_song->get_snap_list()->mark_dirty();
}


// eof

