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

$Id: Song.cpp,v 1.46 2007/01/10 11:32:11 r_sijrier Exp $
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
#include "MtaRegion.h"
#include "MtaRegionList.h"
#include "Peak.h"
#include "Export.h"
#include "DiskIO.h"
#include "WriteSource.h"
#include "AudioClipManager.h"
#include "Tsar.h"
#include "SnapList.h"
#include "Config.h"

#include "ContextItem.h"

#include <Plugin.h>
#include <PluginChain.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


Song::Song(Project* project, int number)
		: ContextItem(), m_project(project), m_id(number)
{
	PENTERCONS;
	title="Untitled";
	m_gain = 1.0f;
	artists = "No artists name yet";
	int level = config().get_int_property("hzoomLevel");
	switch (level) {
		case 8: m_hzoom = 4;
			break;
		case 64: m_hzoom = 7;
			break;
		case 128: m_hzoom = 8;
			break;
		case 256: m_hzoom = 9;
			break;
		case 1024: m_hzoom = 10;
			break;
		case 2048: m_hzoom = 11;
			break;
		case 4096: m_hzoom = 12;
			break;
		case 8192: m_hzoom = 13;
			break;
		case 16384: m_hzoom = 14;
			break;
		default:   m_hzoom = 11;
			break;
	}
	int tracksToCreate = config().get_int_property("trackCreationCount");
	regionList = (MtaRegionList*) 0;

	init();

	activeTrackNumber = 1;
	for (int i=1; i <= tracksToCreate; i++) {
		Track* track = create_track();
		private_add_track(track);
	}

	connect_to_audiodevice();
}

Song::Song(Project* project, const QDomNode node)
		: ContextItem(), m_project(project)
{
	PENTERCONS;
	init();
	set_state( node );

	connect_to_audiodevice();
}

Song::~Song()
{
	PENTERDES;

	delete [] mixdown;
	delete [] gainbuffer;

	delete diskio;
	delete regionList;
	delete masterOut;
	delete m_hs;
	delete audiodeviceClient;
	delete snaplist;
}

void Song::init()
{
	PENTER2;
	threadId = QThread::currentThreadId ();

	diskio = new DiskIO(this);
	
	connect(this, SIGNAL(seekStart(uint)), diskio, SLOT(seek(uint)), Qt::QueuedConnection);
	connect(&audiodevice(), SIGNAL(clientRemoved(Client*)), this, SLOT (audiodevice_client_removed(Client*)));
	connect(&audiodevice(), SIGNAL(started()), this, SLOT(audiodevice_started()));
	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(resize_buffer()), Qt::DirectConnection);
	connect(diskio, SIGNAL(seekFinished()), this, SLOT(seek_finished()), Qt::QueuedConnection);
	connect (diskio, SIGNAL(readSourceBufferUnderRun()), this, SLOT(handle_diskio_readbuffer_underrun()));
	connect (diskio, SIGNAL(writeSourceBufferOverRun()), this, SLOT(handle_diskio_writebuffer_overrun()));
	connect(this, SIGNAL(transferStarted()), diskio, SLOT(start_io()));
	connect(this, SIGNAL(transferStopped()), diskio, SLOT(stop_io()));

	mixdown = new audio_sample_t[audiodevice().get_buffer_size()];
	gainbuffer = new audio_sample_t[audiodevice().get_buffer_size()];
	masterOut = new AudioBus("Master Out", 2);
	regionList = new MtaRegionList();
	m_hs = new QUndoStack(pm().get_undogroup());
	set_history_stack(m_hs);
	acmanager = new AudioClipManager(this);
	
	set_context_item( acmanager );

	playBackBus = audiodevice().get_playback_bus("Playback 1");

	transport = stopTransport = resumeTransport = false;
	snaplist = new SnapList(this);
	realtimepath = false;
	scheduleForDeletion = false;
	isSnapOn=true;
	changed = rendering = false;
	firstVisibleFrame=workingFrame=activeTrackNumber=0;
	trackCount = 0;
	seeking = 0;
	
	pluginChain = new PluginChain(this, this);
}

int Song::set_state( const QDomNode & node )
{
	PENTER;
	QDomNode propertiesNode = node.firstChildElement("Properties");
	m_id = node.toElement().attribute( "id", "" ).toInt();

	QDomElement e = propertiesNode.toElement();

	title = e.attribute( "title", "" );
	artists = e.attribute( "artists", "" );
	activeTrackNumber = e.attribute( "activeTrackNumber", "" ).toInt();
	set_gain(e.attribute( "mastergain", "1.0").toFloat() );
	set_hzoom(e.attribute( "hzoom", "" ).toInt());
	set_first_visible_frame(e.attribute( "firstVisibleFrame", "0" ).toUInt());
	set_work_at(e.attribute( "workingFrame", "0").toUInt());
	
	QDomNode tracksNode = node.firstChildElement("Tracks");
	QDomNode trackNode = tracksNode.firstChild();

	while(!trackNode.isNull()) {
		Track* track = new Track(this, trackNode);
		track->set_id(++trackCount);
		private_add_track(track);
		track->set_state(trackNode);

		trackNode = trackNode.nextSibling();
	}

	QDomNode pluginChainNode = node.firstChildElement("PluginChain");
	pluginChain->set_state(pluginChainNode);
	
	return 1;
}

