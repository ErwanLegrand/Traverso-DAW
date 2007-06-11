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
//	threadId = QThread::currentThreadId ();

	m_diskio = new DiskIO(this);
	
	connect(this, SIGNAL(seekStart(uint)), m_diskio, SLOT(seek(uint)), Qt::QueuedConnection);
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

	m_transport = m_stopTransport = resumeTransport = false;
	snaplist = new SnapList(this);
	workSnap = new Snappable();
	workSnap->set_snap_list(snaplist);

	realtimepath = false;
	scheduleForDeletion = false;
	m_isSnapOn=true;
	changed = m_rendering = m_recording = false;
	firstVisibleFrame=workingFrame=0;
	seeking = 0;
	// TODO seek to old position on project exit ?
	transportFrame = 0;
	m_mode = EDIT;
	m_sbx = m_sby = 0;
	
	m_pluginChain = new PluginChain(this, this);
	m_fader = m_pluginChain->get_fader();
	m_fader->set_gain(0.5);
	m_timeline = new TimeLine(this);
	
	m_audiodeviceClient = new Client("song_" + QByteArray::number(get_id()));
	m_audiodeviceClient->set_process_callback( MakeDelegate(this, &Song::process) );
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
	set_work_at(e.attribute( "workingFrame", "0").toUInt());
	transportFrame = e.attribute( "transportFrame", "0").toUInt();
	// Start seeking to the 'old' transport pos
	set_transport_pos(transportFrame);
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
	properties.setAttribute("workingFrame", (uint)workingFrame);
	properties.setAttribute("transportFrame", (uint)transportFrame);
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
	if (m_transport) {
		m_transport = false;
	}
	audiodevice().remove_client(m_audiodeviceClient);
}

void Song::schedule_for_deletion()
{
	scheduleForDeletion = true;
	pm().scheduled_for_deletion(this);
}

