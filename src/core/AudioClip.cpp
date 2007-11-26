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

#include <cfloat>
#include <QInputDialog>

#include "ContextItem.h"
#include "ReadSource.h"
#include "AudioClip.h"
#include "AudioSource.h"
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
#include "ResourcesManager.h"
#include "Curve.h"
#include "FadeCurve.h"
#include "Tsar.h"
#include "ProjectManager.h"
#include "Peak.h"
#include "ContextPointer.h"
#include "Project.h"
#include "Utils.h"
#include "Information.h"
#include <Config.h>
#include "PluginChain.h"
#include "GainEnvelope.h"

#include "AbstractAudioReader.h"

#include <commands.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


AudioClip::AudioClip(const QString& name)
	: ContextItem()
	, Snappable()
	, m_name(name)
{
	PENTERCONS;
	m_isMuted=false;
	m_id = create_id();
	m_readSourceId = m_songId = 0;
	init();
}


AudioClip::AudioClip(const QDomNode& node)
	: ContextItem()
	, Snappable()
	, m_track(0)
	, m_readSource(0)
{
	PENTERCONS;
	QDomNode clipNode = node.firstChild();
	
	// It makes sense to set these values at this time allready
	// they are for example used by the ResourcesManager!
	QDomElement e = node.toElement();
	m_id = e.attribute("id", "").toLongLong();
	m_readSourceId = e.attribute("source", "").toLongLong();
	m_songId = e.attribute("sheet", "0").toLongLong();
	m_name = e.attribute( "clipname", "" ) ;
	m_isMuted =  e.attribute( "mute", "" ).toInt();
	// FIXME!!!!!!!
	bool ok;
	m_length = TimeRef(e.attribute( "length", "0" ).toLongLong(&ok));
	m_sourceStartLocation = TimeRef(e.attribute( "sourcestart", "" ).toLongLong(&ok));
	
	m_sourceEndLocation = m_sourceStartLocation + m_length;
	TimeRef location(e.attribute( "trackstart", "" ).toLongLong(&ok));
	set_track_start_location(location);
	m_domNode = node.cloneNode();
	init();
}

AudioClip::~AudioClip()
{
	PENTERDES;
	if (m_readSource) {
		m_song->get_diskio()->unregister_read_source(m_readSource);
		delete m_readSource;
	}
	if (m_peak) {
		m_peak->close();
	}
}

void AudioClip::init()
{
	m_song = 0;
	m_track = 0;
	m_readSource = 0;
	m_peak = 0;
	m_recordingStatus = NO_RECORDING;
	m_isSelected = m_isReadSourceValid = false;
	m_isLocked = config().get_property("AudioClip", "LockByDefault", false).toBool();
	fadeIn = 0;
	fadeOut = 0;
	m_pluginChain = new PluginChain(this);
	m_fader = m_pluginChain->get_fader();
	m_fader->automate_port(0, true);
	m_fader->set_gain(1.0);
}

int AudioClip::set_state(const QDomNode& node)
{
	PENTER;
	
	Q_ASSERT(m_song);
	
	QDomElement e = node.toElement();

	m_isTake = e.attribute( "take", "").toInt();
	set_gain( e.attribute( "gain", "" ).toFloat() );
	m_isLocked = e.attribute( "locked", "0" ).toInt();

	if (e.attribute("selected", "0").toInt() == 1) {
		m_song->get_audioclip_manager()->select_clip(this);
	}

	m_readSourceId = e.attribute("source", "").toLongLong();
	m_songId = e.attribute("sheet", "0").toLongLong();
	m_isMuted =  e.attribute( "mute", "" ).toInt();

	bool ok;
	m_sourceStartLocation = TimeRef(e.attribute( "sourcestart", "" ).toLongLong(&ok));
	m_length = TimeRef(e.attribute( "length", "0" ).toLongLong(&ok));
	m_sourceEndLocation = m_sourceStartLocation + m_length;
	TimeRef location(e.attribute( "trackstart", "" ).toLongLong(&ok));
	set_track_start_location(location);
	
	emit stateChanged();
	
	QDomElement fadeInNode = node.firstChildElement("FadeIn");
	if (!fadeInNode.isNull()) {
		if (!fadeIn) {
			fadeIn = new FadeCurve(this, m_song, "FadeIn");
			fadeIn->set_history_stack(get_history_stack());
			private_add_fade(fadeIn);
		}
		fadeIn->set_state( fadeInNode );
	}

	QDomElement fadeOutNode = node.firstChildElement("FadeOut");
	if (!fadeOutNode.isNull()) {
		if (!fadeOut) {
			fadeOut = new FadeCurve(this, m_song, "FadeOut");
			fadeOut->set_history_stack(get_history_stack());
			private_add_fade(fadeOut);
		}
		fadeOut->set_state( fadeOutNode );
	}

	QDomNode pluginChainNode = node.firstChildElement("PluginChain");
	if (!pluginChainNode.isNull()) {
		m_pluginChain->set_state(pluginChainNode);
	}
	
	return 1;
}

