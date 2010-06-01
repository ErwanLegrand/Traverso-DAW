/*
Copyright (C) 2005-2010 Remon Sijrier

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

#include <QTextStream>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QRegExp>
#include <QDebug>

#include <commands.h>

#include "AbstractAudioReader.h"
#include <AudioDevice.h>
#include <AudioBus.h>
#include <Client.h>
#include "ProjectManager.h"
#include "ContextPointer.h"
#include "Information.h"
#include "Sheet.h"
#include "Project.h"
#include "AudioTrack.h"
#include "Mixer.h"
#include "AudioSource.h"
#include "AudioClip.h"
#include "Peak.h"
#include "Export.h"
#include "DiskIO.h"
#include "WriteSource.h"
#include "AudioClipManager.h"
#include "Tsar.h"
#include "SnapList.h"
#include "SubGroup.h"
#include "Config.h"
#include "Utils.h"
#include "ContextItem.h"
#include "TimeLine.h"
#include "Marker.h"
#include "InputEngine.h"                       

#include <Plugin.h>
#include <PluginChain.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


/**	\class Sheet
	\brief The 'work space' (as in WorkSheet) holding the Track 's and the Master Out AudioBus
	
	A Sheet processes each Track, and mixes the result into it's Master Out AudioBus.
	Sheet connects it's Client to the AudioDevice, to become part of audio processing
	chain. The connection is instantiated by Project, who owns the Sheet objects.
 
 
 */


Sheet::Sheet(Project* project, int numtracks)
        : m_project(project)
{
	PENTERCONS;
        m_name = tr("Untitled");
	m_id = create_id();
        m_artists = tr("No artists name set");
	m_hzoom = config().get_property("Sheet", "hzoomLevel", 8192).toInt();

	init();

	for (int i=1; i <= numtracks; i++) {
                AudioTrack* track = create_audio_track();
                private_add_track(track);
	}

        resize_buffer(audiodevice().get_buffer_size());
}

Sheet::Sheet(Project* project, const QDomNode node)
        : m_project(project)
{
	PENTERCONS;
	
	m_id = node.toElement().attribute("id", "0").toLongLong();
	
	if (m_id == 0) {
		m_id = create_id();
	}

        init();
        set_state( node );
}

Sheet::~Sheet()
{
	PENTERDES;

	delete [] mixdown;
	delete [] gainbuffer;

	delete m_diskio;
        delete m_masterOut;
	delete m_renderBus;
	delete m_clipRenderBus;
	delete m_hs;
        delete m_audiodeviceClient;
        delete m_snaplist;
        delete m_workSnap;
}

void Sheet::init()
{
	PENTER2;
#if defined (THREAD_CHECK)
	threadId = QThread::currentThreadId ();
#endif

	QObject::tr("Sheet");

	m_diskio = new DiskIO(this);
	m_currentSampleRate = audiodevice().get_sample_rate();
	m_diskio->output_rate_changed(m_currentSampleRate);
	int converter_type = config().get_property("Conversion", "RTResamplingConverterType", DEFAULT_RESAMPLE_QUALITY).toInt();
	m_diskio->set_resample_quality(converter_type);

        m_hs = new QUndoStack(pm().get_undogroup());
        set_history_stack(m_hs);

        m_acmanager = new AudioClipManager(this);
        set_context_item( m_acmanager );

	connect(this, SIGNAL(seekStart()), m_diskio, SLOT(seek()), Qt::QueuedConnection);
	connect(this, SIGNAL(prepareRecording()), this, SLOT(prepare_recording()));
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(audiodevice_params_changed()), Qt::DirectConnection);
	connect(m_diskio, SIGNAL(seekFinished()), this, SLOT(seek_finished()), Qt::QueuedConnection);
	connect (m_diskio, SIGNAL(readSourceBufferUnderRun()), this, SLOT(handle_diskio_readbuffer_underrun()));
	connect (m_diskio, SIGNAL(writeSourceBufferOverRun()), this, SLOT(handle_diskio_writebuffer_overrun()));
	connect(&config(), SIGNAL(configChanged()), this, SLOT(config_changed()));
	connect(this, SIGNAL(transportStarted()), m_diskio, SLOT(start_io()));
	connect(this, SIGNAL(transportStopped()), m_diskio, SLOT(stop_io()));

	mixdown = gainbuffer = 0;

        BusConfig busConfig;
        busConfig.name = "Sheet Render Bus";
        busConfig.channelcount = 2;
        busConfig.type = "output";
        busConfig.isInternalBus = true;
        m_renderBus = new AudioBus(busConfig);

        busConfig.name = "Sheet Clip Render Bus";
        m_clipRenderBus = new AudioBus(busConfig);

        m_masterOut = new MasterOutSubGroup(this, tr("Sheet Master"));
        resize_buffer(audiodevice().get_buffer_size());

	m_transport = m_stopTransport = m_resumeTransport = m_readyToRecord = false;
        m_snaplist = new SnapList(this);
        m_workSnap = new Snappable();
        m_workSnap->set_snap_list(m_snaplist);

	m_realtimepath = false;
        m_scheduledForDeletion = false;
        m_isSnapOn=true;
	m_changed = m_rendering = m_recording = m_prepareRecording = false;
        m_firstVisibleFrame=0;
	m_workLocation = TimeRef();
	m_seeking = m_startSeek = 0;
	// TODO seek to old position on project exit ?
	m_transportLocation = TimeRef();
	m_mode = EDIT;
	m_sbx = m_sby = 0;
	m_hzoom = config().get_property("Sheet", "hzoomLevel", 8192).toInt();
	m_timeline = new TimeLine(this);
	
	m_skipTimer.setSingleShot(true);
	
        m_audiodeviceClient = new AudioDeviceClient("sheet_" + QByteArray::number(get_id()));
        m_audiodeviceClient->set_process_callback( MakeDelegate(this, &Sheet::process) );
        m_audiodeviceClient->set_transport_control_callback( MakeDelegate(this, &Sheet::transport_control) );
}

