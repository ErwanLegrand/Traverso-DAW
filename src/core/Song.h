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

$Id: Song.h,v 1.12 2006/06/29 22:43:24 r_sijrier Exp $
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
		return frame_to_xpos(transportFrame);
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
	nframes_t get_transport_frame() const;
	nframes_t get_working_frame() const
	{
		return workingFrame;
	}
	nframes_t get_first_visible_frame() const;
	nframes_t get_last_frame() const;
	
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
	void set_first_visible_frame(nframes_t pos);
	void set_title(QString sTitle);
	void set_work_at(nframes_t pos);
	void set_hzoom(int hzoom);
	void set_number(int num)
	{
		m_id = num;
	}


	int frame_to_xpos(nframes_t frame);
	int delete_audio_source(AudioSource* pAudio);
	int process_go(int step);
	int remove_all_clips_for_audio(AudioSource* a);
	int snapped_x(int x);
	int process(nframes_t nframes);
	int process_export(nframes_t nframes);
	int prepare_export(ExportSpecification* spec);
	int render(ExportSpecification* spec);
	void solo_track(Track* track);
	void create(int tracksToCreate);
	nframes_t xpos_to_frame(int xpos);
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
	
	audio_sample_t* 	mixdown;
	audio_sample_t*		gainbuffer;

private:
	QHash<int, Track* >	m_tracks;
	AudioPluginSelector* 	audioPluginSelector;
	MtaRegionList* 		regionList;
	Project*		m_project;
	WriteSource*		exportSource;
	AudioBus*		playBackBus;
	Client* 		audiodeviceClient;
	AudioBus*		masterOut;
	DiskIO*			diskio;
	AudioClipManager*	acmanager;

	nframes_t		transportFrame;
	nframes_t 		firstVisibleFrame;
	nframes_t 		workingFrame;
	uint		 	newTransportFramePos;
	volatile size_t		transport;

	float 			m_gain;
	
	QString 		artists;
	QString 		title;

	int 			activeTrackNumber;
	int 			m_id;
	int 			m_hzoom;
	int			trackCount;

	bool 			rendering;
	bool 			changed;
	bool 			isSnapOn;
	bool			resumeTransport;
	bool 			stopTransport;
	bool			realtimepath;
	bool			scheduleForDeletion;
	bool			seeking;

	void init();
	void connect_to_audiodevice();
	int set_state( const QDomNode & node );

	int finish_audio_export();
	void start_seek();

	Track* create_track();
	
	void add_track(Track* track);
	void remove_track(Track* track);
	
	friend class Track;
	

public slots :
	void seek_finished();
	void handle_diskio_outofsync();
	void audiodevice_client_removed();
	void audiodevice_started();
	void resize_buffer();
	void set_gain(float gain);
	
	float get_gain() const;

	Command* go();
	Command* add_new_track();
	Command* remove_track();

	Command* set_editing_mode();
	Command* set_curve_mode();
	Command* work_next_edge();
	Command* work_previous_edge();
// 	Command* in_crop();
//         Command* add_audio_plugin_controller();
//         Command* select_audio_plugin_controller();
//         Command* remove_current_audio_plugin_controller();
//         Command* remove_audio_plugin_controller();
//         Command* add_node();
//         Command* drag_and_drop_node();
//         Command* audio_plugin_setup();
//         Command* node_setup();
/*	Command* create_region_start();
	Command* create_region_end();
	Command* create_region();
	Command* delete_region_under_x();
	Command* go_regions();
	Command* go_loop_regions();
	Command* jog_create_region();*/
	Command* undo();
	Command* redo();
	Command* toggle_snap();
	Command* playhead_to_workcursor();
	Command* master_gain();

signals:
	void trackCreated(Track* );
	void cursorPosChanged();
	void hzoomChanged();
	void transferStarted();
	void transferStopped();
	void workingPosChanged();
	void firstVisibleFrameChanged();
	void lastFramePositionChanged();
	void seekStart(uint position);
	void snapChanged();
	void propertieChanged();
	void setCursorAtEdge();
	void masterGainChanged();
	void add_track_Signal();
	void remove_track_Signal();

private slots:
	void thread_save_add_track(QObject* obj);
	void thread_save_remove_track(QObject* obj);

};

inline nframes_t Song::get_transport_frame() const
{
	return transportFrame;
}

#endif




//eof
