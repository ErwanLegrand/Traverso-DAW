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

#ifndef SONG_H
#define SONG_H

#include "ContextItem.h"
#include <QDomNode>
#include <QTimer>
#include "defines.h"
#include "APILinkedList.h"

class Project;
class AudioTrack;
class AudioSource;
class WriteSource;
class AudioTrack;
class AudioClip;
class DiskIO;
class AudioClipManager;
class AudioDeviceClient;
class AudioBus;
class SnapList;
class TimeLine;
class Snappable;
class DecodeBuffer;
class SubGroup;
class Track;

struct ExportSpecification;

class Sheet : public ContextItem, public APILinkedListNode
{
	Q_OBJECT
	Q_CLASSINFO("start_transport", tr("Play"))
	Q_CLASSINFO("set_recordable_and_start_transport", tr("Record"));
	Q_CLASSINFO("work_next_edge", tr("Workcursor: To next edge"))
	Q_CLASSINFO("work_previous_edge", tr("Workcursor: To previous edge"))
	Q_CLASSINFO("undo", tr("Undo"))
	Q_CLASSINFO("redo", tr("Redo"))
	Q_CLASSINFO("toggle_snap", tr("Snap: On/Off"))
	Q_CLASSINFO("toggle_solo", tr("Solo: On/Off"))
	Q_CLASSINFO("toggle_mute", tr("Mute: On/Off"))
	Q_CLASSINFO("toggle_arm", tr("Arm: On/Off"))
        Q_CLASSINFO("toggle_effects_mode", tr("Toggle Curve Mode: On/Off"))
	Q_CLASSINFO("prev_skip_pos", tr("To previous snap position"))
	Q_CLASSINFO("next_skip_pos", tr("To next snap position"))

public:

        Sheet(Project* project, int numtracks=0);
	Sheet(Project* project, const QDomNode node);
	~Sheet();
	
	enum Mode {
		EDIT = 1,
  		EFFECTS = 2
	};

	// Get functions
	qreal get_hzoom() const {return m_hzoom;}
	int get_rate();
	int get_bitdepth();
        int get_numtracks() const {return m_audioTracks.size();}
	int get_mode() const {return m_mode;}
	int is_transport_rolling() const {return m_transport;}
	void get_scrollbar_xy(int& x, int& y) {x = m_sbx; y = m_sby;}
	
	const TimeRef& get_work_location() const {return m_workLocation;}
	nframes_t get_first_visible_frame() const;
	TimeRef get_last_location() const;
	const TimeRef& get_transport_location() const {return m_transportLocation;}
	
	const TimeRef& get_new_transport_location() const {return m_newTransportLocation;}
	
        QString get_artists() const {return m_artists;}
        QString get_name() const {return m_name;}
	QDomNode get_state(QDomDocument doc, bool istemplate=false);
        QList<AudioTrack*> get_audio_tracks() const;
        QList<SubGroup*> get_subgroups() const;
        QList<Track*> get_tracks() const;
	
        Project* get_project() const {return m_project;}
	DiskIO*	get_diskio() const;
	AudioClipManager* get_audioclip_manager() const;
	AudioBus* get_render_bus() const {return m_renderBus;}
	AudioBus* get_clip_render_bus() const {return m_clipRenderBus;}
        SubGroup* get_master_out() const {return m_masterOut;}
	SnapList* get_snap_list() const;
	TimeLine* get_timeline() const {return m_timeline;}
        Snappable* get_work_snap() {return m_workSnap;}
        Track* get_track(qint64 id);
        AudioTrack* get_audio_track_for_index(int index);
        SubGroup* get_subgroup(const QString& name);
        QString get_audio_sources_dir() const;

	// Set functions
	void set_artists(const QString& pArtistis);
        void set_name(const QString& name);
	void set_first_visible_frame(nframes_t pos);
        void set_work_at(TimeRef location, bool isFolder=false);
        void set_work_at_for_sheet_as_track_folder(const TimeRef& location);
	void set_hzoom(qreal hzoom);
	void set_snapping(bool snap);
	void set_scrollbar_xy(int x, int y) {m_sbx = x; m_sby = y;}
	int set_state( const QDomNode & node );
	void set_recording(bool recording, bool realtime);
        void set_audio_sources_dir(const QString& dir);

	void skip_to_start();
	void skip_to_end();
	

	int process(nframes_t nframes);
	// jackd only feature
	int transport_control(transport_state_t state);
	int process_export(nframes_t nframes);
	int prepare_export(ExportSpecification* spec);
	int render(ExportSpecification* spec);
        int start_export(ExportSpecification* spec);

