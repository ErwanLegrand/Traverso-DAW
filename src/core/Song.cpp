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

#include <Plugin.h>
#include <PluginChain.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Song::Song(Project* project, int numtracks)
	: ContextItem()
	, m_project(project)
{
	PENTERCONS;
	title = tr("Untitled");
	m_id = create_id();
	m_gain = 1.0f;
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
	delete m_hs;
	delete m_audiodeviceClient;
	delete snaplist;
}

void Song::init()
{
	PENTER2;
//	threadId = QThread::currentThreadId ();

	m_diskio = new DiskIO(this);
	
	connect(this, SIGNAL(seekStart(uint)), m_diskio, SLOT(seek(uint)), Qt::QueuedConnection);
	connect(&audiodevice(), SIGNAL(clientRemoved(Client*)), this, SLOT (audiodevice_client_removed(Client*)));
	connect(&audiodevice(), SIGNAL(started()), this, SLOT(audiodevice_started()));
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(resize_buffer()), Qt::DirectConnection);
	connect(m_diskio, SIGNAL(seekFinished()), this, SLOT(seek_finished()), Qt::QueuedConnection);
	connect (m_diskio, SIGNAL(readSourceBufferUnderRun()), this, SLOT(handle_diskio_readbuffer_underrun()));
	connect (m_diskio, SIGNAL(writeSourceBufferOverRun()), this, SLOT(handle_diskio_writebuffer_overrun()));
	connect(this, SIGNAL(transferStarted()), m_diskio, SLOT(start_io()));
	connect(this, SIGNAL(transferStopped()), m_diskio, SLOT(stop_io()));

	mixdown = new audio_sample_t[audiodevice().get_buffer_size()];
	gainbuffer = new audio_sample_t[audiodevice().get_buffer_size()];
	m_masterOut = new AudioBus("Master Out", 2);
	m_hs = new QUndoStack(pm().get_undogroup());
	set_history_stack(m_hs);
	m_acmanager = new AudioClipManager(this);
	
	set_context_item( m_acmanager );

	m_playBackBus = audiodevice().get_playback_bus("Playback 1");

	transport = stopTransport = resumeTransport = false;
	snaplist = new SnapList(this);
	workSnap = new Snappable();
	workSnap->set_snap_list(snaplist);

	realtimepath = false;
	scheduleForDeletion = false;
	isSnapOn=true;
	changed = rendering = false;
	firstVisibleFrame=workingFrame=0;
	seeking = 0;
	// TODO seek to old position on project exit ?
	transportFrame = 0;
	
	m_pluginChain = new PluginChain(this, this);
	m_timeline = new TimeLine(this);
	
	m_audiodeviceClient = new Client("song_" + QByteArray::number(get_id()));
	m_audiodeviceClient->set_process_callback( MakeDelegate(this, &Song::process) );
}

