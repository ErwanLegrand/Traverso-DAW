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

$Id: Song.h,v 1.4 2006/05/02 19:20:16 r_sijrier Exp $
*/

#ifndef SONG_H
#define SONG_H

#include "ContextItem.h"
#include <QHash>
#include <QDomDocument>
#include "defines.h"

class Project;
class Track;
class AudioSource;
class WriteSource;
class Track;
class AudioClip;
class AudioPluginSelector;
class MtaRegionList;
class DiskIO;
class AudioClipManager;
class Client;
class AudioBus;

struct ExportSpecification;

class Song : public ContextItem
{
	Q_OBJECT

public:

	enum EditMode
	{
		EDIT_NORMAL,
		EDIT_TRACK_CURVES
	};


	Song(Project* project, int m_id);
	Song(Project* project, const QDomNode node);
	~Song();

	// Get functions
	int get_hzoom() const
	{
		return m_hzoom;
	}
	int get_clips_count_for_audio(AudioSource* a);
	int get_floorY();
	int get_playing_xpos()
	{
		return block_to_xpos(transport_frame);
	}
	int get_rate();
	int get_bitdepth();
	int get_numtracks() const
	{
		return m_tracks.size();
	}
	int get_id() const
	{
		return m_id;
	}
	int get_active_track_number() const
	{
		return activeTrackNumber;
	}
	nframes_t get_transfer_frame() const;
	nframes_t get_working_block() const
	{
		return workingFrame;
	}
	nframes_t get_firstblock() const;
	nframes_t get_last_block() const;
	
	Track*       get_track(int trackNumber);
	Track*       get_track_under_y (int y);
	QString get_title() const
	{
		return title;
	}
	QString get_artists() const
	{
		return artists;
	}
	QDomNode get_state(QDomDocument doc);
	QHash<int, Track* > get_tracks() const;
	DiskIO*	get_diskio();
	AudioClipManager* get_audioclip_manager();
	AudioBus* get_master_out() const {return masterOut;}

	// Set functions
	void set_artists(QString pArtistis);
	void set_active_track(int trackNumber);
	void update_cursor_pos();
	void set_first_block(nframes_t pos);
	void set_master_gain(float pMasterGain);
	void set_title(QString sTitle);
	void set_work_at(nframes_t pos);
	void set_hzoom(int hzoom);
	void set_number(int num)
	{
		m_id = num;
	}


	int block_to_xpos(nframes_t block);
	int delete_audio_source(AudioSource* pAudio);
	int delete_track(int trackNumber);
	int process_go(int step);
	int remove_all_clips_for_audio(AudioSource* a);
	int snapped_x(int x);
	int process(nframes_t nframes);
	int process_export(nframes_t nframes);
	int prepare_export(ExportSpecification* spec);
	int render(ExportSpecification* spec);
	void solo_track(Track* track);
	void create(int tracksToCreate);
	nframes_t xpos_to_block(int xpos);
	bool any_track_armed();
	bool realtime_path() const {return realtimepath;}
	bool is_transporting() const
	{
		return transport;
	}
	bool is_changed() const
	{
		return changed;
	}
	bool is_snap_on() const
	{
		return isSnapOn;
	}

	void disconnect_from_audiodevice_and_delete();
	
	audio_sample_t* 		mixdown;

private:
	QHash<int, Track* >	m_tracks;
	AudioPluginSelector* 	audioPluginSelector;
	MtaRegionList* 		regionList;
	Project*				m_project;
	WriteSource*			exportSource;
	AudioBus*			playBackBus;
	Client* 				audiodeviceClient;
	AudioBus*			masterOut;
	DiskIO*				diskio;
	AudioClipManager*		acmanager;

	nframes_t			transport_frame;
	nframes_t 			firstFrame;
	nframes_t 			origBlockL;
	nframes_t 			origBlockR;
	nframes_t 			workingFrame;

	float 				masterGain;

	QString 				artists;
	QString 				title;

	int 			activeTrackNumber;
	int 			m_id;
	int 			m_hzoom;


	bool 			rendering;
	bool 			changed;
	bool 			isSnapOn;
	volatile size_t	transport;
	bool			resumeTransport;
	bool 			stopTransport;
	bool			realtimepath;
	bool			scheduleForDeletion;

	void init();
	void connect_to_audiodevice();
	int set_state( const QDomNode & node );

	int finish_audio_export();
	void start_seek();

	void add_track(Track* track, int id);
	
	uint		 	newTransportFramePos;
	bool			seeking;

	friend class Track;
	
public slots :
	void seek_finished();
	void handle_diskio_outofsync();
	void audiodevice_client_request_processed();
	void audiodevice_started();
	void resize_buffer();

	Command* go();
	Command* create_track();
	Command* delete_track();

	Command* set_editing_mode();
	Command* set_curve_mode();
	Command* work_next_edge();
	Command* work_previous_edge();
	Command* in_crop();
//         Command* add_audio_plugin_controller();
//         Command* select_audio_plugin_controller();
//         Command* remove_current_audio_plugin_controller();
//         Command* remove_audio_plugin_controller();
//         Command* add_node();
//         Command* drag_and_drop_node();
//         Command* audio_plugin_setup();
//         Command* node_setup();
	Command* create_region_start();
	Command* create_region_end();
	Command* create_region();
	Command* delete_region_under_x();
	Command* go_regions();
	Command* go_loop_regions();
	Command* jog_create_region();
	Command* undo();
	Command* redo();
	Command* toggle_snap();

signals:
	void trackCreated(Track* );
	void cursorPosChanged();
	void hzoomChanged();
	void transferStarted();
	void transferStopped();
	void workingPosChanged(int newPos);
	void firstBlockChanged();
	void lastFramePositionChanged();
	void seekStart(uint position);
	void snapChanged();


};

inline nframes_t Song::get_transfer_frame() const
{
	return transport_frame;
}

#endif




//eof