QDomNode Song::get_state(QDomDocument doc)
{
	QDomElement songNode = doc.createElement("Song");
	songNode.setAttribute("id", m_id);
	QDomElement properties = doc.createElement("Properties");
	properties.setAttribute("title", title);
	properties.setAttribute("artists", artists);
	properties.setAttribute("activeTrackNumber", activeTrackNumber);
	properties.setAttribute("firstVisibleFrame", firstVisibleFrame);
	properties.setAttribute("workingFrame", (uint)workingFrame);
	properties.setAttribute("hzoom", m_hzoom);
	properties.setAttribute("mastergain", m_gain);
	songNode.appendChild(properties);

	doc.appendChild(songNode);

	QDomNode tracksNode = doc.createElement("Tracks");

	foreach(Track* track, m_tracks) {
		tracksNode.appendChild(track->get_state(doc));
	}

	songNode.appendChild(tracksNode);

	QDomNode pluginChainNode = doc.createElement("PluginChain");
	pluginChainNode.appendChild(pluginChain->get_state(doc));
	songNode.appendChild(pluginChainNode);


	return songNode;
}

void Song::connect_to_audiodevice( )
{
	PENTER;
	audiodeviceClient = new Client("song_" + QByteArray::number(get_id()));
	audiodeviceClient->set_process_callback( MakeDelegate(this, &Song::process) );
	audiodevice().add_client(audiodeviceClient);
}

void Song::disconnect_from_audiodevice_and_delete()
{
	PENTER;
	if (transport) {
		transport = false;
	}
	scheduleForDeletion = true;
	pm().scheduled_for_deletion(this);
	audiodevice().remove_client(audiodeviceClient);
}

void Song::audiodevice_client_removed(Client* client )
{
	PENTER;
	if (audiodeviceClient == client) {
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

	return new AddRemoveItemCommand(this, track, historable, this,
					"private_add_track(Track*)", "trackAdded(Track*)",
					"private_remove_track(Track*)", "trackRemoved(Track*)", tr("Add Track"));
}


Command* Song::remove_track(Track* track, bool historable)
{
	return new AddRemoveItemCommand(this, track, historable, this,
					"private_remove_track(Track*)", "trackRemoved(Track*)",
					"private_add_track(Track*)", "trackAdded(Track*)", tr("Remove Track"));
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

		if (startframe < spec->start_frame) {
			spec->start_frame = startframe;
		}

	}

	spec->total_frames = spec->end_frame - spec->start_frame;

// 	PWARN("Render length is: %s",frame_to_smpte(spec->total_frames, m_project->get_rate()).toAscii().data() );

	spec->pos = spec->start_frame;
	transportFrame = spec->start_frame;
	spec->progress = 0;

	QString idString = QString::number(m_id);
	if (m_id < 10)
		idString.prepend("0");
	spec->name =  idString +" - " + title + spec->extension;

	if (spec->start_frame >= spec->end_frame) {
		PWARN("illegal frame range in export specification");
		return -1;
	}

	if (spec->channels == 0) {
		PWARN("Illegal channel count (0) in export specification");
		return -1;
	}

	exportSource = new WriteSource(spec);

	return 1;
}

int Song::finish_audio_export()
{
	exportSource->finish_export();
	delete exportSource;
	return 0;
}