void Song::audiodevice_client_removed(Client* client )
{
	PENTER;
	if (m_audiodeviceClient == client) {
		if (scheduleForDeletion) {
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
		if (m_transport) {
			spec->resumeTransport = true;
			m_stopTransport = true;
		}
		
		m_rendering = true;
	}

	spec->start_frame = INT_MAX;
	spec->end_frame = 0;

	nframes_t endframe, startframe;

	foreach (Track* track, m_tracks) {
		track->get_render_range(startframe, endframe);

		if (track->is_solo()) {
			spec->end_frame = endframe;
			spec->start_frame = startframe;
			break;
		}

		if (endframe > spec->end_frame) {
			spec->end_frame = endframe;
		}

		if (startframe < (uint)spec->start_frame) {
			spec->start_frame = startframe;
		}

	}
	
	if (spec->isCdExport) {
		QList<Marker*> markers = m_timeline->get_markers();
		if (markers.size() >= 2) {
			startframe = markers.at(0)->get_when();
			PMESG2("  Start marker found at %d", startframe);
			spec->start_frame = startframe;
		} else {
			PMESG2("  No start marker found");
		}
		
		if (m_timeline->get_end_position(endframe)) {
			PMESG2("  End marker found at %d", endframe);
			spec->end_frame = endframe;
		} else {
			PMESG2("  No end marker found");
		}
	}

	spec->total_frames = spec->end_frame - spec->start_frame;

// 	PWARN("Render length is: %s",frame_to_ms_3(spec->total_frames, m_project->get_rate()).toAscii().data() );

	spec->pos = spec->start_frame;
	spec->progress = 0;

	spec->basename = "Song" + QString::number(m_project->get_song_index(m_id)) +"-" + title;
	spec->name = spec->basename + spec->extension;

	if (spec->start_frame >= spec->end_frame) {
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
	}

	transportFrame = spec->start_frame;
	
	resize_buffer(false, spec->blocksize);

	return 1;
}

int Song::finish_audio_export()
{
	m_exportSource->finish_export();
	delete m_exportSource;
	resize_buffer(false, audiodevice().get_buffer_size());
	return 0;
}

int Song::render(ExportSpecification* spec)
{
	int chn;
	uint32_t x;
	int ret = -1;
	int progress;
	nframes_t this_nframes;
	nframes_t nframes = spec->blocksize;

	if (!spec->running || spec->stop || (this_nframes = std::min ((nframes_t)(spec->end_frame - spec->pos), nframes)) == 0) {
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
	

	spec->pos += nframes;

	if (! spec->normalize ) {
		progress = (int) (( 100.0 * (float)(spec->pos) ) / spec->total_frames);
	} else {
		progress = (int) (( 100.0 * (float)(spec->pos) ) / (spec->total_frames * 2));
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

void Song::set_work_at(nframes_t pos)
{
	PENTER;

 	workingFrame = pos;
	snaplist->mark_dirty(workSnap);
	emit workingPosChanged();
}


void Song::set_transport_pos(nframes_t position)
{
	newTransportFramePos = (uint) position;
	// If there is no m_transport, start_seek() will _not_ be
	// called from within process(). So we do it now!
	if (!m_transport) {
		start_seek();
	}

	seeking = 1;
}



//
//  Function _could_ be called in RealTime AudioThread processing path
//  Be EXTREMELY carefull to not call functions() that have blocking behavior!!
//
void Song::start_seek()
{
	PMESG2("Song :: entering start_seek");
// 	PMESG2("Song :: thread id is: %ld", QThread::currentThreadId ());
	PMESG2("Song::start_seek()");
	if (m_transport) {
		realtimepath = false;
		resumeTransport = true;
	}

	// only sets a boolean flag, save to call.
	m_diskio->prepare_for_seek();

	// 'Tell' the diskio it should start a seek action.
	if (!m_transport) {
		emit seekStart(newTransportFramePos);
	} else {
		m_transport = false;
		RT_THREAD_EMIT(this, (void*)newTransportFramePos, seekStart(uint));
	}

	PMESG2("Song :: leaving start_seek");
}

void Song::seek_finished()
{
	PMESG2("Song :: entering seek_finished");
	transportFrame = newTransportFramePos;
	seeking = 0;

	if (resumeTransport) {
		m_transport = true;
		realtimepath = true;
		resumeTransport = false;
	}

	emit transportPosSet();
	PMESG2("Song :: leaving seek_finished");
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

Command* Song::go_and_record()
{
	if (!is_recording() && !is_transporting()) {
		if (!any_track_armed()) {
			info().critical(tr("No Tracks armed to record too!"));
			return 0;
		}
	}
	
	if ( ! is_transporting() && ! m_recording) {
		set_recording(true);
		return go();
	} else if (is_transporting() && m_recording) {
		set_recording(false);
		return go();
	}
	
	return 0;
}

Command* Song::go()
{
// 	printf("Song-%d::go m_transport is %d\n", m_id, m_transport);
	
	if (is_transporting() && m_recording) {
		set_recording(false);
	}
	
	if (m_transport) {
		m_stopTransport = true;
	} else {
		emit transferStarted();
		
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
		
		m_transport = true;
		realtimepath = true;
	}
	
	return ie().succes();
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
	nframes_t w = m_acmanager->get_last_frame();

	foreach(Track* track, m_tracks) {
		AudioClip* c=track->get_clip_after(workingFrame);

		if ((c) && (c->get_track_start_frame() < w && c->get_track_start_frame() > workingFrame))
			w = c->get_track_start_frame();
	}

	set_work_at(w);

	emit setCursorAtEdge();

	return (Command*) 0;
}

Command* Song::work_previous_edge()
{
	nframes_t w = 0;
	foreach(Track* track, m_tracks) {
		AudioClip* c = track->get_clip_before(workingFrame);
		if ((c) && (c->get_track_end_frame() >= w && c->get_track_end_frame() < workingFrame) )
			w=c->get_track_end_frame();
	}

	set_work_at(w);

	emit setCursorAtEdge();

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
	// If no need for playback/record, return.
// 	printf("Song-%d::process m_transport is %d\n", m_id, m_transport);
	if (!m_transport) {
		return 0;
	}

	if (m_stopTransport) {
		RT_THREAD_EMIT(this, 0, transferStopped());
		m_transport = false;
		realtimepath = false;
		m_stopTransport = false;

		return 0;
	}

	if (seeking) {
		start_seek();
		return 0;
	}
	
	// zero the m_masterOut buffers
	m_masterOut->silence_buffers(nframes);

	int processResult = 0;

	// Process all Tracks.
	for (int i=0; i<m_tracks.size(); ++i) {
		processResult |= m_tracks.at(i)->process(nframes);
	}

	// update the transportFrame
	transportFrame += nframes;

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

	// update the transportFrame
	transportFrame += nframes;

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
				mlist.append(new Marker(m_timeline, spec->start_frame, Marker::TEMP_CDTRACK));
				mlist.append(new Marker(m_timeline, spec->end_frame, Marker::TEMP_ENDMARKER));
				break;
			case 1:
				// one marker is present. We add two more at the beginning
				// and at the end of the render area. But we must check if 
				// the present marker happens to be at one of these positions.

				// deactivate the next if-condition (only the first one) if you want the
				// stuff before the first marker to go into the pre-gap
				if (mlist.at(0)->get_when() != spec->start_frame) {
					mlist.append(new Marker(m_timeline, spec->start_frame, Marker::TEMP_CDTRACK));
				}
				if (mlist.at(0)->get_when() != spec->end_frame) {
					mlist.append(new Marker(m_timeline, spec->end_frame, Marker::TEMP_ENDMARKER));
				}
				break;
		}
	} else {
		// would be ok, but let's check if there is an end marker present. If not,
		// add one to spec->end_frame
		if (!m_timeline->has_end_marker()) {
			mlist.append(new Marker(m_timeline, spec->end_frame, Marker::TEMP_ENDMARKER));
		}
	}

	// Sort the list according to Marker::get_when() values. This
	// is the correct way to do it according to the Qt docu.
	QMap<nframes_t, Marker*> markermap;
	foreach(Marker *marker, mlist) {
		markermap.insert(marker->get_when(), marker);
	}
	mlist = markermap.values();

	nframes_t start = 0;
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
		
		nframes_t length = cd_to_frame(frame_to_cd(endmarker->get_when(), m_project->get_rate()), m_project->get_rate()) - cd_to_frame(frame_to_cd(startmarker->get_when(), m_project->get_rate()), m_project->get_rate());
		
		QString s_start = frame_to_cd(start, m_project->get_rate());
		QString s_length = frame_to_cd(length, m_project->get_rate());

		output += "  FILE \"" + spec->name + "\" " + s_start + " " + s_length + "\n\n";
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
	if (m_transport) {
		printf("Song:: DiskIO ReadBuffer UnderRun signal received!\n");
		info().critical(tr("Hard Disk overload detected!"));
		info().critical(tr("Failed to fill ReadBuffer in time"));
	}
}

void Song::handle_diskio_writebuffer_overrun( )
{
	if (m_transport) {
		printf("Song:: DiskIO WriteBuffer OverRun signal received!\n");
		info().critical(tr("Hard Disk overload detected!"));
		info().critical(tr("Failed to empty WriteBuffer in time"));
	}
}

void Song::audiodevice_started( )
{
	m_playBackBus = audiodevice().get_playback_bus("Playback 1");
}

nframes_t Song::get_last_frame( ) const
{
	return m_acmanager->get_last_frame();
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

void Song::move_clip(Track * from, Track * too, AudioClip * clip, nframes_t pos)
{
	if (from == too) {
		clip->set_track_start_frame(pos);
		return;
	}
	
	Command::process_command(from->remove_clip(clip, false, true));
	Command::process_command(too->add_clip(clip, false, true));

	clip->set_track_start_frame(pos);
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

void Song::set_recording(bool recording)
{
	m_recording = recording;
	emit recordingStateChanged();
}

void Song::set_temp_follow_state(bool state)
{
	emit tempFollowChanged(state);
}

// eof

