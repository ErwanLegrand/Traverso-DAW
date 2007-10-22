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

#include <QTextStream>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QRegExp>
#include <QDebug>

#include <libtraverso.h>
#include <commands.h>

#include "AbstractAudioReader.h"
#include <Client.h>
#include "ProjectManager.h"
#include "ContextPointer.h"
#include "Information.h"
#include "Song.h"
#include "Project.h"
#include "Track.h"
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


Song::Song(Project* project)
	: ContextItem()
	, m_project(project)
{
	PENTERCONS;
	title = tr("Untitled");
	m_id = create_id();
	artists = tr("No artists name set");
	m_hzoom = config().get_property("Song", "hzoomLevel", 14).toInt();

	init();
}

Song::Song(Project* project, int numtracks)
	: ContextItem()
	, m_project(project)
{
	PENTERCONS;
	title = tr("Untitled");
	m_id = create_id();
	artists = tr("No artists name set");
	m_hzoom = config().get_property("Song", "hzoomLevel", 14).toInt();

	init();

	for (int i=1; i <= numtracks; i++) {
		Track* track = create_track();
		private_add_track(track);
	}
}

Song::Song(Project* project, const QDomNode node)
		: ContextItem(), m_project(project)
{
	PENTERCONS;
	init();
	set_state( node );
}

Song::~Song()
{
	PENTERDES;

	delete [] mixdown;
	delete [] gainbuffer;

	delete m_diskio;
	delete m_masterOut;
	delete m_renderBus;
	delete m_hs;
	delete m_audiodeviceClient;
	delete snaplist;
	delete workSnap;
}

void Song::init()
{
	PENTER2;
#if defined (THREAD_CHECK)
	threadId = QThread::currentThreadId ();
#endif

	m_diskio = new DiskIO(this);
	m_currentSampleRate = audiodevice().get_sample_rate();
	m_diskio->output_rate_changed(m_currentSampleRate);
	
	connect(this, SIGNAL(seekStart()), m_diskio, SLOT(seek()), Qt::QueuedConnection);
	connect(this, SIGNAL(prepareRecording()), this, SLOT(prepare_recording()));
	connect(&audiodevice(), SIGNAL(clientRemoved(Client*)), this, SLOT (audiodevice_client_removed(Client*)));
	connect(&audiodevice(), SIGNAL(started()), this, SLOT(audiodevice_started()));
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(audiodevice_params_changed()), Qt::DirectConnection);
	connect(m_diskio, SIGNAL(seekFinished()), this, SLOT(seek_finished()), Qt::QueuedConnection);
	connect (m_diskio, SIGNAL(readSourceBufferUnderRun()), this, SLOT(handle_diskio_readbuffer_underrun()));
	connect (m_diskio, SIGNAL(writeSourceBufferOverRun()), this, SLOT(handle_diskio_writebuffer_overrun()));
	connect(this, SIGNAL(transferStarted()), m_diskio, SLOT(start_io()));
	connect(this, SIGNAL(transferStopped()), m_diskio, SLOT(stop_io()));

	mixdown = gainbuffer = 0;
	m_masterOut = new AudioBus("Master Out", 2);
	m_renderBus = new AudioBus("Render Bus", 2);
	resize_buffer(false, audiodevice().get_buffer_size());
	m_hs = new QUndoStack(pm().get_undogroup());
	set_history_stack(m_hs);
	m_acmanager = new AudioClipManager(this);
	
	set_context_item( m_acmanager );

	m_playBackBus = audiodevice().get_playback_bus("Playback 1");

	m_transport = m_stopTransport = m_resumeTransport = m_readyToRecord = false;
	snaplist = new SnapList(this);
	workSnap = new Snappable();
	workSnap->set_snap_list(snaplist);

	m_realtimepath = false;
	m_scheduledForDeletion = false;
	m_isSnapOn=true;
	changed = m_rendering = m_recording = m_prepareRecording = false;
	firstVisibleFrame=0;
	m_workLocation=0;
	m_seeking = m_startSeek = 0;
	// TODO seek to old position on project exit ?