int Sheet::set_state( const QDomNode & node )
{
	PENTER;
	
	QDomNode propertiesNode = node.firstChildElement("Properties");
	QDomElement e = propertiesNode.toElement();

        m_name = e.attribute( "title", "" );
        m_artists = e.attribute( "artists", "" );
        set_audio_sources_dir(e.attribute("audiosourcesdir", ""));
	qreal zoom = e.attribute("hzoom", "4096").toDouble();
	set_hzoom(zoom);
	m_sbx = e.attribute("sbx", "0").toInt();
	m_sby = e.attribute("sby", "0").toInt();
	set_first_visible_frame(e.attribute( "firstVisibleFrame", "0" ).toUInt());
	
	bool ok;
        m_workLocation = e.attribute( "m_workLocation", "0").toLongLong(&ok);
	m_transportLocation = TimeRef(e.attribute( "transportlocation", "0").toLongLong(&ok));
	
	// Start seeking to the 'old' transport pos
	set_transport_pos(m_transportLocation);
	set_snapping(e.attribute("snapping", "0").toInt());
	m_mode = e.attribute("mode", "0").toInt();
	
	m_timeline->set_state(node.firstChildElement("TimeLine"));

        QDomNode subgroupsNode = node.firstChildElement("SubGroups");
        
        QDomNode masterOutNode = node.firstChildElement("MasterOut");
        m_masterOut->set_state(masterOutNode.firstChildElement());
        // Force the proper name for our Master Bus
        m_masterOut->set_name(tr("Sheet Master"));

        QDomNode subgroupNode = subgroupsNode.firstChild();

        while(!subgroupNode.isNull()) {
                SubGroup* subgroup = new SubGroup(this, subgroupNode);
                subgroup->set_state(subgroupNode);
                private_add_track(subgroup);

                subgroupNode = subgroupNode.nextSibling();
        }

        QDomNode tracksNode = node.firstChildElement("Tracks");
	QDomNode trackNode = tracksNode.firstChild();

        while(!trackNode.isNull()) {
		AudioTrack* track = new AudioTrack(this, trackNode);
                private_add_track(track);
		track->set_state(trackNode);

		trackNode = trackNode.nextSibling();
	}

        m_acmanager->set_state(node.firstChildElement("ClipManager"));
	
	return 1;
}

QDomNode Sheet::get_state(QDomDocument doc, bool istemplate)
{
	QDomElement sheetNode = doc.createElement("Sheet");
	
	if (! istemplate) {
		sheetNode.setAttribute("id", m_id);
        } else {
                sheetNode.setAttribute("id", create_id());
        }
	
	QDomElement properties = doc.createElement("Properties");
        properties.setAttribute("title", m_name);
        properties.setAttribute("artists", m_artists);
        properties.setAttribute("audiosourcesdir", m_audioSourcesDir);
        properties.setAttribute("firstVisibleFrame", m_firstVisibleFrame);
	properties.setAttribute("m_workLocation", m_workLocation.universal_frame());
	properties.setAttribute("transportlocation", m_transportLocation.universal_frame());
	properties.setAttribute("hzoom", m_hzoom);
	properties.setAttribute("sbx", m_sbx);
	properties.setAttribute("sby", m_sby);
	properties.setAttribute("snapping", m_isSnapOn);
	properties.setAttribute("mode", m_mode);
	sheetNode.appendChild(properties);

	doc.appendChild(sheetNode);

	sheetNode.appendChild(m_acmanager->get_state(doc));
	
	sheetNode.appendChild(m_timeline->get_state(doc));

        QDomNode masterOutNode = doc.createElement("MasterOut");
        masterOutNode.appendChild(m_masterOut->get_state(doc, istemplate));
        sheetNode.appendChild(masterOutNode);

	QDomNode tracksNode = doc.createElement("Tracks");

        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		tracksNode.appendChild(track->get_state(doc, istemplate));
	}

	sheetNode.appendChild(tracksNode);


        QDomNode subgroupsNode = doc.createElement("SubGroups");
        apill_foreach(SubGroup* group, SubGroup, m_subGroups) {
                subgroupsNode.appendChild(group->get_state(doc, istemplate));
        }

        sheetNode.appendChild(subgroupsNode);

	return sheetNode;
}


Command* Sheet::add_track(Track* track, bool historable)
{
        apill_foreach(Track* existing, Track, m_audioTracks) {
		if (existing->is_solo()) {
                        track->set_muted_by_solo( true );
			break;
		}
	}

        return new AddRemove(this, track, historable, this,
                "private_add_track(Track*)", "trackAdded(Track*)",
                "private_remove_track(Track*)", "trackRemoved(Track*)",
                tr("Added %1: %2").arg(track->metaObject()->className()).arg(track->get_name()));
}


Command* Sheet::remove_track(Track* track, bool historable)
{
        return new AddRemove(this, track, historable, this,
                "private_remove_track(Track*)", "trackRemoved(Track*)",
                "private_add_track(Track*)", "trackAdded(Track*)",
                tr("Removed %1: %2").arg(track->metaObject()->className()).arg(track->get_name()));
}

