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

$Id: Song.h,v 1.20 2006/11/09 15:45:42 r_sijrier Exp $
*/

#ifndef SONG_H
#define SONG_H

#include "ContextItem.h"
#include <QHash>
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
class PluginChain;
class SnapList;

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
	SnapList* get_snap_list();

	// Set functions
	void set_artists(const QString& pArtistis);
	void set_active_track(int trackNumber);
	void update_cursor_pos();
	void set_first_visible_frame(nframes_t pos);
	void set_title(const QString& sTitle);
	void set_work_at(nframes_t pos);
	void set_transport_pos(nframes_t pos);
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
	Command* add_track(Track* track, bool historable=true);
	Command* remove_track(Track* track, bool historable=true);
	
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
	
	unsigned long	threadId;

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
	PluginChain*		pluginChain;

	// The following data could be read/written by multiple threads
	// (gui, audio and diskio thread). Therefore they should have 
	// atomic behaviour, still not sure if volatile size_t declaration
	// would suffice, or should we use g_atomic_int_set/get() to make
	// it 100% portable and working on all platforms...?
	volatile size_t		transportFrame;
	volatile size_t		workingFrame;
	volatile size_t		newTransportFramePos;
	volatile size_t		transport;
	volatile size_t		seeking;

	
	nframes_t 		firstVisibleFrame;
	
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
	SnapList*		snaplist;

	void init();
	void connect_to_audiodevice();
	int set_state( const QDomNode & node );

	int finish_audio_export();
	void start_seek();

	Track* create_track();
	
	friend class AudioClipManager;

public slots :
	void seek_finished();
	void audiodevice_client_removed(Client* );
	void audiodevice_started();
	void resize_buffer();
	void set_gain(float gain);
	
	float get_gain() const;

	Command* go();
	Command* add_new_track();

	Command* set_editing_mode();
	Command* set_curve_mode();
	Command* work_next_edge();
	Command* work_previous_edge();
	Command* undo();
	Command* redo();
	Command* toggle_snap();
	Command* playhead_to_workcursor();
	Command* master_gain();
	Command* toggle_solo();
	Command* toggle_mute();

signals:
	void trackRemoved(Track* );
	void trackAdded(Track* );
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

private slots:
	void private_add_track(Track* track);
	void private_remove_track(Track* track);
	void handle_diskio_writebuffer_overrun();
	void handle_diskio_readbuffer_underrun();

};

inline nframes_t Song::get_transport_frame() const
{
	return transportFrame;
}

#endif




//eof