QDomNode AudioClip::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("Clip");
	node.setAttribute("trackstart", m_trackStartLocation.universal_frame());
	node.setAttribute("sourcestart", m_sourceStartLocation.universal_frame());
	node.setAttribute("length", m_length.universal_frame());
	node.setAttribute("mute", m_isMuted);
	node.setAttribute("take", m_isTake);
	node.setAttribute("clipname", m_name );
	node.setAttribute("selected", m_isSelected );
	node.setAttribute("id", m_id );
	node.setAttribute("sheet", m_songId );
	node.setAttribute("locked", m_isLocked);

	node.setAttribute("source", m_readSourceId);

	if (fadeIn) {
		node.appendChild(fadeIn->get_state(doc));
	}
	if (fadeOut) {
		node.appendChild(fadeOut->get_state(doc));
	}

	QDomNode pluginChainNode = doc.createElement("PluginChain");
	pluginChainNode.appendChild(m_pluginChain->get_state(doc));
	node.appendChild(pluginChainNode);
	
	return node;
}

void AudioClip::toggle_mute()
{
	PENTER;
	m_isMuted=!m_isMuted;
	set_sources_active_state();
	emit muteChanged();
}

void AudioClip::toggle_lock()
{
	m_isLocked = !m_isLocked;
	emit lockChanged();
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
		m_readSource->set_active(false);
	} else {
		m_readSource->set_active(true);
	}

}

void AudioClip::set_left_edge(TimeRef newLeftLocation)
{
	if (newLeftLocation < qint64(0)) {
		newLeftLocation = TimeRef();
	}
	
	if (newLeftLocation < m_trackStartLocation) {

		TimeRef availableTimeLeft = m_sourceStartLocation;

		TimeRef movingToLeft = m_trackStartLocation - newLeftLocation;

		if (movingToLeft > availableTimeLeft) {
			movingToLeft = availableTimeLeft;
		}

		m_trackStartLocation -= movingToLeft;
		set_source_start_location( m_sourceStartLocation - movingToLeft );

	} else if (newLeftLocation > m_trackStartLocation) {

		TimeRef availableTimeRight = m_length;

		TimeRef movingToRight = newLeftLocation - m_trackStartLocation;

		if (movingToRight > (availableTimeRight - TimeRef(4, get_rate())) ) {
			movingToRight = (availableTimeRight - TimeRef(4, get_rate()));
		}

		m_trackStartLocation += movingToRight;
		set_source_start_location( m_sourceStartLocation + movingToRight );

	} else {
		return;
	}

	emit positionChanged(this);
}

void AudioClip::set_right_edge(TimeRef newRightLocation)
{
	if (newRightLocation < TimeRef()) {
		newRightLocation = TimeRef();
	}
	
	if (newRightLocation > m_trackEndLocation) {

		TimeRef availableTimeRight = m_sourceLength - m_sourceEndLocation;

		TimeRef movingToRight = newRightLocation - m_trackEndLocation;

		if (movingToRight > availableTimeRight) {
			movingToRight = availableTimeRight;
		}

		set_track_end_location( m_trackEndLocation + movingToRight );
		set_source_end_location( m_sourceEndLocation + movingToRight );

	} else if (newRightLocation < m_trackEndLocation) {

		TimeRef availableTimeLeft = m_length;

		TimeRef movingToLeft = m_trackEndLocation - newRightLocation;

		if (movingToLeft > availableTimeLeft - TimeRef(4, get_rate())) {
			movingToLeft = availableTimeLeft - TimeRef(4, get_rate());
		}

		set_track_end_location( m_trackEndLocation - movingToLeft );
		set_source_end_location( m_sourceEndLocation - movingToLeft);

	} else {
		return;
	}

	emit positionChanged(this);
}