bool Sheet::any_audio_track_armed()
{
        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		if (track->armed()) {
			return true;
		}
	}
	return false;
}

// this function is called from the parent project before it calls Sheet::render().
// depending on the renderpass mode, additional information is gathered here for
// the ExportSpecification (e.g. the start and end location, the marker list,
// transport is stopped, a new writeSource is created etc.)
int Sheet::prepare_export(ExportSpecification* spec)
{
	PENTER;
	
	if ( ! (spec->renderpass == ExportSpecification::CREATE_CDRDAO_TOC) ) {
		if (is_transport_rolling()) {
			spec->resumeTransport = true;
			// When transport is rolling, this equals stopping the transport!
			// prepare_export() is called from another thread, so use a queued connection
			// to call the function in the correct thread!
			if (!QMetaObject::invokeMethod(this, "start_transport",  Qt::QueuedConnection)) {
				printf("Invoking Sheet::start_transport() failed\n");
				return -1;
			}
			int count = 0;
			uint msecs = (audiodevice().get_buffer_size() * 1000) / audiodevice().get_sample_rate();
			// wait a number (max 10) of process() cycles to be sure we really stopped transport
			while (m_transport) {
				spec->thread->sleep_for(msecs);
				count++;
				if (count > 10) {
					break;
				}
			}
			printf("Sheet::prepare_export: had to wait %d process cycles before the transport was stopped\n", count);
		}
		
		m_rendering = true;
	}

	spec->startLocation = LONG_LONG_MAX;
	spec->endLocation = TimeRef();

	TimeRef endlocation, startlocation;

        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		track->get_render_range(startlocation, endlocation);

		if (track->is_solo()) {
			spec->endLocation = endlocation;
			spec->startLocation = startlocation;
			break;
		}

		if (endlocation > spec->endLocation) {
			spec->endLocation = endlocation;
		}

		if (startlocation < spec->startLocation) {
			spec->startLocation = startlocation;
		}
	}
	
	if (spec->isCdExport) {
		if (m_timeline->get_start_location(startlocation)) {
			PMESG2("  Start marker found at %s", QS_C(timeref_to_ms(startlocation)));
			// round down to the start of the CD frame (75th of a sec)
			startlocation = cd_to_timeref(timeref_to_cd(startlocation));
			spec->startLocation = startlocation;
		} else {
			PMESG2("  No start marker found");
		}			
		
		if (m_timeline->get_end_location(endlocation)) {
			PMESG2("  End marker found at %s", QS_C(timeref_to_ms(endlocation)));
			spec->endLocation = endlocation;
		} else {
			PMESG2("  No end marker found");
		}
	}

        // compute some default values
	spec->totalTime = spec->endLocation - spec->startLocation;

// 	PWARN("Render length is: %s",timeref_to_ms_3(spec->totalTime).toAscii().data() );

	spec->pos = spec->startLocation;
	spec->progress = 0;

        spec->basename = "Sheet_" + QString::number(m_project->get_sheet_index(m_id)) +"-" + m_name;
	spec->name = spec->basename;

	if (spec->startLocation == spec->endLocation) {
		info().warning(tr("No audio to export! (Is everything muted?)"));
		return -1;
	}
	else if (spec->startLocation > spec->endLocation) {
		info().warning(tr("Export start frame starts beyond export end frame!!"));
		return -1;
	}

	if (spec->channels == 0) {
		info().warning(tr("Export tries to render to 0 channels wav file??"));
		return -1;
	}

	if (spec->renderpass == ExportSpecification::CREATE_CDRDAO_TOC) {
		return 1;
	}
	
	m_transportLocation = spec->startLocation;
	
        resize_buffer(spec->blocksize);
	
	renderDecodeBuffer = new DecodeBuffer;

	return 1;
}

int Sheet::finish_audio_export()
{
        delete renderDecodeBuffer;
        resize_buffer(audiodevice().get_buffer_size());
        m_rendering = false;
        return 0;
}

// this function is called from the parent project. if several cd-tracks should be exported
// to separate files, we will call the render() process for each file.
int Sheet::start_export(ExportSpecification* spec)
{
        QString message;
        float peakvalue = 0.0;

        spec->markers = m_timeline->get_cdtrack_list(spec);

        for (int i = 0; i < spec->markers.size()-1; ++i) {
                spec->progress      = 0;
                                      // round down to the start of the CD frame (75th of a sec)
                spec->cdTrackStart  = cd_to_timeref(timeref_to_cd(spec->markers.at(i)->get_when()));
                spec->cdTrackEnd    = cd_to_timeref(timeref_to_cd(spec->markers.at(i+1)->get_when()));
                spec->name          = m_timeline->format_cdtrack_name(spec->markers.at(i), i+1);
                spec->totalTime     = spec->cdTrackEnd - spec->cdTrackStart;
                spec->pos           = spec->cdTrackStart;
                m_transportLocation = spec->cdTrackStart;


                if (spec->renderpass == ExportSpecification::WRITE_TO_HARDDISK) {
                        m_exportSource = new WriteSource(spec);

                        if (m_exportSource->prepare_export() == -1) {
                                delete m_exportSource;
                                return -1;
                        }

                        message = QString(tr("Rendering Sheet %1 - Track %2 of %3")).arg(m_name).arg(i+1).arg(spec->markers.size()-1);

                } else if (spec->renderpass == ExportSpecification::CALC_NORM_FACTOR) {
                        message = QString(tr("Normalising Sheet %1 - Track %2 of %3")).arg(m_name).arg(i+1).arg(spec->markers.size()-1);
                }

                m_project->set_export_message(message);

                while(render(spec) > 0) {}

                peakvalue = f_max(peakvalue, spec->peakvalue);
                spec->peakvalue = peakvalue;

                if (spec->renderpass == ExportSpecification::WRITE_TO_HARDDISK) {
                        m_exportSource->finish_export();
                        delete m_exportSource;
                }
        }

        finish_audio_export();
        return 1;
}