// 	m_transportFrame = 0;
	m_transportLocation = 0;
	m_mode = EDIT;
	m_sbx = m_sby = 0;
	
	m_pluginChain = new PluginChain(this, this);
	m_fader = m_pluginChain->get_fader();
	m_fader->set_gain(0.5);
	m_timeline = new TimeLine(this);
	
	m_audiodeviceClient = new Client("song_" + QByteArray::number(get_id()));
	m_audiodeviceClient->set_process_callback( MakeDelegate(this, &Song::process) );
	m_audiodeviceClient->set_transport_control_callback( MakeDelegate(this, &Song::transport_control) );
}

int Song::set_state( const QDomNode & node )
{
	PENTER;
	QDomNode propertiesNode = node.firstChildElement("Properties");
	m_id = node.toElement().attribute("id", "0").toLongLong();
	if (m_id == 0) {
		m_id = create_id();
	}

	QDomElement e = propertiesNode.toElement();

	title = e.attribute( "title", "" );
	artists = e.attribute( "artists", "" );
	set_gain(e.attribute( "mastergain", "1.0").toFloat() );
	set_hzoom(e.attribute("hzoom", "" ).toInt());
	m_sbx = e.attribute("sbx", "0").toInt();
	m_sby = e.attribute("sby", "0").toInt();
	set_first_visible_frame(e.attribute( "firstVisibleFrame", "0" ).toUInt());
	
	bool ok;
	TimeRef location(e.attribute( "m_workLocation", "0").toLongLong(&ok));
	set_work_at(location);
	m_transportLocation = TimeRef(e.attribute( "transportlocation", "0").toLongLong(&ok));
	
	// Start seeking to the 'old' transport pos
	set_transport_pos(m_transportLocation);
	set_snapping(e.attribute("snapping", "0").toInt());
	m_mode = e.attribute("mode", "0").toInt();
	
	m_timeline->set_state(node.firstChildElement("TimeLine"));

	QDomNode tracksNode = node.firstChildElement("Tracks");
	QDomNode trackNode = tracksNode.firstChild();

	while(!trackNode.isNull()) {
		Track* track = new Track(this, trackNode);
		private_add_track(track);
		track->set_state(trackNode);

		trackNode = trackNode.nextSibling();
	}

	QDomNode pluginChainNode = node.firstChildElement("PluginChain");
	m_pluginChain->set_state(pluginChainNode);
	
	return 1;
}

QDomNode Song::get_state(QDomDocument doc, bool istemplate)
{
	QDomElement songNode = doc.createElement("Sheet");
	
	if (! istemplate) {
		songNode.setAttribute("id", m_id);
	}
	
	QDomElement properties = doc.createElement("Properties");
	properties.setAttribute("title", title);
	properties.setAttribute("artists", artists);
	properties.setAttribute("firstVisibleFrame", firstVisibleFrame);
	properties.setAttribute("m_workLocation", m_workLocation.universal_frame());
	properties.setAttribute("transportlocation", m_transportLocation.universal_frame());
	properties.setAttribute("hzoom", m_hzoom);
	properties.setAttribute("sbx", m_sbx);
	properties.setAttribute("sby", m_sby);
	properties.setAttribute("snapping", m_isSnapOn);
	properties.setAttribute("mode", m_mode);
	songNode.appendChild(properties);

	doc.appendChild(songNode);

	songNode.appendChild(m_timeline->get_state(doc));
	
	QDomNode tracksNode = doc.createElement("Tracks");

	foreach(Track* track, m_tracks) {
		tracksNode.appendChild(track->get_state(doc, istemplate));
	}

	songNode.appendChild(tracksNode);

	QDomNode pluginChainNode = doc.createElement("PluginChain");
	pluginChainNode.appendChild(m_pluginChain->get_state(doc));
	songNode.appendChild(pluginChainNode);


	return songNode;
}

void Song::connect_to_audiodevice( )
{
	PENTER;
	audiodevice().add_client(m_audiodeviceClient);
}