void AudioClip::set_source_start_location(const TimeRef& location)
{
	m_sourceStartLocation = location;
	m_length = m_sourceEndLocation - m_sourceStartLocation;
}

void AudioClip::set_source_end_location(const TimeRef& location)
{
	m_sourceEndLocation = location;
	m_length = m_sourceEndLocation - m_sourceStartLocation;
}

void AudioClip::set_track_start_location(const TimeRef& location)
{
	m_trackStartLocation = location;
	set_track_end_location(m_trackStartLocation + m_length);
	if (m_track) {
		THREAD_SAVE_INVOKE(m_track, this, clip_position_changed(AudioClip*));
	}
	emit positionChanged(this);
}

void AudioClip::set_track_end_location(const TimeRef& location)
{
// 	PWARN("m_trackEndLocation is %d", endFrame);
	m_trackEndLocation = location;
	emit trackEndLocationChanged();
}

void AudioClip::set_fade_in(double range)
{
	if (!fadeIn) {
		create_fade_in();
	}
	fadeIn->set_range(range);
}

void AudioClip::set_fade_out(double range)
{
	if (!fadeOut) {
		create_fade_out();
	}
	fadeOut->set_range(range);
}

void AudioClip::set_gain(float gain)
{
	PENTER3;
	if (gain < 0.0)
		gain = 0.0;
	if (gain > 32.0)
		gain = 32.0;
	
	m_fader->set_gain(gain);
	emit stateChanged();
}

void AudioClip::set_selected(bool selected)
{
	m_isSelected = selected;
	emit stateChanged();
}

//
//  Function called in RealTime AudioThread processing path
//
int AudioClip::process(nframes_t nframes)
{
	Q_ASSERT(m_song);
	
	if (m_recordingStatus == RECORDING) {
		process_capture(nframes);
		return 0;
	}

	if (!m_isReadSourceValid) {
		return -1;
	}

	if (m_isMuted || (get_gain() == 0.0f) ) {
		return 0;
	}
	
	Q_ASSERT(m_readSource);
	
	AudioBus* bus = m_song->get_clip_render_bus();
	AudioBus* sendbus = m_song->get_render_bus();
	
	TimeRef mix_pos;
	int channelcount = get_channels();
	audio_sample_t* mixdown[channelcount];


	TimeRef transportLocation = m_song->get_transport_location();
	TimeRef upperRange = transportLocation + TimeRef(nframes, get_rate());
	
	if ( (m_trackStartLocation < upperRange) && (m_trackEndLocation > transportLocation) ) {
		if (transportLocation < m_trackStartLocation) {
			uint offset = (m_trackStartLocation - transportLocation).to_frame(get_rate());
			mix_pos = m_sourceStartLocation;
			
			for (int chan=0; chan<bus->get_channel_count(); ++chan) {
				mixdown[chan] = bus->get_buffer(chan, nframes) + offset;
			}
			nframes = nframes - offset;
		} else {
			mix_pos = (transportLocation - m_trackStartLocation + m_sourceStartLocation);
			
			for (int chan=0; chan<bus->get_channel_count(); ++chan) {
				mixdown[chan] = bus->get_buffer(chan, nframes);
			}
		}
		if (m_trackEndLocation < upperRange) {
			nframes -= (upperRange - m_trackEndLocation).to_frame(get_rate());
		}
	} else {
		return 0;
	}


	int read_frames = 0;


	if (m_song->realtime_path()) {
		read_frames = m_readSource->rb_read(mixdown, mix_pos, nframes);
	} else {
		read_frames = m_readSource->file_read(m_song->renderDecodeBuffer, mix_pos, nframes);
		if (read_frames > 0) {
			for (int chan=0; chan<channelcount; ++chan) {
				memcpy(mixdown[chan], m_song->renderDecodeBuffer->destination[chan], read_frames * sizeof(audio_sample_t));
			}
		}
	}
	
	if (read_frames <= 0) {
		return 0;
	}
	

	apill_foreach(FadeCurve* fade, FadeCurve, m_fades) {
		fade->process(mixdown, read_frames, channelcount);
	}
	
	TimeRef endlocation = mix_pos + TimeRef(read_frames, get_rate());
	m_fader->process_gain(mixdown, mix_pos, endlocation, read_frames, channelcount);
	
	if (channelcount == 1) {
		Mixer::mix_buffers_no_gain(sendbus->get_buffer(0, read_frames), bus->get_buffer(0, read_frames), read_frames);
		Mixer::mix_buffers_no_gain(sendbus->get_buffer(1, read_frames), bus->get_buffer(0, read_frames), read_frames);
	} else if (channelcount == 2) {
		Mixer::mix_buffers_no_gain(sendbus->get_buffer(0, read_frames), bus->get_buffer(0, read_frames), read_frames);
		Mixer::mix_buffers_no_gain(sendbus->get_buffer(1, read_frames), bus->get_buffer(1, read_frames), read_frames);
	}
	
	return 1;
}