int Sheet::render(ExportSpecification* spec)
{
	int chn;
	uint32_t x;
	int ret = -1;
        int progress = 0;

        nframes_t diff = (spec->cdTrackEnd - spec->pos).to_frame(audiodevice().get_sample_rate());
	nframes_t nframes = spec->blocksize;
	nframes_t this_nframes = std::min(diff, nframes);

	if (!spec->running || spec->stop || this_nframes == 0) {
		process_export (nframes);
		/*		PWARN("Finished Rendering for this sheet");
				PWARN("running is %d", spec->running);
				PWARN("stop is %d", spec->stop);
                                PWARN("this_nframes is %d", this_nframes);*/
                return 0;
	}

	/* do the usual stuff */

	process_export(nframes);

	/* and now export the results */

	nframes = this_nframes;

	memset (spec->dataF, 0, sizeof (spec->dataF[0]) * nframes * spec->channels);

	/* foreach output channel ... */

	float* buf;

	for (chn = 0; chn < spec->channels; ++chn) {
                buf = m_masterOut->get_process_bus()->get_buffer(chn, nframes);

		if (!buf) {
			// Seem we are exporting at least to Stereo from an AudioBus with only one channel...
			// Use the first channel..
                        buf = m_masterOut->get_process_bus()->get_buffer(0, nframes);
		}

		for (x = 0; x < nframes; ++x) {
			spec->dataF[chn+(x*spec->channels)] = buf[x];
		}
	}


	int bufsize = spec->blocksize * spec->channels;
	if (spec->normalize) {
		if (spec->renderpass == ExportSpecification::CALC_NORM_FACTOR) {
			spec->peakvalue = Mixer::compute_peak(spec->dataF, bufsize, spec->peakvalue);
		}
	}
	
	if (spec->renderpass == ExportSpecification::WRITE_TO_HARDDISK) {
		if (spec->normalize) {
			Mixer::apply_gain_to_buffer(spec->dataF, bufsize, spec->normvalue);
		}
		if (m_exportSource->process (nframes)) {
			goto out;
		}
	}
	

	spec->pos.add_frames(nframes, audiodevice().get_sample_rate());

        progress = (int) (double( 100 * (spec->pos - spec->cdTrackStart).universal_frame()) / (spec->totalTime.universal_frame()));

        // only update the progress info if progress is higher then the
        // old progress value, to avoid a flood of progress changed signals!
        if (progress > spec->progress) {
                spec->progress = progress;
                m_project->set_sheet_export_progress(progress);
        }


	/* and we're good to go */

	ret = 1;

out:
	if (!ret) {
		spec->status = ret;
	}

	return ret;
}


SnapList* Sheet::get_snap_list() const
{
        return m_snaplist;
}


void Sheet::set_artists(const QString& pArtists)
{
        m_artists = pArtists;
        emit propertyChanged();
}

void Sheet::set_name(const QString& name)
 {
        m_name = name;
        emit propertyChanged();
}


void Sheet::set_gain(float gain)
{
	if (gain < 0.0)
		gain = 0.0;
	if (gain > 2.0)
		gain = 2.0;

        m_masterOut->set_gain(gain);

        emit stateChanged();
}

void Sheet::set_first_visible_frame(nframes_t pos)
{
	PENTER;
        m_firstVisibleFrame = pos;
	emit firstVisibleFrameChanged();
}

void Sheet::set_work_at(TimeRef location, bool isFolder)
{
        if ((! isFolder) && m_project->sheets_are_track_folder()) {
                return m_project->set_work_at(location);
        }

        // catch location < 0
        if (location < TimeRef()) {
                location = TimeRef();
        }

	m_workLocation = location;

        if (m_workSnap->is_snappable()) {
                m_snaplist->mark_dirty();
	}

	emit workingPosChanged();
}

void Sheet::set_work_at_for_sheet_as_track_folder(const TimeRef &location)
{
        set_work_at(location, true);
}

Command* Sheet::toggle_snap()
{
	set_snapping( ! m_isSnapOn );
	return 0;
}


void Sheet::set_snapping(bool snapping)
{
	m_isSnapOn = snapping;
	emit snapChanged();
}

/******************************** SLOTS *****************************/

AudioTrack* Sheet::create_audio_track()
{
	int height = AudioTrack::INITIAL_HEIGHT;

	AudioTrack* track = new AudioTrack(this, "Unnamed", height);

	return track;
}

