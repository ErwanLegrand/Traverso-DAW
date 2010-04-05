/*
Copyright (C) 2005-2008 Remon Sijrier

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
#include "Sheet.h"
#include "SnapList.h"
#include "AudioTrack.h"
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
#include "InputEngine.h"

#include "AbstractAudioReader.h"

#include <commands.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

/**
 *	\class AudioClip
	\brief Represents (part of) an audiofile.
 */

AudioClip::AudioClip(const QString& name)
	: Snappable()
{
	PENTERCONS;
        m_name = name;
	m_isMuted=false;
	m_id = create_id();
	m_readSourceId = m_sheetId = 0;
	init();
}


AudioClip::AudioClip(const QDomNode& node)
	: Snappable()
	, m_track(0)
	, m_readSource(0)
{
	PENTERCONS;
	
	// It makes sense to set these values at this time allready
	// they are for example used by the ResourcesManager!
	QDomElement e = node.toElement();
	m_id = e.attribute("id", "").toLongLong();
	m_readSourceId = e.attribute("source", "").toLongLong();
	m_sheetId = e.attribute("sheet", "0").toLongLong();
	m_name = e.attribute( "clipname", "" ) ;
	m_isMuted =  e.attribute( "mute", "" ).toInt();
        m_length = TimeRef(e.attribute( "length", "0" ).toLongLong());
        m_sourceStartLocation = TimeRef(e.attribute( "sourcestart", "" ).toLongLong());
	
	m_sourceEndLocation = m_sourceStartLocation + m_length;
        TimeRef location(e.attribute( "trackstart", "" ).toLongLong());
	m_domNode = node.cloneNode();
	init();
        // first init to set variables that are referenced in:
        set_track_start_location(location);
}

AudioClip::~AudioClip()
{
	PENTERDES;
	if (m_readSource) {
		m_sheet->get_diskio()->unregister_read_source(m_readSource);
		delete m_readSource;
	}
	if (m_peak) {
		m_peak->close();
	}
}

void AudioClip::init()
{
	QObject::tr("AudioClip");
	m_sheet = 0;
	m_track = 0;
	m_readSource = 0;
	m_peak = 0;
	m_recordingStatus = NO_RECORDING;
	m_isReadSourceValid = m_isMoving = false;
	m_isLocked = config().get_property("AudioClip", "LockByDefault", false).toBool();
	fadeIn = 0;
	fadeOut = 0;
	m_fader->automate_port(0, true);
	m_fader->set_gain(1.0);
}

int AudioClip::set_state(const QDomNode& node)
{
	PENTER;
	
	Q_ASSERT(m_sheet);
	
	QDomElement e = node.toElement();

	m_isTake = e.attribute( "take", "").toInt();
	set_gain( e.attribute( "gain", "" ).toFloat() );
	m_isLocked = e.attribute( "locked", "0" ).toInt();
	m_readSourceId = e.attribute("source", "").toLongLong();
	m_sheetId = e.attribute("sheet", "0").toLongLong();
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
			fadeIn = new FadeCurve(this, m_sheet, "FadeIn");
			fadeIn->set_history_stack(get_history_stack());
			private_add_fade(fadeIn);
		}
		fadeIn->set_state( fadeInNode );
	}

	QDomElement fadeOutNode = node.firstChildElement("FadeOut");
	if (!fadeOutNode.isNull()) {
		if (!fadeOut) {
			fadeOut = new FadeCurve(this, m_sheet, "FadeOut");
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
	node.setAttribute("id", m_id );
	node.setAttribute("sheet", m_sheetId );
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
	
	bool syncDuringDrag = config().get_property("AudioClip", "SyncDuringDrag", false).toBool();
	bool stopSyncDueMove;
	if (m_isMoving && syncDuringDrag) {
		stopSyncDueMove = false;
	} else if (m_isMoving && !syncDuringDrag){
		stopSyncDueMove = true;
	} else {
		stopSyncDueMove = false;
	}
		
	if ( m_track->is_muted() || m_track->is_muted_by_solo() || is_muted() || stopSyncDueMove) {
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

		set_source_start_location( m_sourceStartLocation - movingToLeft );
		set_track_start_location(m_trackStartLocation - movingToLeft);
	} else if (newLeftLocation > m_trackStartLocation) {

		TimeRef availableTimeRight = m_length;

		TimeRef movingToRight = newLeftLocation - m_trackStartLocation;

		if (movingToRight > (availableTimeRight - TimeRef(nframes_t(4), get_rate())) ) {
			movingToRight = (availableTimeRight - TimeRef(nframes_t(4), get_rate()));
		}

		set_source_start_location( m_sourceStartLocation + movingToRight );
		set_track_start_location(m_trackStartLocation + movingToRight);
	}
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

		set_source_end_location( m_sourceEndLocation + movingToRight );
		set_track_end_location( m_trackEndLocation + movingToRight );

	} else if (newRightLocation < m_trackEndLocation) {

		TimeRef availableTimeLeft = m_length;

		TimeRef movingToLeft = m_trackEndLocation - newRightLocation;

		if (movingToLeft > availableTimeLeft - TimeRef(nframes_t(4), get_rate())) {
			movingToLeft = availableTimeLeft - TimeRef(nframes_t(4), get_rate());
		}

		set_source_end_location( m_sourceEndLocation - movingToLeft);
		set_track_end_location( m_trackEndLocation - movingToLeft );
	}
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
	PENTER2;
	m_trackStartLocation = location;
	
	// set_track_end_location will emit positionChanged(), so we 
	// don't emit it in this function to avoid emitting it twice
	// (although it seems more logical to emit it here, there are
	// accasions where only set_track_end_location() is called, and 
	// then we also want to emit positionChanged())
	set_track_end_location(m_trackStartLocation + m_length);
}