//
//  Function called in RealTime AudioThread processing path
//
void AudioClip::process_capture(nframes_t nframes)
{
	if (!m_captureBus) {
		return;
	}
	
	m_length.add_frames(nframes, get_rate());
	nframes_t written = 0;
	
	if (m_track->capture_left_channel() && m_track->capture_right_channel()) {
		audio_sample_t* buffer[2];
		buffer[0] = m_captureBus->get_buffer(0, nframes);
		buffer[1] = m_captureBus->get_buffer(1, nframes);
		written = m_recorder->rb_write(buffer, nframes);
	} else if (m_track->capture_left_channel()) {
		audio_sample_t* buffer[1];
		buffer[0] = m_captureBus->get_buffer(0, nframes);
		written = m_recorder->rb_write(buffer, nframes);
	} else if (m_track->capture_right_channel()) {
		audio_sample_t* buffer[1];
		buffer[0] = m_captureBus->get_buffer(1, nframes);
		written = m_recorder->rb_write(buffer, nframes);
	}
	
	if (written != nframes) {
		printf("couldn't write nframes %d to recording buffer for channel 0, only %d\n", nframes, written);
	}
}

int AudioClip::init_recording( QByteArray name )
{
	Q_ASSERT(m_song);
	Q_ASSERT(m_track);
	
	m_captureBusName = name;
	get_capture_bus();

	if (!m_captureBus) {
		info().critical(tr("Unable to Record to Track"));
		info().warning(tr("AudioDevice doesn't have this Capture Bus: %1 (Track %2)").
				arg(name.data()).arg(m_track->get_id()) );
		return -1;
	}

	m_sourceStartLocation = TimeRef();
	m_isTake = 1;
	m_recordingStatus = RECORDING;
	int channelcount;
	
	if (m_track->capture_left_channel() && m_track->capture_right_channel()) {
		channelcount = 2;
	} else {
		channelcount = 1;
	}
	
	ReadSource* rs = resources_manager()->create_recording_source(
				pm().get_project()->get_root_dir() + "/audiosources/",
				m_name, channelcount, m_song->get_id());
	
	resources_manager()->set_source_for_clip(this, rs);
	
	QString sourceid = QString::number(rs->get_id());
	
	ExportSpecification* spec = new ExportSpecification;

	spec->exportdir = pm().get_project()->get_root_dir() + "/audiosources/";
	
	QString recordFormat = config().get_property("Recording", "FileFormat", "wav").toString();
	if (recordFormat == "wavpack") {
		spec->writerType = "wavpack";
		QString compression = config().get_property("Recording", "WavpackCompressionType", "fast").toString();
		QString skipwvx = config().get_property("Recording", "WavpackSkipWVX", "false").toString();
		spec->extraFormat["quality"] = compression;
		spec->extraFormat["skip_wvx"] = skipwvx;
	}
	else if (recordFormat == "w64") {
		spec->writerType = "sndfile";
		spec->extraFormat["filetype"] = "w64";
	} else {
		spec->writerType = "sndfile";
		spec->extraFormat["filetype"] = "wav";
	}
	
	spec->data_width = 1;	// 1 means float
	spec->channels = channelcount;
	spec->sample_rate = audiodevice().get_sample_rate();
	spec->src_quality = SRC_SINC_MEDIUM_QUALITY;
	spec->isRecording = true;
	spec->startLocation = TimeRef();
	spec->endLocation = TimeRef();
	spec->totalTime = TimeRef();
	spec->blocksize = audiodevice().get_buffer_size();
	spec->name = m_name + "-" + sourceid;
	spec->dataF = m_captureBus->get_buffer(0, audiodevice().get_buffer_size());

	m_recorder = new WriteSource(spec);
	if (m_recorder->prepare_export() == -1) {
		delete m_recorder;
		delete spec;
		spec = 0;
		return -1;
	}
	m_recorder->set_process_peaks( true );
	m_recorder->set_recording( true );
	
	m_song->get_diskio()->register_write_source(m_recorder);
	
	connect(m_recorder, SIGNAL(exportFinished()), this, SLOT(finish_write_source()));
	connect(m_song, SIGNAL(transferStopped()), this, SLOT(finish_recording()));
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(get_capture_bus()));

	return 1;
}