void Sheet::solo_track(Track *track)
{
        bool wasSolo = track->is_solo();

        track->set_muted_by_solo(!wasSolo);
        track->set_solo(!wasSolo);

        QList<AudioTrack*> tracks= get_audio_tracks();


        // If the Track was a SubGroup, then also (un) solo all the AudioTracks
        // that have this SubGroup as the output bus.
        if ((track->get_type() == Track::SUBGROUP) && !(track == m_masterOut)) {
                QList<AudioTrack*> subgroupAudioTracks;
                foreach(AudioTrack* sgTrack, tracks) {
                        if (sgTrack->get_bus_out_name() == track->get_name()) {
                                subgroupAudioTracks.append(sgTrack);
                        }
                }

                if (wasSolo) {
                        foreach(AudioTrack* sgTrack, subgroupAudioTracks) {
                                sgTrack->set_solo(false);
                                sgTrack->set_muted_by_solo(false);
                        }
                } else {
                        foreach(AudioTrack* sgTrack, subgroupAudioTracks) {
                                sgTrack->set_solo(true);
                                sgTrack->set_muted_by_solo(true);
                        }
                }
        }

        bool hasSolo = false;

        foreach(Track* t, tracks) {
                t->set_muted_by_solo(!t->is_solo());
                if (t->is_solo()) {
                        hasSolo = true;
                }
        }

        if (!hasSolo) {
                foreach(Track* t, tracks) {
                        t->set_muted_by_solo(false);
                }
        }

}

Command* Sheet::toggle_solo()
{
	bool hasSolo = false;

        QList<Track*> tracks= get_tracks();

        foreach(Track* track, tracks) {
		if (track->is_solo()) hasSolo = true;
	}

        foreach(Track* track, tracks) {
                track->set_solo(!hasSolo);
		track->set_muted_by_solo(false);
	}

	return (Command*) 0;
}

Command *Sheet::toggle_mute()
{
	bool hasMute = false;
        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		if (track->is_muted()) hasMute = true;
	}
	
        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		track->set_muted(!hasMute);
	}

	return (Command*) 0;
}

Command *Sheet::toggle_arm()
{
	bool hasArmed = false;
        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		if (track->armed()) hasArmed = true;
	}

        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		if (hasArmed) {
			track->disarm();
		} else {
			track->arm();
		}
	}

	return (Command*) 0;
}

void Sheet::set_hzoom( qreal hzoom )
{
	// Traverso <= 0.42.0 doesn't store the real zoom factor, but an 
	// index. This currently causes problems as there is no real support
	// (yet) for zoomlevels other then powers of 2, so we force that for now.
	// NOTE: Remove those 2 lines when floating point zoomlevel is implemented!
	int highbit;
	hzoom = nearest_power_of_two(hzoom, highbit);


	if (hzoom > Peak::max_zoom_value()) {
		hzoom = Peak::max_zoom_value();
	}
	
	if (hzoom < 1.0) {
		hzoom = 1.0;
	}
	
	if (m_hzoom == hzoom) {
		return;
	}
	
	m_hzoom = hzoom;
	
	emit hzoomChanged();
}

//
//  Function called in RealTime AudioThread processing path
//
int Sheet::process( nframes_t nframes )
{
	if (m_startSeek) {
                printf("process: starting seek\n");
		start_seek();
		return 0;
	}

        if (m_seeking) {
                return 0;
        }
	
	// If no need for playback/record, return.
	if (!is_transport_rolling()) {
		return 0;
	}

	if (m_stopTransport) {
		m_transport = false;
		m_realtimepath = false;
		m_stopTransport = false;
		
		RT_THREAD_EMIT(this, 0, transportStopped());

		return 0;
	}

	// zero the m_masterOut buffers
        m_masterOut->get_process_bus()->silence_buffers(nframes);
        apill_foreach(SubGroup* group, SubGroup, m_subGroups) {
                group->get_process_bus()->silence_buffers(nframes);
        }


	int processResult = 0;


	// Process all Tracks.
        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		processResult |= track->process(nframes);
	}

        apill_foreach(SubGroup* group, SubGroup, m_subGroups) {
                group->process(nframes);
        }

	// update the transport location
	m_transportLocation.add_frames(nframes, audiodevice().get_sample_rate());

	if (!processResult) {
		return 0;
	}

        // Mix the result into the AudioDevice "physical" buffers
        m_masterOut->process(nframes);
	
	return 1;
}

int Sheet::process_export( nframes_t nframes )
{
	// Get the masterout buffers, and fill with zero's
        m_masterOut->get_process_bus()->silence_buffers(nframes);
        apill_foreach(SubGroup* group, SubGroup, m_subGroups) {
                group->get_process_bus()->silence_buffers(nframes);
        }
        memset (mixdown, 0, sizeof (audio_sample_t) * nframes);

	// Process all Tracks.
        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		track->process(nframes);
	}

        Mixer::apply_gain_to_buffer(m_masterOut->get_process_bus()->get_buffer(0, nframes), nframes, m_masterOut->get_gain());
        Mixer::apply_gain_to_buffer(m_masterOut->get_process_bus()->get_buffer(1, nframes), nframes, m_masterOut->get_gain());

	// update the m_transportFrame
// 	m_transportFrame += nframes;
	m_transportLocation.add_frames(nframes, audiodevice().get_sample_rate());

	return 1;
}


void Sheet::resize_buffer(nframes_t size)
{
	if (mixdown)
		delete [] mixdown;
	if (gainbuffer)
		delete [] gainbuffer;
	mixdown = new audio_sample_t[size];
	gainbuffer = new audio_sample_t[size];
        QList<AudioBus*> buses;
        buses.append(m_masterOut->get_process_bus());
        buses.append(m_renderBus);
        buses.append(m_clipRenderBus);
        foreach(AudioBus* bus, buses) {
                for(int i=0; i<bus->get_channel_count(); i++) {
                        if (AudioChannel* chan = bus->get_channel(i)) {
                                chan->set_buffer_size(size);
                        }
                }
        }
}