void AudioClip::set_track_end_location(const TimeRef& location)
{
// 	PWARN("m_trackEndLocation is %d", endFrame);
	m_trackEndLocation = location;
	
	if (!is_moving()) {
		// TODO find out if we also have to call this even during moving?
		// And somehow Track itself should manage this, and not from here!
		// The purpose of this call is to keep the AudioClip list in track 
		// sorted on the clips track start location.
		if (m_track) {
                        if (m_sheet && m_sheet->is_transport_rolling()) {
                                THREAD_SAVE_INVOKE(m_track, this, clip_position_changed(AudioClip*));
                        } else {
                                m_track->clip_position_changed(this);
                        }
		}
		
		if (m_sheet) {
			m_sheet->get_snap_list()->mark_dirty();
		}
	}

	emit positionChanged();
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
	emit stateChanged();
}

//
//  Function called in RealTime AudioThread processing path
//
int AudioClip::process(nframes_t nframes)
{
	Q_ASSERT(m_sheet);
	
	// Handle silence clips
	if (get_channels() == 0) {
		return 0;
	}
	
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
	
	AudioBus* bus = m_sheet->get_clip_render_bus();
	bus->silence_buffers(nframes);
	
	TimeRef mix_pos;
	int channelcount = get_channels();
	audio_sample_t* mixdown[channelcount];
	uint framesToProcess = nframes;


	int outputRate = m_readSource->get_output_rate();
	TimeRef transportLocation = m_sheet->get_transport_location();
	TimeRef upperRange = transportLocation + TimeRef(framesToProcess, outputRate);
	
	
	if ( (m_trackStartLocation < upperRange) && (m_trackEndLocation > transportLocation) ) {
		if (transportLocation < m_trackStartLocation) {
			// Using to_frame() for both the m_trackStartLocation and transportLocation seems to round 
			// better then using (m_trackStartLocation - transportLocation).to_frame()
			// TODO : find out why!
			uint offset = (m_trackStartLocation).to_frame(outputRate) - transportLocation.to_frame(outputRate);
			mix_pos = m_sourceStartLocation;
// 			printf("offset %d\n", offset);
			
			for (int chan=0; chan<bus->get_channel_count(); ++chan) {
				audio_sample_t* buf = bus->get_buffer(chan, framesToProcess);
				mixdown[chan] = buf + offset;
			}
			framesToProcess = framesToProcess - offset;
		} else {
			mix_pos = (transportLocation - m_trackStartLocation + m_sourceStartLocation);
// 			printf("else: Setting mix pos to start location %d\n", mix_pos.to_frame(96000));
			
			for (int chan=0; chan<bus->get_channel_count(); ++chan) {
				mixdown[chan] = bus->get_buffer(chan, framesToProcess);
			}
		}
		if (m_trackEndLocation < upperRange) {
			// Using to_frame() for both the upperRange and m_trackEndLocation seems to round 
			// better then using (upperRange - m_trackEndLocation).to_frame()
			// TODO : find out why!
			framesToProcess -= upperRange.to_frame(outputRate) - m_trackEndLocation.to_frame(outputRate);
// 			printf("if (m_trackEndLocation < upperRange): framesToProcess %d\n", framesToProcess);
		}
	} else {
		return 0;
	}

	uint read_frames = 0;

	if (m_sheet->realtime_path()) {
		read_frames = m_readSource->rb_read(mixdown, mix_pos, framesToProcess);
	} else {
		read_frames = m_readSource->file_read(m_sheet->renderDecodeBuffer, mix_pos, framesToProcess);
		if (read_frames > 0) {
			for (int chan=0; chan<channelcount; ++chan) {
				memcpy(mixdown[chan], m_sheet->renderDecodeBuffer->destination[chan], read_frames * sizeof(audio_sample_t));
			}
		}
	}
	
	if (read_frames <= 0) {
// 		printf("read_frames == 0\n");
		return 0;
	}
	
	if (read_frames != framesToProcess) {
		printf("read_frames, framesToProcess %d, %d\n", read_frames, framesToProcess);
	}		
	

	apill_foreach(FadeCurve* fade, FadeCurve, m_fades) {
                fade->process(bus, nframes);
	}
	
	TimeRef endlocation = mix_pos + TimeRef(read_frames, get_rate());
	m_fader->process_gain(mixdown, mix_pos, endlocation, read_frames, channelcount);
	
        AudioBus* processBus = m_track->get_process_bus();
	
	// NEVER EVER FORGET that the mixing should be done on the WHOLE buffer, not just part of it
	// so use an unmodified nframes variable!!!!!!!!!!!!!!!!!!!!!!!!!!!1
	if (channelcount == 1) {
                Mixer::mix_buffers_no_gain(processBus->get_buffer(0, nframes), bus->get_buffer(0, nframes), nframes);
                Mixer::mix_buffers_no_gain(processBus->get_buffer(1, nframes), bus->get_buffer(0, nframes), nframes);
	} else if (channelcount == 2) {
                Mixer::mix_buffers_no_gain(processBus->get_buffer(0, nframes), bus->get_buffer(0, nframes), nframes);
                Mixer::mix_buffers_no_gain(processBus->get_buffer(1, nframes), bus->get_buffer(1, nframes), nframes);
	}
	
	return 1;
}