int Song::set_state( const QDomNode & node )
{
	PENTER;
	QDomNode propertiesNode = node.firstChildElement("Properties");
	m_id = node.toElement().attribute( "id", "" ).toLongLong();

	QDomElement e = propertiesNode.toElement();

	title = e.attribute( "title", "" );
	artists = e.attribute( "artists", "" );
	set_gain(e.attribute( "mastergain", "1.0").toFloat() );
	set_hzoom(e.attribute( "hzoom", "" ).toInt());
	set_first_visible_frame(e.attribute( "firstVisibleFrame", "0" ).toUInt());
	set_work_at(e.attribute( "workingFrame", "0").toUInt());
	
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

QDomNode Song::get_state(QDomDocument doc)
{
	QDomElement songNode = doc.createElement("Song");
	songNode.setAttribute("id", m_id);
	QDomElement properties = doc.createElement("Properties");
	properties.setAttribute("title", title);
	properties.setAttribute("artists", artists);
	properties.setAttribute("firstVisibleFrame", firstVisibleFrame);
	properties.setAttribute("workingFrame", (uint)workingFrame);
	properties.setAttribute("hzoom", m_hzoom);
	properties.setAttribute("mastergain", m_gain);
	songNode.appendChild(properties);

	doc.appendChild(songNode);

	songNode.appendChild(m_timeline->get_state(doc));
	
	QDomNode tracksNode = doc.createElement("Tracks");

	foreach(Track* track, m_tracks) {
		tracksNode.appendChild(track->get_state(doc));
	}

	songNode.appendChild(tracksNode);

	QDomNode m_pluginChainNode = doc.createElement("PluginChain");
	m_pluginChainNode.appendChild(m_pluginChain->get_state(doc));
	songNode.appendChild(m_pluginChainNode);


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
	if (transport) {
		transport = false;
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

	if (transport) {
		stopTransport = true;
	}

	rendering = true;

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

	spec->total_frames = spec->end_frame - spec->start_frame;

// 	PWARN("Render length is: %s",frame_to_smpte(spec->total_frames, m_project->get_rate()).toAscii().data() );

	spec->pos = spec->start_frame;
	transportFrame = spec->start_frame;
	spec->progress = 0;

	QString idString = QString::number(m_id);
	if (m_id < 10) {
		idString.prepend("0");
	}
	spec->name =  "Song" + QString::number(m_project->get_song_index(m_id)) +"-" + title + spec->extension;

	if (spec->start_frame >= spec->end_frame) {
		PWARN("illegal frame range in export specification");
		return -1;
	}

	if (spec->channels == 0) {
		PWARN("Illegal channel count (0) in export specification");
		return -1;
	}

	m_exportSource = new WriteSource(spec);

	return 1;
}

int Song::finish_audio_export()
{
	m_exportSource->finish_export();
	delete m_exportSource;
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
		process (nframes);
		/*		PWARN("Finished Rendering for this song");
				PWARN("running is %d", spec->running);
				PWARN("stop is %d", spec->stop);
				PWARN("this_nframes is %d", this_nframes);*/
		return finish_audio_export();
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


	if (m_exportSource->process (nframes)) {
		goto out;
	}

	spec->pos += nframes;

	progress = (int) (( 100.0 * (float)(spec->pos) ) / spec->total_frames);
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
		rendering = false;
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

	m_gain = gain;

	emit masterGainChanged();
}

void Song::set_title(const QString& sTitle)
{
	title=sTitle;
	emit propertieChanged();
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
	// If there is no transport, start_seek() will _not_ be
	// called from within process(). So we do it now!
	if (!transport) {
		start_seek();
	}

	seeking = 1;
	emit transportPosSet();
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
	if (transport) {
		realtimepath = false;
		resumeTransport = true;
	}

	m_diskio->prepare_for_seek();

	if (!transport) {
		emit seekStart(newTransportFramePos);
	} else {
		transport = false;
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
		transport = true;
		realtimepath = true;
		resumeTransport = false;
	}

	PMESG2("Song :: leaving seek_finished");
}

Command* Song::toggle_snap()
{
	set_snapping( ! isSnapOn );
	return 0;
}


void Song::set_snapping(bool snapping)
{
	isSnapOn = snapping;
	emit snapChanged();
}

/******************************** SLOTS *****************************/

Track* Song::create_track()
{
	int height = Track::INITIAL_HEIGHT;

	Track* track = new Track(this, "Unnamed", height);

	return track;
}

Command* Song::go()
{
// 	printf("Song-%d::go transport is %d\n", m_id, transport);
	
	if (transport) {
		stopTransport = true;
	} else {
		emit transferStarted();
		
		if (any_track_armed()) {
			CommandGroup* group = new CommandGroup(this, tr("Recording to Clip(s)"));
			
			foreach(Track* track, m_tracks) {
				if (track->armed()) {
					group->add_command(track->init_recording());
				}
			}
			
			ie().process_command(group);
		}
		
		transport = true;
// 		printf("transport is %d\n", transport);
		realtimepath = true;
	}
	
	return 0;
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
// 	printf("Song-%d::process transport is %d\n", m_id, transport);
	if (!transport)
		return 0;

	if (stopTransport) {
		RT_THREAD_EMIT(this, 0, transferStopped());
		transport = false;
		realtimepath = false;
		stopTransport = false;

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

	if (seeking) {
		start_seek();
	}


	if (!processResult) {
		return 0;
	}

	// Mix the result into the AudioDevice "physical" buffers
	if (m_playBackBus) {
		Mixer::mix_buffers_with_gain(m_playBackBus->get_buffer(0, nframes), m_masterOut->get_buffer(0, nframes), nframes, m_gain);
		Mixer::mix_buffers_with_gain(m_playBackBus->get_buffer(1, nframes), m_masterOut->get_buffer(1, nframes), nframes, m_gain);
		
		// Process all the plugins for this Song
		QList<Plugin* >* pluginList = m_pluginChain->get_plugin_list();
		for (int i=0; i<pluginList->size(); ++i) {
			pluginList->at(i)->process(m_playBackBus, nframes);
		}
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

	Mixer::apply_gain_to_buffer(m_masterOut->get_buffer(0, nframes), nframes, m_gain);
	Mixer::apply_gain_to_buffer(m_masterOut->get_buffer(1, nframes), nframes, m_gain);

	// update the transportFrame
	transportFrame += nframes;

	return 1;
}

void Song::resize_buffer( )
{
	delete [] mixdown;
	delete [] gainbuffer;
	mixdown = new audio_sample_t[audiodevice().get_buffer_size()];
	gainbuffer = new audio_sample_t[audiodevice().get_buffer_size()];
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

float Song::get_gain() const
{
	return m_gain;
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
	if (transport) {
		printf("Song:: DiskIO ReadBuffer UnderRun signal received!\n");
		info().critical(tr("Hard Disk overload detected!"));
		info().critical(tr("Failed to fill ReadBuffer in time"));
	}
}

void Song::handle_diskio_writebuffer_overrun( )
{
	if (transport) {
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

Command * Song::playhead_to_workcursor( )
{
	set_transport_pos( workingFrame );

	return (Command*) 0;
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

// eof