void Sheet::audiodevice_params_changed()
{
        resize_buffer(audiodevice().get_buffer_size());
	
	// The samplerate possibly has been changed, this initiates
	// a seek in DiskIO, which clears the buffers and refills them
	// with the correct resampled audio data!
	// We need to seek to a different position then the current one,
	// else the seek won't happen at all :)
	if (m_currentSampleRate != audiodevice().get_sample_rate()) {
		m_currentSampleRate = audiodevice().get_sample_rate();
		
		m_diskio->output_rate_changed(m_currentSampleRate);
		
		TimeRef location = m_transportLocation;
		location.add_frames(1, audiodevice().get_sample_rate());
	
		set_transport_pos(location);
	}
}

int Sheet::get_bitdepth( )
{
	return m_project->get_bitdepth();
}

int Sheet::get_rate( )
{
	return m_project->get_rate();
}

nframes_t Sheet::get_first_visible_frame( ) const
{
        return m_firstVisibleFrame;
}

DiskIO * Sheet::get_diskio( ) const
{
	return m_diskio;
}

AudioClipManager * Sheet::get_audioclip_manager( ) const
{
	return m_acmanager;
}

QString Sheet::get_audio_sources_dir() const
{
        if (m_audioSourcesDir.isEmpty() || m_audioSourcesDir.isNull()) {
                printf("no audio sources dir set, returning projects one\n");
                return m_project->get_audiosources_dir();
        }

        return m_audioSourcesDir + "/";
}

void Sheet::set_audio_sources_dir(const QString &dir)
{
        if (dir.isEmpty() || dir.isNull()) {
                m_audioSourcesDir = m_project->get_audiosources_dir();
                return;
        }

        // We're having our own audio sources dir, do the usual checks.
        m_audioSourcesDir = dir;



        QDir asDir;

        if (!asDir.exists(m_audioSourcesDir)) {
                printf("creating new audio sources dir: %s\n", dir.toAscii().data());
                if (asDir.mkdir(m_audioSourcesDir) < 0) {
                        info().critical(tr("Cannot create dir %1").arg(m_audioSourcesDir));
                }
        }

        printf("created new audiosrouces dir here: %s\n", m_audioSourcesDir.toAscii().data());
}

void Sheet::handle_diskio_readbuffer_underrun( )
{
	if (is_transport_rolling()) {
		printf("Sheet:: DiskIO ReadBuffer UnderRun signal received!\n");
		info().critical(tr("Hard Disk overload detected!"));
		info().critical(tr("Failed to fill ReadBuffer in time"));
	}
}

void Sheet::handle_diskio_writebuffer_overrun( )
{
	if (is_transport_rolling()) {
		printf("Sheet:: DiskIO WriteBuffer OverRun signal received!\n");
		info().critical(tr("Hard Disk overload detected!"));
		info().critical(tr("Failed to empty WriteBuffer in time"));
	}
}


TimeRef Sheet::get_last_location() const
{
	TimeRef lastAudio = m_acmanager->get_last_location();
	
	if (m_timeline->get_markers().size()) {
		TimeRef lastMarker = m_timeline->get_markers().last()->get_when();
		return (lastAudio > lastMarker) ? lastAudio : lastMarker;
	}
	
	return lastAudio;
}

void Sheet::private_add_track(Track* track)
{
        switch (track->get_type()) {
        case Track::AUDIOTRACK: m_audioTracks.append(track);
              break;
        case Track::SUBGROUP: m_subGroups.append(track);
              break;
        default: ;// do nothing
        }
}

void Sheet::private_remove_track(Track* track)
{
        switch (track->get_type()) {
        case Track::AUDIOTRACK: m_audioTracks.remove(track);
              break;
        case Track::SUBGROUP: m_subGroups.remove(track);
              break;
        default: ;// do nothing
        }
}

Track* Sheet::get_track(qint64 id)
{
        QList<Track*> tracks = get_tracks();
        tracks.append(m_masterOut);
        foreach(Track* track, tracks) {
		if (track->get_id() == id) {
			return track;
		}
	}
	return 0;
}

Command* Sheet::set_editing_mode( )
{
	m_mode = EDIT;
	emit modeChanged();
	return 0;
}

Command* Sheet::set_effects_mode( )
{
	m_mode = EFFECTS;
	emit modeChanged();
	return 0;
}

Command* Sheet::toggle_effects_mode()
{
        if (m_mode == EDIT) {
                set_effects_mode();
        } else {
                set_editing_mode();
        }
        return 0;
}

void Sheet::set_temp_follow_state(bool state)
{
	emit tempFollowChanged(state);
}

// Function is only to be called from GUI thread.
Command * Sheet::set_recordable()
{
#if defined (THREAD_CHECK)
	Q_ASSERT(QThread::currentThreadId() == threadId);
#endif
	
	// Do nothing if transport is rolling!
	if (is_transport_rolling()) {
		return 0;
	}
	
	// Transport is not rolling, it's save now to switch 
	// recording state to on /off
	if (is_recording()) {
		set_recording(false, false);
	} else {
                if (!any_audio_track_armed()) {
			info().critical(tr("No Tracks armed for recording!"));
			return 0;
		}
		
		set_recording(true, false);
	}
	
	return 0;
}

// Function is only to be called from GUI thread.
Command* Sheet::set_recordable_and_start_transport()
{
	if (!is_recording()) {
		set_recordable();
	}
	
	start_transport();
	
	return 0;
}