        void solo_track(Track* track);
	void create(int tracksToCreate);
        Command* add_track(Track* api, bool historable=true);
        Command* remove_track(Track* api, bool historable=true);
	
        bool any_audio_track_armed();
	bool realtime_path() const {return m_realtimepath;}
        bool is_changed() const {return m_changed;}
	bool is_snap_on() const	{return m_isSnapOn;}
        bool is_recording() const {return m_recording;}
	bool is_smaller_then(APILinkedListNode* node) {Q_UNUSED(node); return false;}

	void disconnect_from_audiodevice();
	void connect_to_audiodevice();
        void schedule_for_deletion();

	audio_sample_t* 	mixdown;
	audio_sample_t*		readbuffer;
	audio_sample_t*		gainbuffer;
	DecodeBuffer*		renderDecodeBuffer;

#if defined (THREAD_CHECK)
	unsigned long	threadId;
#endif

private:
        APILinkedList		m_audioTracks;
        APILinkedList           m_subGroups;
        SubGroup*               m_masterOut;
        QList<AudioClip*>	m_recordingClips;
	QTimer			m_skipTimer;
	Project*		m_project;
	WriteSource*		m_exportSource;
	AudioDeviceClient* 		m_audiodeviceClient;
	AudioBus*		m_renderBus;
	AudioBus*		m_clipRenderBus;
	DiskIO*			m_diskio;
	AudioClipManager*	m_acmanager;
	TimeLine*		m_timeline;
	QList<TimeRef>		m_xposList;
        QString                 m_audioSourcesDir;

	// The following data could be read/written by multiple threads
	// (gui, audio and m_diskio thread). Therefore they should have 
	// atomic behaviour, still not sure if volatile size_t declaration
	// would suffice, or should we use t_atomic_int_set/get() to make
	// it 100% portable and working on all platforms...?
	volatile size_t		m_transportFrame;
	volatile size_t		m_newTransportFramePos;
	volatile size_t		m_transport;
	volatile size_t		m_seeking;
	volatile size_t		m_startSeek;
	
	TimeRef		m_transportLocation;
	TimeRef		m_workLocation;
	TimeRef		m_newTransportLocation;

	
        nframes_t 	m_firstVisibleFrame;
        QString 	m_artists;
        QString         m_name;
	int		m_mode;
	qreal		m_hzoom;
	int		m_sbx;
	int		m_sby;
	uint		m_currentSampleRate;
	bool 		m_rendering;
        bool 		m_changed;
	bool 		m_isSnapOn;
	bool		m_resumeTransport;
	bool 		m_stopTransport;
	bool		m_realtimepath;
        bool		m_scheduledForDeletion;
        bool		m_recording;
	bool		m_prepareRecording;
	bool		m_readyToRecord;
        SnapList*	m_snaplist;
        Snappable*	m_workSnap;
	
	void init();

	int finish_audio_export();
	void start_seek();
        void initiate_seek_start(TimeRef location);
	void start_transport_rolling(bool realtime);
	void stop_transport_rolling();
	void update_skip_positions();
	
        void resize_buffer(nframes_t size);

        AudioTrack* create_audio_track();
	
	friend class AudioClipManager;
	friend class TimeLine;

public slots :
	void seek_finished();
	void audiodevice_client_removed(AudioDeviceClient* );
	void audiodevice_params_changed();
        void set_gain(float gain);
	void set_transport_pos(TimeRef location);
	void set_temp_follow_state(bool state);


	Command* next_skip_pos();
	Command* prev_skip_pos();
	Command* start_transport();
	Command* set_recordable();
	Command* set_recordable_and_start_transport();
	Command* toggle_snap();
	Command* toggle_solo();
	Command* toggle_mute();
	Command* toggle_arm();
	Command* set_editing_mode();
	Command* set_effects_mode();
        Command* toggle_effects_mode();

signals:
        void trackRemoved(Track* );
        void trackAdded(Track* );
	void hzoomChanged();
	void transportStarted();
	void transportStopped();
	void workingPosChanged();
	void transportPosSet();
	void firstVisibleFrameChanged();
	void lastFramePositionChanged();
	void seekStart();
	void snapChanged();
	void tempFollowChanged(bool state);
	void setCursorAtEdge();
	void modeChanged();
	void recordingStateChanged();
	void prepareRecording();
        void stateChanged();
        void propertyChanged();
	
private slots:
        void private_add_track(Track* track);
        void private_remove_track(Track* track);
	void handle_diskio_writebuffer_overrun();
	void handle_diskio_readbuffer_underrun();
	void prepare_recording();
	void clip_finished_recording(AudioClip* clip);
	void config_changed();
};

#endif

//eof