int Song::render(ExportSpecification* spec)
{
	uint32_t chn;
	uint32_t x;
	int ret = -1;
	int progress;
	nframes_t this_nframes;
	nframes_t nframes = spec->blocksize;

	if (!spec->running || spec->stop || (this_nframes = std::min ((spec->end_frame - spec->pos), nframes)) == 0) {
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
		buf = masterOut->get_buffer(chn, nframes);

		if (!buf) {
			// Seem we are exporting at least to Stereo from an AudioBus with only one channel...
			// Use the first channel..
			buf = masterOut->get_buffer(0, nframes);
		}

		for (x = 0; x < nframes; ++x) {
			spec->dataF[chn+(x*spec->channels)] = buf[x];
		}
	}


	if (exportSource->process (nframes)) {
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


SnapList* Song::get_snap_list()
{
	return snaplist;
}

Track* Song::get_track(int trackNumber)
{
	return m_tracks.value(trackNumber);
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

void Song::set_active_track(int trackNumber)
{
	if (!m_tracks.value(trackNumber))
		return;

	m_tracks.value(activeTrackNumber)->deactivate();
	activeTrackNumber=trackNumber;
	m_tracks.value(activeTrackNumber)->activate();
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

/** use this part if the work cursor should _not_ snap **/
// 	newTransportFramePos = pos;
// 	workingFrame = pos;
/** use this if it should snap **/
	long position = snaplist->get_snap_value(pos);
	workingFrame = position;
/** **/

	printf("position is %d\n", position);
	Q_ASSERT(position >= 0);

	newTransportFramePos = (uint) position;

	// If there is no transport, start_seek() will _not_ be
	// called from within process(). So we do it now!
	if (!transport) {
		start_seek();
	}

	seeking = 1;
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
}



//
//  Function _could_ be called in RealTime AudioThread processing path
//
void Song::start_seek()
{
	PMESG2("Song :: entering start_seek");
	PMESG2("Song :: thread id is: %ld", QThread::currentThreadId ());
	PMESG2("Song::start_seek()");
	if (transport) {
		transport = false;
		realtimepath = false;
		resumeTransport = true;
	}

	diskio->prepare_for_seek();

	if (!transport) {
		emit seekStart(newTransportFramePos);
	} else {
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

	emit workingPosChanged();
	PMESG2("Song :: leaving seek_finished");
}

Command* Song::toggle_snap()
{
	isSnapOn=!isSnapOn;

	emit propertieChanged();

	return (Command*) 0;
}

/******************************** SLOTS *****************************/

Track* Song::create_track()
{
	int trackBaseY = 0;
	int height = Track::INITIAL_HEIGHT;

	foreach(Track* track, m_tracks) {
		trackBaseY += track->get_height();
		height = track->get_height();
	}

	Track* track = new Track(this, ++trackCount, "Unnamed", trackBaseY, height);

	return track;
}

Command* Song::add_new_track()
{
	Track* track = create_track();

	return add_track(track);
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

Command* Song::work_next_edge()
{
	nframes_t w = acmanager->get_last_frame();

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

Command* Song::undo()
{
	m_hs->undo();
	return (Command*) 0;
}

Command* Song::redo()
{
	m_hs->redo();
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

	// zero the masterOut buffers
	masterOut->silence_buffers(nframes);

	int processResult = 0;

	// Process all Tracks.
	foreach(Track* track, m_tracks) {
		processResult |= track->process(nframes);
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
	if (playBackBus) {
		Mixer::mix_buffers_with_gain(playBackBus->get_buffer(0, nframes), masterOut->get_buffer(0, nframes), nframes, m_gain);
		Mixer::mix_buffers_with_gain(playBackBus->get_buffer(1, nframes), masterOut->get_buffer(1, nframes), nframes, m_gain);
	}

	// Process all the plugins for this Song (Currently no one)
	foreach(Plugin* plugin, pluginChain->get_plugin_list()) {
		plugin->process(playBackBus, nframes);
	}
	
	return 1;
}

int Song::process_export( nframes_t nframes )
{
	// Get the masterout buffers, and fill with zero's
	masterOut->silence_buffers(nframes);
	memset (mixdown, 0, sizeof (audio_sample_t) * nframes);

	// Process all Tracks.
	foreach(Track* track, m_tracks) {
		track->process(nframes);
	}

	Mixer::apply_gain_to_buffer(masterOut->get_buffer(0, nframes), nframes, m_gain);
	Mixer::apply_gain_to_buffer(masterOut->get_buffer(1, nframes), nframes, m_gain);

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

void Song::update_cursor_pos()
{
	if (!transport)
		emit cursorPosChanged();
}

QHash< int, Track * > Song::get_tracks( ) const
{
	return m_tracks;
}

DiskIO * Song::get_diskio( )
{
	return diskio;
}

AudioClipManager * Song::get_audioclip_manager( )
{
	return acmanager;
}

PluginChain* Song::get_plugin_chain()
{
	return pluginChain;
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
	playBackBus = audiodevice().get_playback_bus("Playback 1");
}

nframes_t Song::get_last_frame( ) const
{
	return acmanager->get_last_frame();
}

Command * Song::playhead_to_workcursor( )
{
	set_work_at( workingFrame );

	return (Command*) 0;
}

Command * Song::master_gain( )
{
	return new Gain(this, tr("Master Gain"));
}

void Song::private_add_track(Track* track)
{
	m_tracks.insert(track->get_id(), track);
}

void Song::private_remove_track(Track* track)
{
	if (m_tracks.take(track->get_id() )) {
// 		printf("Removing Track with id %d\n", track->get_id());
	}
}


// eof