// Function is only to be called from GUI thread.
Command* Sheet::start_transport()
{
#if defined (THREAD_CHECK)
	Q_ASSERT(QThread::currentThreadId() == threadId);
#endif
	// Delegate the transport start (or if we are rolling stop)
	// request to the audiodevice. Depending on the driver in use
	// this call will return directly to us (by a call to transport_control),
	// or handled by the driver
	if (is_transport_rolling()) {
                audiodevice().transport_stop(m_audiodeviceClient, m_transportLocation);
	} else {
		audiodevice().transport_start(m_audiodeviceClient);
	}
	
	return ie().succes();
}

// Function can be called either from the GUI or RT thread.
// So ALL functions called here need to be RT thread save!!
int Sheet::transport_control(transport_state_t state)
{
        if (m_scheduledForDeletion) {
                return true;
        }

        switch(state.transport) {
	case TransportStopped:
                if (state.location != m_transportLocation) {
                        initiate_seek_start(state.location);
                }
                if (is_transport_rolling()) {
			stop_transport_rolling();
			if (is_recording()) {
				set_recording(false, state.realtime);
			}
		}
		return true;
	
	case TransportStarting:
                printf("TransportStarting\n");
		if (state.location != m_transportLocation) {
                        initiate_seek_start(state.location);
                        return false;
		}
		if (! m_seeking) {
			if (is_recording()) {
				if (!m_prepareRecording) {
					m_prepareRecording = true;
					// prepare_recording() is only to be called from the GUI thread
					// so we delegate the prepare_recording() function call via a 
					// RT thread save signal!
					Q_ASSERT(state.realtime);
					RT_THREAD_EMIT(this, 0, prepareRecording());
					PMESG("transport starting: initiating prepare for record");
					return false;
				}
				if (!m_readyToRecord) {
					PMESG("transport starting: still preparing for record");
					return false;
				}
			}
					
					
			PMESG("tranport starting: seek finished");
			return true;
		} else {
			PMESG("tranport starting: still seeking");
			return false;
		}
	
	case TransportRolling:
		if (!is_transport_rolling()) {
			// When the transport rolling request came from a non slave
			// driver, we currently can assume it's comming from the GUI 
			// thread, and TransportStarting never was called before!
			// So in case we are recording we have to prepare for recording now!
			if ( ! state.isSlave && is_recording() ) {
				Q_ASSERT(!state.realtime);
				prepare_recording();
			}
			start_transport_rolling(state.realtime);
		}
		return true;
	}
	
	return false;
}

void Sheet::initiate_seek_start(TimeRef location)
{
        if ( ! m_seeking ) {
                m_newTransportLocation = location;
                m_startSeek = 1;
                m_seeking = 1;

                PMESG("tranport starting: initiating seek");
        }
}

// RT thread save function
void Sheet::start_transport_rolling(bool realtime)
{
	m_realtimepath = true;
	m_transport = 1;
	
	if (realtime) {
		RT_THREAD_EMIT(this, 0, transportStarted());
	} else {
		emit transportStarted();
	}
	
	PMESG("transport rolling");
}

// RT thread save function
void Sheet::stop_transport_rolling()
{
	m_stopTransport = 1;
	PMESG("transport stopped");
}

// RT thread save function
void Sheet::set_recording(bool recording, bool realtime)
{
	m_recording = recording;
	
	if (!m_recording) {
		m_readyToRecord = false;
		m_prepareRecording = false;
	}
	
	if (realtime) {
		RT_THREAD_EMIT(this, 0, recordingStateChanged());
	} else {
		emit recordingStateChanged();
	}
}


// NON RT thread save function, should only be called from GUI thread!!
void Sheet::prepare_recording()
{
#if defined (THREAD_CHECK)
	Q_ASSERT(QThread::currentThreadId() == threadId);
#endif
	
        if (m_recording && any_audio_track_armed()) {
		CommandGroup* group = new CommandGroup(this, "");
		int clipcount = 0;
                apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
			if (track->armed()) {
				AudioClip* clip = track->init_recording();
				if (clip) {
					// For autosave purposes, we connect the recordingfinished
					// signal to the clip_finished_recording() slot, and add this
					// clip to our recording clip list.
					// At the time the cliplist is empty, we're sure the recording 
					// session is finished, at which time an autosave makes sense.
					connect(clip, SIGNAL(recordingFinished(AudioClip*)), 
						this, SLOT(clip_finished_recording(AudioClip*)));
					m_recordingClips.append(clip);
					
					group->add_command(new AddRemoveClip(clip, AddRemoveClip::ADD));
					clipcount++;
				}
			}
		}
		group->setText(tr("Recording to %n Clip(s)", "", clipcount));
		Command::process_command(group);
	}
	
	m_readyToRecord = true;
}

void Sheet::clip_finished_recording(AudioClip * clip)
{
	if (!m_recordingClips.removeAll(clip)) {
		PERROR("clip %s was not in recording clip list, cannot remove it!", QS_C(clip->get_name()));
	}
	
	if (m_recordingClips.isEmpty()) {
		// seems we finished recording completely now
		// all clips have set their resulting ReadSource
		// length and whatsoever, let's do an autosave:
		m_project->save(true);
	}
}


void Sheet::set_transport_pos(TimeRef location)
{
        if (location < TimeRef()) {
                // do nothing
                return;
        }

#if defined (THREAD_CHECK)
	Q_ASSERT(QThread::currentThreadId() ==  threadId);
#endif
        printf("sheet: set transport to: %lld\n", location.universal_frame());
	audiodevice().transport_seek_to(m_audiodeviceClient, location);
}