Command* AudioClip::mute()
{
	PCommand* cmd = new PCommand(this, "toggle_mute", tr("Toggle Mute"));
	cmd->set_historable(false);
	return cmd;
}

Command* AudioClip::lock()
{
	PCommand* cmd = new PCommand(this, "toggle_lock", tr("Toggle Lock"));
	cmd->set_historable(false);
	return cmd;
}

Command* AudioClip::reset_fade_in()
{
	return new FadeRange(this, fadeIn, 1.0);
}

Command* AudioClip::reset_fade_out()
{
	return new FadeRange(this, fadeOut, 1.0);
}

Command* AudioClip::reset_fade_both()
{
	CommandGroup* group = new CommandGroup(this, tr("Reset Fades"));
	group->add_command(reset_fade_in());
	group->add_command(reset_fade_out());

	return group;
}

AudioClip* AudioClip::create_copy( )
{
	PENTER;
	Q_ASSERT(m_song);
	Q_ASSERT(m_track);
	QDomDocument doc("AudioClip");
	QDomNode clipState = get_state(doc);
	AudioClip* clip = new AudioClip(m_name);
	clip->set_song(m_song);
	clip->set_track(m_track);
	clip->set_state(clipState);
	return clip;
}

void AudioClip::set_audio_source(ReadSource* rs)
{
	PENTER;
	
	if (!rs) {
		m_isReadSourceValid = false;
		return;
	}
	
	if (rs->get_error() < 0) {
		m_isReadSourceValid = false;
	} else {
		m_isReadSourceValid = true;
	}
		
	m_readSource = rs;
	m_readSourceId = rs->get_id();
	m_sourceLength = rs->get_length();

	// If m_length isn't set yet, it means we are importing stuff instead of reloading from project file.
	// it's a bit weak this way, hopefull I'll get up something better in the future.
	// The positioning-length-offset and such stuff is still a bit weak :(
	// NOTE: don't change, audio recording (finish_writesource()) assumes there is checked for length == 0 !!!
	if (m_length == TimeRef()) {
		m_sourceEndLocation = rs->get_length();
		m_length = m_sourceEndLocation;
	}

	set_track_end_location(m_trackStartLocation + m_sourceLength - m_sourceStartLocation);

	set_sources_active_state();

	rs->set_audio_clip(this);
	
	
	if (m_recordingStatus == NO_RECORDING) {
		if (m_peak) {
			m_peak->close();
		}
		m_peak = new Peak(rs);
	}

	emit stateChanged();
}

void AudioClip::finish_write_source()
{
	PENTER;
	
	Q_ASSERT(m_readSource);
	
	if (m_readSource->set_file(m_recorder->get_filename()) < 0) {
		PERROR("Setting file for ReadSource failed after finishing recording");
	} else {
		m_song->get_diskio()->register_read_source(m_readSource);
		// re-inits the lenght from the audiofile due calling rsm->set_source_for_clip()
		m_length = TimeRef();
	}
	
	delete m_recorder;
	m_recorder = 0;
	
	m_recordingStatus = NO_RECORDING;
	
	resources_manager()->set_source_for_clip(this, m_readSource);
	
	emit recordingFinished();
}

void AudioClip::finish_recording()
{
	PENTER;
	
	m_recordingStatus = FINISHING_RECORDING;
	m_recorder->set_recording(false);

	disconnect(m_song, SIGNAL(transferStopped()), this, SLOT(finish_recording()));
	disconnect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(get_capture_bus()));
}