void Song::disconnect_from_audiodevice()
{
	PENTER;
	if (is_transport_rolling()) {
		m_transport = false;
	}
	audiodevice().remove_client(m_audiodeviceClient);
}

void Song::schedule_for_deletion()
{
	m_scheduledForDeletion = true;
	pm().scheduled_for_deletion(this);
}

void Song::audiodevice_client_removed(Client* client )
{
	PENTER;
	if (m_audiodeviceClient == client) {
		if (m_scheduledForDeletion) {
			pm().delete_song(this);
		}
	}
}

Command* Song::add_track(Track* track, bool historable)
{
	foreach(Track* existingTrack, m_tracks) {
		if (existingTrack->is_solo()) {
			track->set_muted_by_solo( true );
			break;
		}
	}

	return new AddRemove(this, track, historable, this,
		"private_add_track(Track*)", "trackAdded(Track*)",
		"private_remove_track(Track*)", "trackRemoved(Track*)",
   		tr("Add Track"));
}


Command* Song::remove_track(Track* track, bool historable)
{
	return new AddRemove(this, track, historable, this,
		"private_remove_track(Track*)", "trackRemoved(Track*)",
		"private_add_track(Track*)", "trackAdded(Track*)",
   		tr("Remove Track"));
}

bool Song::any_track_armed()
{
	foreach(Track* track, m_tracks) {
		if (track->armed()) {
			return true;
		}
	}
	return false;
}


int Song::prepare_export(ExportSpecification* spec)
{
	PENTER;
	
	if ( ! (spec->renderpass == ExportSpecification::CREATE_CDRDAO_TOC) ) {
		if (is_transport_rolling()) {
			spec->resumeTransport = true;
			// When transport is rolling, this equals stopping the transport!
			// prepare_export() is called from another thread, so use a queued connection
			// to call the function in the correct thread!
			if (!QMetaObject::invokeMethod(this, "start_transport",  Qt::QueuedConnection)) {
				printf("Invoking Song::start_transport() failed\n");
				return -1;
			}
			int count = 0;
			uint msecs = (audiodevice().get_buffer_size() * 1000) / audiodevice().get_sample_rate();
			// wait a number (max 10) of process() cycles to be sure we really stopped transport
			while (m_transport) {
				spec->thread->sleep_for(msecs);
				if (count > 10) {
					break;
				}
			}
		}
		
		m_rendering = true;
	}

	spec->startLocation = LONG_LONG_MAX;
	spec->endLocation = 0;

	TimeRef endlocation, startlocation;

	foreach (Track* track, m_tracks) {
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
		QList<Marker*> markers = m_timeline->get_markers();
		if (markers.size() >= 2) {
			startlocation = markers.at(0)->get_when();
			PMESG2("  Start marker found at %s", QS_C(timeref_to_ms(startlocation)));
			// round down to the start of the CD frome (75th of a sec)
			startlocation = cd_to_timeref(timeref_to_cd(startlocation));
			spec->startLocation = startlocation;
		} else {
			PMESG2("  No start marker found");
		}
		
		if (m_timeline->get_end_position(endlocation)) {
			PMESG2("  End marker found at %s", QS_C(timeref_to_ms(endlocation)));
			spec->endLocation = endlocation;
		} else {
			PMESG2("  No end marker found");
		}
	}

	spec->totalTime = spec->endLocation - spec->startLocation;

// 	PWARN("Render length is: %s",timeref_to_ms_3(spec->totalTime).toAscii().data() );

	spec->pos = spec->startLocation;
	spec->progress = 0;

	spec->basename = "Song" + QString::number(m_project->get_song_index(m_id)) +"-" + title;
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
	
	if (spec->renderpass == ExportSpecification::WRITE_TO_HARDDISK) {
		m_exportSource = new WriteSource(spec);
		if (m_exportSource->prepare_export() == -1) {
			delete m_exportSource;
			return -1;
		}
	}

	m_transportLocation = spec->startLocation;
	
	resize_buffer(false, spec->blocksize);
	
	renderDecodeBuffer = new DecodeBuffer;
// 	renderDecodeBuffer->use_custom_destination_buffer(true);

	return 1;
}