//
//  Function called in RealTime AudioThread processing path
//
void AudioClip::process_capture(nframes_t nframes)
{
        AudioBus* bus = m_track->get_input_bus();

        if (!bus) {
		return;
	}
	
        nframes_t written = m_writer->rb_write(bus, nframes);

        m_length.add_frames(written, get_rate());

	if (written != nframes) {
		printf("couldn't write nframes %d to recording buffer for channel 0, only %d\n", nframes, written);
	}
}

int AudioClip::init_recording()
{
	Q_ASSERT(m_sheet);
	Q_ASSERT(m_track);
	
        AudioBus* bus = m_track->get_input_bus();
        int channelcount = bus->get_channel_count();

        if (channelcount == 0) {
                // Can't record from a Bus with no channels!
                return -1;
        }

	m_sourceStartLocation = TimeRef();
	m_isTake = 1;
	m_recordingStatus = RECORDING;
	
	ReadSource* rs = resources_manager()->create_recording_source(
				pm().get_project()->get_root_dir() + "/audiosources/",
				m_name, channelcount, m_sheet->get_id());
	
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
        spec->dataF = bus->get_buffer(0, audiodevice().get_buffer_size());

        m_writer = new WriteSource(spec);
        if (m_writer->prepare_export() == -1) {
                delete m_writer;
		delete spec;
		spec = 0;
		return -1;
	}
        m_writer->set_process_peaks( true );
        m_writer->set_recording( true );
	
        m_sheet->get_diskio()->register_write_source(m_writer);
	
        connect(m_writer, SIGNAL(exportFinished()), this, SLOT(finish_write_source()));
	connect(m_sheet, SIGNAL(transportStopped()), this, SLOT(finish_recording()));

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
	if (fadeIn) {
		return new FadeRange(this, fadeIn, 1.0);
	}
	return 0;
}

Command* AudioClip::reset_fade_out()
{
	if (fadeOut) {
		return new FadeRange(this, fadeOut, 1.0);
	}
	return 0;
}

Command* AudioClip::reset_fade_both()
{
	if (!fadeOut && !fadeIn) {
		return 0;
	}
	
	CommandGroup* group = new CommandGroup(this, tr("Remove Fades"));
	
	if (fadeIn) {
		group->add_command(reset_fade_in());
	}
	if (fadeOut) {
		group->add_command(reset_fade_out());
	}
	
	return group;
}

AudioClip* AudioClip::create_copy( )
{
	PENTER;
	Q_ASSERT(m_sheet);
	Q_ASSERT(m_track);
	QDomDocument doc("AudioClip");
	QDomNode clipState = get_state(doc);
	AudioClip* clip = new AudioClip(m_name);
	clip->set_sheet(m_sheet);
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

	set_sources_active_state();

	rs->set_audio_clip(this);
	
	
	if (m_recordingStatus == NO_RECORDING) {
		if (m_peak) {
			m_peak->close();
		}
		if (m_isReadSourceValid) {
			m_peak = new Peak(rs);
		}
	}

	// This will also emit positionChanged() which is more or less what we want.
	set_track_end_location(m_trackStartLocation + m_sourceLength - m_sourceStartLocation);
}