int AudioClip::get_channels( ) const
{
	if (m_readSource) {
		return m_readSource->get_channel_count();
	} else {
		if (m_recorder) {
			return m_recorder->get_channel_count();
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
	if (m_readSource && m_isReadSourceValid) {
		m_song->get_diskio()->register_read_source( m_readSource );
	} else {
		PWARN("AudioClip::set_song() : Setting Song, but no ReadSource available!!");
	}
	
	m_songId = song->get_id();
	
	set_history_stack(m_song->get_history_stack());
	m_pluginChain->set_song(m_song);
	set_snap_list(m_song->get_snap_list());
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
	emit stateChanged();
}

bool AudioClip::is_selected( ) const
{
	return m_isSelected;
}

bool AudioClip::is_take( ) const
{
	return m_isTake;
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

TimeRef& AudioClip::get_source_length( ) const
{
	return m_sourceLength;
}

TimeRef& AudioClip::get_length() const
{
	return m_length;
}

int AudioClip::recording_state( ) const
{
	return m_recordingStatus;
}

Command * AudioClip::normalize( )
{
        bool ok;
        double d = QInputDialog::getDouble(0, tr("Normalization"),
                                           tr("Set Normalization level:"), 0.0, -120, 0, 1, &ok);
        if (ok) {
		calculate_normalization_factor(d);
	}

	return (Command*) 0;
}

Command * AudioClip::denormalize( )
{
	set_gain(1.0);
	return (Command*) 0;
}

void AudioClip::calculate_normalization_factor(float targetdB)
{
	float target = dB_to_scale_factor (targetdB);

	if (target == 1.0f) {
		/* do not normalize to precisely 1.0 (0 dBFS), to avoid making it appear
		   that we may have clipped.
		*/
		target -= FLT_EPSILON;
	}

	double maxamp = m_peak->get_max_amplitude(m_sourceStartLocation, m_sourceEndLocation);
	
	if (maxamp == 0.0f) {
		printf("AudioClip::normalization: max amplitude == 0\n");
		/* don't even try */
		return;
	}
	
	if (maxamp == target) {
		printf("AudioClip::normalization: max amplitude == target amplitude\n");
		/* we can't do anything useful */
		return;
	}

	/* compute scale factor */
	set_gain(target/maxamp);
}

FadeCurve * AudioClip::get_fade_in( ) const
{
	return fadeIn;
}

FadeCurve * AudioClip::get_fade_out( ) const
{
	return fadeOut;
}

void AudioClip::private_add_fade( FadeCurve* fade )
{
	m_fades.append(fade);
	
	if (fade->get_fade_type() == FadeCurve::FadeIn) {
		fadeIn = fade;
	} else if (fade->get_fade_type() == FadeCurve::FadeOut) {
		fadeOut = fade;
	}
}

void AudioClip::private_remove_fade( FadeCurve * fade )
{
	if (fade == fadeIn) {
		fadeIn = 0;
	} else if (fade == fadeOut) {
		fadeOut = 0;
	}
	
	m_fades.remove(fade);
}

void AudioClip::create_fade_in( )
{
	fadeIn = new FadeCurve(this, m_song, "FadeIn");
	fadeIn->set_shape("Fast");
	fadeIn->set_history_stack(get_history_stack());
	THREAD_SAVE_INVOKE_AND_EMIT_SIGNAL(this, fadeIn, private_add_fade(FadeCurve*), fadeAdded(FadeCurve*));
}

void AudioClip::create_fade_out( )
{
	fadeOut = new FadeCurve(this, m_song, "FadeOut");
	fadeOut->set_shape("Fast");
	fadeOut->set_history_stack(get_history_stack());
	THREAD_SAVE_INVOKE_AND_EMIT_SIGNAL(this, fadeOut, private_add_fade(FadeCurve*), fadeAdded(FadeCurve*));
}

QDomNode AudioClip::get_dom_node() const
{
	return m_domNode;
}

bool AudioClip::has_song() const
{
	if (m_song) {
		return true;
	}
	return false;
}

ReadSource * AudioClip::get_readsource() const
{
	return m_readSource;
}

void AudioClip::get_capture_bus()
{
	m_captureBus = audiodevice().get_capture_bus(m_captureBusName);
}