int Song::finish_audio_export()
{
	m_exportSource->finish_export();
	delete m_exportSource;
	delete renderDecodeBuffer;
	resize_buffer(false, audiodevice().get_buffer_size());
	return 0;
}

int Song::render(ExportSpecification* spec)
{
	int chn;
	uint32_t x;
	int ret = -1;
	int progress;
	
	nframes_t diff = (spec->endLocation - spec->pos).to_frame(audiodevice().get_sample_rate());
	nframes_t nframes = spec->blocksize;
	nframes_t this_nframes = std::min(diff, nframes);

	if (!spec->running || spec->stop || this_nframes == 0) {
		process_export (nframes);
		/*		PWARN("Finished Rendering for this song");
				PWARN("running is %d", spec->running);
				PWARN("stop is %d", spec->stop);
				PWARN("this_nframes is %d", this_nframes);*/
		if (spec->renderpass == ExportSpecification::WRITE_TO_HARDDISK) {
			return finish_audio_export();
		} else {
			return 0;
		}
	}

	/* do the usual stuff */

	process_export(nframes);

	/* and now export the results */

	nframes = this_nframes;

	memset (spec->dataF, 0, sizeof (spec->dataF[0]) * nframes * spec->channels);

	/* foreach output channel ... */

	float* buf;

	for (chn = 0; chn < spec->channels; ++chn) {
		buf = m_masterOut->get_buffer(chn, nframes);

		if (!buf) {
			// Seem we are exporting at least to Stereo from an AudioBus with only one channel...
			// Use the first channel..
			buf = m_masterOut->get_buffer(0, nframes);
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

	if (! spec->normalize ) {
		progress =  int((double(spec->pos.universal_frame()) / spec->totalTime.universal_frame()) * 100);
	} else {
		progress = (int) (double( 100 * spec->pos.universal_frame()) / (spec->totalTime.universal_frame() * 2));
		if (spec->renderpass == ExportSpecification::WRITE_TO_HARDDISK) {
			progress += 50;
		}
	}
	
	if (progress > spec->progress) {
		spec->progress = progress;
		m_project->set_song_export_progress(progress);
	}


	/* and we're good to go */

	ret = 1;

out:
	if (!ret) {
		spec->running = false;
		spec->status = ret;
		m_rendering = false;
	}

	return ret;
}


SnapList* Song::get_snap_list() const
{
	return snaplist;
}


void Song::set_artists(const QString& pArtists)
{
	artists = pArtists;
}

void Song::set_gain(float gain)
{
	if (gain < 0.0)
		gain = 0.0;
	if (gain > 2.0)
		gain = 2.0;

	m_fader->set_gain(gain);

	emit masterGainChanged();
}

void Song::set_title(const QString& sTitle)
{
	title=sTitle;
	emit propertyChanged();
}

void Song::set_first_visible_frame(nframes_t pos)
{
	PENTER;
	firstVisibleFrame = pos;
	emit firstVisibleFrameChanged();
}

void Song::set_work_at(const TimeRef& location)
{
	m_workLocation = location;
	snaplist->mark_dirty(workSnap);
	emit workingPosChanged();
}

Command* Song::toggle_snap()
{
	set_snapping( ! m_isSnapOn );
	return 0;
}


void Song::set_snapping(bool snapping)
{
	m_isSnapOn = snapping;
	emit snapChanged();
}

/******************************** SLOTS *****************************/

Track* Song::create_track()
{
	int height = Track::INITIAL_HEIGHT;

	Track* track = new Track(this, "Unnamed", height);

	return track;
}

void Song::solo_track(Track* t)
{
	bool wasSolo = t->is_solo();

	t->set_muted_by_solo(!wasSolo);
	t->set_solo(!wasSolo);

	bool hasSolo = false;
	foreach(Track* track, m_tracks) {
		track->set_muted_by_solo(!track->is_solo());
		if (track->is_solo()) hasSolo = true;
	}

	if (!hasSolo) {
		foreach(Track* track, m_tracks) {
			track->set_muted_by_solo(false);
		}
	}
}

Command* Song::toggle_solo()
{
	bool hasSolo = false;
	foreach(Track* track, m_tracks) {
		if (track->is_solo()) hasSolo = true;
	}

	foreach(Track* track, m_tracks) {
		track->set_solo(!hasSolo);
		track->set_muted_by_solo(false);
	}

	return (Command*) 0;
}

Command *Song::toggle_mute()
{
	bool hasMute = false;
	foreach(Track* track, m_tracks) {
		if (track->is_muted()) hasMute = true;
	}

	foreach(Track* track, m_tracks) {
		track->set_muted(!hasMute);
	}

	return (Command*) 0;
}

Command *Song::toggle_arm()
{
	bool hasArmed = false;
	foreach(Track* track, m_tracks) {
		if (track->armed()) hasArmed = true;
	}

	foreach(Track* track, m_tracks) {
		if (hasArmed) {
			track->disarm();
		} else {
			track->arm();
		}
	}

	return (Command*) 0;
}

Command* Song::work_next_edge()
{
/*	nframes_t w = m_acmanager->get_last_frame();

	foreach(Track* track, m_tracks) {
		AudioClip* c=track->get_clip_after(m_workLocation);

		if ((c) && (c->get_track_start_location() < w && c->get_track_start_location() > m_workLocation))
			w = c->get_track_start_location();
	}

	set_work_at(w);

	emit setCursorAtEdge();
*/
	return (Command*) 0;
}

Command* Song::work_previous_edge()
{
/*	TimeRef w(0);
	foreach(Track* track, m_tracks) {
		AudioClip* c = track->get_clip_before(m_workLocation);
		if ((c) && (c->get_track_end_location() >= w && c->get_track_end_location() < m_workLocation) )
			w=c->get_track_end_location();
	}

	set_work_at(w);

	emit setCursorAtEdge();
*/
	return (Command*) 0;
}

void Song::set_hzoom( int hzoom )
{
	if (hzoom > (Peak::ZOOM_LEVELS - 1))
		hzoom = (Peak::ZOOM_LEVELS - 1);
	if (hzoom  < 0)
		hzoom = 0;
	m_hzoom = hzoom;
	emit hzoomChanged();
}

//
//  Function called in RealTime AudioThread processing path
//
int Song::process( nframes_t nframes )
{
	if (m_startSeek) {
		start_seek();
		return 0;
	}
	
	// If no need for playback/record, return.
	if (!is_transport_rolling()) {
		return 0;
	}

	if (m_stopTransport) {
		RT_THREAD_EMIT(this, 0, transferStopped());
		m_transport = false;
		m_realtimepath = false;
		m_stopTransport = false;

		return 0;
	}

	// zero the m_masterOut buffers
	m_masterOut->silence_buffers(nframes);

	int processResult = 0;

	// Process all Tracks.
	for (int i=0; i<m_tracks.size(); ++i) {
		processResult |= m_tracks.at(i)->process(nframes);
	}

	// update the m_transportFrame
// 	m_transportFrame += nframes;
	m_transportLocation.add_frames(nframes, audiodevice().get_sample_rate());

	if (!processResult) {
		return 0;
	}

	// Mix the result into the AudioDevice "physical" buffers
	if (m_playBackBus) {
		Mixer::mix_buffers_with_gain(m_playBackBus->get_buffer(0, nframes), m_masterOut->get_buffer(0, nframes), nframes, get_gain());
		Mixer::mix_buffers_with_gain(m_playBackBus->get_buffer(1, nframes), m_masterOut->get_buffer(1, nframes), nframes, get_gain());
		
		m_pluginChain->process_post_fader(m_masterOut, nframes);
	}

	
	return 1;
}

int Song::process_export( nframes_t nframes )
{
	// Get the masterout buffers, and fill with zero's
	m_masterOut->silence_buffers(nframes);
	memset (mixdown, 0, sizeof (audio_sample_t) * nframes);

	// Process all Tracks.
	for (int i=0; i<m_tracks.size(); ++i) {
		m_tracks.at(i)->process(nframes);
	}

	Mixer::apply_gain_to_buffer(m_masterOut->get_buffer(0, nframes), nframes, get_gain());
	Mixer::apply_gain_to_buffer(m_masterOut->get_buffer(1, nframes), nframes, get_gain());

	// update the m_transportFrame
// 	m_transportFrame += nframes;
	m_transportLocation.add_frames(nframes, audiodevice().get_sample_rate());

	return 1;
}

QString Song::get_cdrdao_tracklist(ExportSpecification* spec, bool pregap)
{
	QString output;

	QList<Marker*> mlist = m_timeline->get_markers();

	// Here we make the marker-stuff idiot-proof ;-). Traverso doesn't insist on having any
	// marker at all, so we need to handle cases like:
	// - no markers at all
	// - one marker (doesn't make sense)
	// - enough markers, but no end marker

	if (mlist.size() < 2) {
		switch (mlist.size()) {
			case 0:
				// no markers present. We add one at the beginning and one at the
				// end of the render area.
				mlist.append(new Marker(m_timeline, spec->startLocation, Marker::TEMP_CDTRACK));
				mlist.append(new Marker(m_timeline, spec->endLocation, Marker::TEMP_ENDMARKER));
				break;
			case 1:
				// one marker is present. We add two more at the beginning
				// and at the end of the render area. But we must check if 
				// the present marker happens to be at one of these positions.

				// deactivate the next if-condition (only the first one) if you want the
				// stuff before the first marker to go into the pre-gap
				if (mlist.at(0)->get_when() != (spec->startLocation)) {
					mlist.append(new Marker(m_timeline, spec->startLocation, Marker::TEMP_CDTRACK));
				}
				if (mlist.at(0)->get_when() != spec->endLocation) {
					mlist.append(new Marker(m_timeline, spec->endLocation, Marker::TEMP_ENDMARKER));
				}
				break;
		}
	} else {
		// would be ok, but let's check if there is an end marker present. If not,
		// add one to spec->end_frame
		if (!m_timeline->has_end_marker()) {
			mlist.append(new Marker(m_timeline, spec->endLocation, Marker::TEMP_ENDMARKER));
		}
	}

	// Sort the list according to Marker::get_when() values. This
	// is the correct way to do it according to the Qt docu.
	QMap<TimeRef, Marker*> markermap;
	foreach(Marker *marker, mlist) {
		markermap.insert(marker->get_when(), marker);
	}
	mlist = markermap.values();

	TimeRef start;
	for(int i = 0; i < mlist.size()-1; ++i) {
		Marker* startmarker = mlist.at(i);
		Marker* endmarker = mlist.at(i+1);

		output += "TRACK AUDIO\n";

		if (startmarker->get_copyprotect()) {
			output += "  NO COPY\n";
		} else {
			output += "  COPY\n";
		}

		if (startmarker->get_preemphasis()) {
			output += "  PRE_EMPHASIS\n";
		}

		output += "  CD_TEXT {\n    LANGUAGE 0 {\n";
		output += "      TITLE \"" + startmarker->get_description() + "\"\n";
		output += "      PERFORMER \"" + startmarker->get_performer() + "\"\n";
		output += "      ISRC \"" + startmarker->get_isrc() + "\"\n";
		output += "      ARRANGER \"" + startmarker->get_arranger() + "\"\n";
		output += "      SONGWRITER \"" + startmarker->get_songwriter() + "\"\n";
		output += "      MESSAGE \"" + startmarker->get_message() + "\"\n    }\n  }\n";

		// add some stuff only required for the first track (e.g. pre-gap)
		if (i == 0 && pregap) {
			//if (start == 0) {
				// standard pregap, because we have a track marker at the beginning
				output += "  PREGAP 00:02:00\n";
			//} else {
			//	// no track marker at the beginning, thus use the part from 0 to the first
			//	// track marker for the pregap
			//	output += "  START " + frame_to_cd(start, m_project->get_rate()) + "\n";
			//	start = 0;
			//}
		}
		
		TimeRef length = cd_to_timeref(timeref_to_cd(endmarker->get_when())) - cd_to_timeref(timeref_to_cd(startmarker->get_when()));
		
		QString s_start = timeref_to_cd(start);
		QString s_length = timeref_to_cd(length);

		output += "  FILE \"" + spec->name + "." + spec->extraFormat["filetype"] + "\" " + s_start + " " + s_length + "\n\n";
		start += length;

		// check if the second marker is of type "Endmarker"
		if ((endmarker->get_type() == Marker::ENDMARKER) || (endmarker->get_type() == Marker::TEMP_ENDMARKER)) {
			break;
		}
	}

	// delete all temporary markers
	foreach(Marker* marker, mlist) {
		if ((marker->get_type() == Marker::TEMP_CDTRACK) || (marker->get_type() == Marker::TEMP_ENDMARKER)) {
			delete marker;
		}
	}

	return output;
}

void Song::resize_buffer(bool updateArmStatus, nframes_t size)
{
	if (mixdown)
		delete [] mixdown;
	if (gainbuffer)
		delete [] gainbuffer;
	mixdown = new audio_sample_t[size];
	gainbuffer = new audio_sample_t[size];
	m_masterOut->set_buffer_size(size);
	m_renderBus->set_buffer_size(size);
	
	if (updateArmStatus) {
		foreach(Track* track, m_tracks) {
			AudioBus* bus = audiodevice().get_capture_bus(track->get_bus_in().toAscii());
			if (bus && track->armed()) {
				bus->set_monitor_peaks(true);
			}
		}
	}
}

void Song::audiodevice_params_changed()
{
	resize_buffer(true, audiodevice().get_buffer_size());
	
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

int Song::get_bitdepth( )
{
	return m_project->get_bitdepth();
}

int Song::get_rate( )
{
	return m_project->get_rate();
}

nframes_t Song::get_first_visible_frame( ) const
{
	return firstVisibleFrame;
}

QList<Track* > Song::get_tracks( ) const
{
	return m_tracks;
}

DiskIO * Song::get_diskio( ) const
{
	return m_diskio;
}

AudioClipManager * Song::get_audioclip_manager( ) const
{
	return m_acmanager;
}

PluginChain* Song::get_plugin_chain() const
{
	return m_pluginChain;
}

int Song::get_track_index(qint64 id) const
{
	for (int i=0; i<m_tracks.size(); ++i) {
		if (m_tracks.at(i)->get_id() == id) {
			return i + 1;
		}
	}
	return 0;
}

void Song::handle_diskio_readbuffer_underrun( )
{
	if (is_transport_rolling()) {
		printf("Song:: DiskIO ReadBuffer UnderRun signal received!\n");
		info().critical(tr("Hard Disk overload detected!"));
		info().critical(tr("Failed to fill ReadBuffer in time"));
	}
}

void Song::handle_diskio_writebuffer_overrun( )
{
	if (is_transport_rolling()) {
		printf("Song:: DiskIO WriteBuffer OverRun signal received!\n");
		info().critical(tr("Hard Disk overload detected!"));
		info().critical(tr("Failed to empty WriteBuffer in time"));
	}
}

void Song::audiodevice_started( )
{
	m_playBackBus = audiodevice().get_playback_bus("Playback 1");
}

const TimeRef& Song::get_last_location() const
{
	return m_acmanager->get_last_location();
}

void Song::private_add_track(Track* track)
{
	m_tracks.append(track);
}

void Song::private_remove_track(Track* track)
{
	m_tracks.removeAll(track);
}

Track* Song::get_track(qint64 id)
{
	for (int i=0; i<m_tracks.size(); ++i) {
		if (m_tracks.at(i)->get_id() == id) {
			return m_tracks.at(i);
		}
	}
	return 0;
}

void Song::move_clip(Track * from, Track * too, AudioClip * clip, TimeRef location)
{
	if (from == too) {
		clip->set_track_start_location(location);
		return;
	}
	
	Command::process_command(from->remove_clip(clip, false, true));
	Command::process_command(too->add_clip(clip, false, true));

	clip->set_track_start_location(location);
}

Command* Song::set_editing_mode( )
{
	m_mode = EDIT;
	emit modeChanged();
	return 0;
}

Command* Song::set_effects_mode( )
{
	m_mode = EFFECTS;
	emit modeChanged();
	return 0;
}

void Song::set_temp_follow_state(bool state)
{
	emit tempFollowChanged(state);
}

// Function is only to be called from GUI thread.
Command * Song::set_recordable()
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
		if (!any_track_armed()) {
			info().critical(tr("No Tracks armed for recording!"));
			return 0;
		}
		
		set_recording(true, false);
	}
	
	return 0;
}

// Function is only to be called from GUI thread.
Command* Song::set_recordable_and_start_transport()
{
	if (!is_recording()) {
		set_recordable();
	}
	
	start_transport();
	
	return 0;
}

// Function is only to be called from GUI thread.
Command* Song::start_transport()
{
#if defined (THREAD_CHECK)
	Q_ASSERT(QThread::currentThreadId() == threadId);
#endif
	// Delegate the transport start (or if we are rolling stop)
	// request to the audiodevice. Depending on the driver in use
	// this call will return directly to us (by a call to transport_control),
	// or handled by the driver
	if (is_transport_rolling()) {
		audiodevice().transport_stop(m_audiodeviceClient);
	} else {
		audiodevice().transport_start(m_audiodeviceClient);
	}
	
	return ie().succes();
}

// Function can be called either from the GUI or RT thread.
// So ALL functions called here need to be RT thread save!!
int Song::transport_control(transport_state_t state)
{
	if (m_scheduledForDeletion) {
		return true;
	}
	
	switch(state.tranport) {
	case TransportStopped:
		if (is_transport_rolling()) {
			stop_transport_rolling();
			if (is_recording()) {
				set_recording(false, state.realtime);
			}
		}
		return true;
	
	case TransportStarting:
		if (state.location != m_transportLocation) {
			if ( ! m_seeking ) {
				m_newTransportLocation = state.location;
				m_startSeek = 1;
				m_seeking = 1;
				
				PMESG("tranport starting: initiating seek");
				return false;
			}
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

// RT thread save function
void Song::start_transport_rolling(bool realtime)
{
	m_realtimepath = true;
	m_transport = 1;
	
	if (realtime) {
		RT_THREAD_EMIT(this, 0, transferStarted());
	} else {
		emit transferStarted();
	}
	
	PMESG("tranport rolling");
}

// RT thread save function
void Song::stop_transport_rolling()
{
	m_stopTransport = 1;
	PMESG("tranport stopped");
}

// RT thread save function
void Song::set_recording(bool recording, bool realtime)
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
void Song::prepare_recording()
{
#if defined (THREAD_CHECK)
	Q_ASSERT(QThread::currentThreadId() == threadId);
#endif
	
	if (m_recording && any_track_armed()) {
		CommandGroup* group = new CommandGroup(this, "");
		int clipcount = 0;
		foreach(Track* track, m_tracks) {
			if (track->armed()) {
				AudioClip* clip = track->init_recording();
				if (clip) {
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

void Song::set_transport_pos(TimeRef location)
{
#if defined (THREAD_CHECK)
	Q_ASSERT(QThread::currentThreadId() ==  threadId);
#endif
	audiodevice().transport_seek_to(m_audiodeviceClient, location);
}


//
//  Function is ALWAYS called in RealTime AudioThread processing path
//  Be EXTREMELY carefull to not call functions() that have blocking behavior!!
//
void Song::start_seek()
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

void Song::seek_finished()
{
#if defined (THREAD_CHECK)
	Q_ASSERT_X(threadId == QThread::currentThreadId (), "Song::seek_finished", "Called from other Thread!");
#endif
	PMESG2("Song :: entering seek_finished");
	m_transportLocation  = m_newTransportLocation;
	m_seeking = 0;

	if (m_resumeTransport) {
		start_transport_rolling(false);
		m_resumeTransport = false;
	}

	emit transportPosSet();
	PMESG2("Song :: leaving seek_finished");
}


// eof