void AudioClip::finish_write_source()
{
	PENTER;
	
	Q_ASSERT(m_readSource);
	
        if (m_readSource->set_file(m_writer->get_filename()) < 0) {
		PERROR("Setting file for ReadSource failed after finishing recording");
	} else {
		m_sheet->get_diskio()->register_read_source(m_readSource);
		// re-inits the lenght from the audiofile due calling rsm->set_source_for_clip()
		m_length = TimeRef();
	}
	
        delete m_writer;
        m_writer = 0;
	
	m_recordingStatus = NO_RECORDING;
	
	resources_manager()->set_source_for_clip(this, m_readSource);
	
	emit recordingFinished(this);
}

void AudioClip::finish_recording()
{
	PENTER;
	
	m_recordingStatus = FINISHING_RECORDING;
        m_writer->set_recording(false);

	disconnect(m_sheet, SIGNAL(transportStopped()), this, SLOT(finish_recording()));
}

int AudioClip::get_channels( ) const
{
	if (m_readSource) {
		return m_readSource->get_channel_count();
	} else {
                if (m_writer) {
                        return m_writer->get_channel_count();
		}
	}
	
	return 0;
}

Sheet* AudioClip::get_sheet( ) const
{
	Q_ASSERT(m_sheet);
	return m_sheet;
}

AudioTrack* AudioClip::get_track( ) const
{
	Q_ASSERT(m_track);
	return m_track;
}

void AudioClip::set_sheet( Sheet * sheet )
{
	m_sheet = sheet;
	if (m_readSource && m_isReadSourceValid) {
		m_sheet->get_diskio()->register_read_source( m_readSource );
	} else {
		PWARN("AudioClip::set_sheet() : Setting Sheet, but no ReadSource available!!");
	}
	
	m_sheetId = sheet->get_id();
	
	set_history_stack(m_sheet->get_history_stack());
	m_pluginChain->set_sheet(m_sheet);
	set_snap_list(m_sheet->get_snap_list());
}


// TODO: this function is (also) called from the GUI thread when moving a clip from one track
//      to another. When this clip is being 'played' during the move, m_track is changed in the gui
//      thread, but we use m_track in ::process(). Could this result in an invalid pointer?
void AudioClip::set_track( AudioTrack * track )
{
	if (m_track) {
		disconnect(m_track, SIGNAL(audibleStateChanged()), this, SLOT(track_audible_state_changed()));
	}
	
	m_track = track;
	
	connect(m_track, SIGNAL(audibleStateChanged()), this, SLOT(track_audible_state_changed()));
	set_sources_active_state();
}

bool AudioClip::is_selected( )
{
	Q_ASSERT(m_sheet);
	return m_sheet->get_audioclip_manager()->is_clip_in_selection(this);
}

bool AudioClip::is_take( ) const
{
	return m_isTake;
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

TimeRef AudioClip::get_source_length( ) const
{
	return m_sourceLength;
}

int AudioClip::recording_state( ) const
{
	return m_recordingStatus;
}

Command * AudioClip::normalize( )
{
        bool ok;
        float normfactor;
        double d = QInputDialog::getDouble(0, tr("Normalization"),
                                           tr("Set Normalization level:"), 0.0, -120, 0, 1, &ok);
        if (ok) {
                normfactor = calculate_normalization_factor(d);
        }

        if (!ok || (normfactor == get_gain())) {
            return ie().failure();
        }

        return new PCommand(this, "set_gain", normfactor, get_gain(), tr("AudioClip: Normalize"));
}


float AudioClip::calculate_normalization_factor(float targetdB)
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
                return get_gain();
	}
	
	if (maxamp == target) {
		printf("AudioClip::normalization: max amplitude == target amplitude\n");
		/* we can't do anything useful */
                return get_gain();
	}

	/* compute scale factor */
        return float(target/maxamp);
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
	fadeIn = new FadeCurve(this, m_sheet, "FadeIn");
	fadeIn->set_shape("Fast");
	fadeIn->set_history_stack(get_history_stack());
	THREAD_SAVE_INVOKE_AND_EMIT_SIGNAL(this, fadeIn, private_add_fade(FadeCurve*), fadeAdded(FadeCurve*));
}

void AudioClip::create_fade_out( )
{
	fadeOut = new FadeCurve(this, m_sheet, "FadeOut");
	fadeOut->set_shape("Fast");
	fadeOut->set_history_stack(get_history_stack());
	THREAD_SAVE_INVOKE_AND_EMIT_SIGNAL(this, fadeOut, private_add_fade(FadeCurve*), fadeAdded(FadeCurve*));
}

QDomNode AudioClip::get_dom_node() const
{
	return m_domNode;
}

bool AudioClip::has_sheet() const
{
	if (m_sheet) {
		return true;
	}
	return false;
}

ReadSource * AudioClip::get_readsource() const
{
	return m_readSource;
}

void AudioClip::set_as_moving(bool moving)
{
	m_isMoving = moving;
	set_snappable(!m_isMoving);
	set_sources_active_state();
}