//
//  Function is ALWAYS called in RealTime AudioThread processing path
//  Be EXTREMELY carefull to not call functions() that have blocking behavior!!
//
void Sheet::start_seek()
{
#if defined (THREAD_CHECK)
	Q_ASSERT(threadId != QThread::currentThreadId ());
#endif
	
	if (is_transport_rolling()) {
		m_realtimepath = false;
		m_resumeTransport = true;
	}

	m_transport = false;
	m_startSeek = 0;
	
	// only sets a boolean flag, save to call.
	m_diskio->prepare_for_seek();

	// 'Tell' the diskio it should start a seek action.
	RT_THREAD_EMIT(this, NULL, seekStart());
}

void Sheet::seek_finished()
{
#if defined (THREAD_CHECK)
	Q_ASSERT_X(threadId == QThread::currentThreadId (), "Sheet::seek_finished", "Called from other Thread!");
#endif
	PMESG2("Sheet :: entering seek_finished");
	m_transportLocation  = m_newTransportLocation;
        printf("seek finished, setting transport location to %lld\n", m_transportLocation.universal_frame());
	m_seeking = 0;

	if (m_resumeTransport) {
		start_transport_rolling(false);
		m_resumeTransport = false;
	}

	emit transportPosSet();
	PMESG2("Sheet :: leaving seek_finished");
}

void Sheet::config_changed()
{
	int quality = config().get_property("Conversion", "RTResamplingConverterType", DEFAULT_RESAMPLE_QUALITY).toInt();
	if (m_diskio->get_resample_quality() != quality) {
		m_diskio->set_resample_quality(quality);
	}
}



QList< AudioTrack * > Sheet::get_audio_tracks() const
{
	QList<AudioTrack*> list;
        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		list.append(track);
	}
	return list;	
}

QList<SubGroup*> Sheet::get_subgroups() const
{
        QList<SubGroup*> list;
        apill_foreach(SubGroup* group, SubGroup, m_subGroups) {
                list.append(group);
        }

        return list;
}

SubGroup* Sheet::get_subgroup(const QString &name)
{
        apill_foreach(SubGroup* group, SubGroup, m_subGroups) {
                if (group->get_name() == name) {
                        return group;
                }
        }
        return 0;
}

QList<Track*> Sheet::get_tracks() const
{
        QList<Track*> list;
        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
                list.append(track);
        }
        apill_foreach(SubGroup* group, SubGroup, m_subGroups) {
                list.append(group);
        }
        return list;

}

AudioTrack * Sheet::get_audio_track_for_index(int index)
{
        apill_foreach(AudioTrack* track, AudioTrack, m_audioTracks) {
		if (track->get_sort_index() == index) {
			return track;
		}
	}
	
	return 0;
}


// the timer is used to allow 'hopping' to the left from snap position to snap position
// even during playback.
Command* Sheet::prev_skip_pos()
{
        if (m_snaplist->was_dirty()) {
		update_skip_positions();
	}

	TimeRef p = get_transport_location();

	if (p < TimeRef()) {
		PERROR("pos < 0");
		set_transport_pos(TimeRef());
		return ie().failure();
	}

	QListIterator<TimeRef> it(m_xposList);

	it.toBack();

	int steps = 1;

	if (m_skipTimer.isActive()) 
	{
		++steps;
	}

	int i = 0;
	while (it.hasPrevious()) {
		TimeRef pos = it.previous();
		if (pos < p) {
			p = pos;
			++i;
		}
		if (i >= steps) {
			break;
		}
	}

	set_transport_pos(p);
	
	m_skipTimer.start(500);
	
	return ie().succes();
}

Command* Sheet::next_skip_pos()
{
        if (m_snaplist->was_dirty()) {
		update_skip_positions();
	}

	TimeRef p = get_transport_location();

	if (p > m_xposList.last()) {
		PERROR("pos > last snap position");
		return ie().failure();
	}

	QListIterator<TimeRef> it(m_xposList);

	int i = 0;
	int steps = 1;
	
	while (it.hasNext()) {
		TimeRef pos = it.next();
		if (pos > p) {
			p = pos;
			++i;
		}
		if (i >= steps) {
			break;
		}
	}

	set_transport_pos(p);
	
	return ie().succes();
}

void Sheet::update_skip_positions()
{
	m_xposList.clear();

	// store the beginning of the sheet and the work cursor
	m_xposList << TimeRef();
	m_xposList << get_work_location();

	// store all clip borders
	QList<AudioClip* > acList = get_audioclip_manager()->get_clip_list();
	for (int i = 0; i < acList.size(); ++i) {
		m_xposList << acList.at(i)->get_track_start_location();
		m_xposList << acList.at(i)->get_track_end_location();
	}

	// store all marker positions
	QList<Marker*> markerList = get_timeline()->get_markers();
	for (int i = 0; i < markerList.size(); ++i) {
		m_xposList << markerList.at(i)->get_when();
	}

	// remove duplicates
	QMutableListIterator<TimeRef> it(m_xposList);
	while (it.hasNext()) {
		TimeRef val = it.next();
		if (m_xposList.count(val) > 1) {
			it.remove();
		}
	}

	qSort(m_xposList);
}

void Sheet::skip_to_start()
{
	set_transport_pos((TimeRef()));
	set_work_at((TimeRef()));
}

void Sheet::skip_to_end()
{
	// stop the transport, no need to play any further than the end of the sheet
	if (is_transport_rolling())
	{
		start_transport();
	}
	set_transport_pos(get_last_location());
}

//eof
