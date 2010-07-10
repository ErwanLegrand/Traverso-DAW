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

#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include <QString>
#include <QList>
#include <QDomNode>

#include "ContextItem.h"
#include "ProcessingData.h"
#include "Snappable.h"
#include "defines.h"
#include "GainEnvelope.h"


class Sheet;
class ReadSource;
class WriteSource;
class AudioTrack;
class Peak;
class AudioBus;
class FadeCurve;
class PluginChain;

class AudioClip : public ProcessingData, public Snappable
{
	Q_OBJECT
	Q_CLASSINFO("mute", tr("Mute"))
	Q_CLASSINFO("reset_fade_in", tr("In: Remove"))
	Q_CLASSINFO("reset_fade_out", tr("Out: Remove"))
	Q_CLASSINFO("reset_fade_both", tr("Both: Remove"))
	Q_CLASSINFO("normalize", tr("Normalize"))
	Q_CLASSINFO("lock", tr("Lock"))

public:

	AudioClip(const QString& name);
	AudioClip(const QDomNode& node);
	~AudioClip();

	enum RecordingStatus {
		NO_RECORDING,
		RECORDING,
  		FINISHING_RECORDING
	};
	
	void set_audio_source(ReadSource* source);
        int init_recording();
	int process(nframes_t nframes);
	
	void set_track_start_location(const TimeRef& location);
	void set_fade_in(double range);
	void set_fade_out(double range);
	void set_track(AudioTrack* track);
	void set_sheet(Sheet* sheet);

	void set_selected(bool selected);
	void set_as_moving(bool moving);
	int set_state( const QDomNode& node );

	AudioClip* create_copy();
	AudioTrack* get_track() const;
	Sheet* get_sheet() const;
	Peak* get_peak() const {return m_peak;}
	QDomNode get_state(QDomDocument doc);
	FadeCurve* get_fade_in() const;
	FadeCurve* get_fade_out() const;
	
	TimeRef get_source_length() const;
	TimeRef get_length() const {return m_length;}
	TimeRef get_track_start_location() const {return m_trackStartLocation;}
	TimeRef get_track_end_location() const {return m_trackEndLocation;}
	TimeRef get_source_start_location() const {return m_sourceStartLocation;}
	TimeRef get_source_end_location() const {return m_sourceEndLocation;}
	
	int get_channels() const;
	int get_rate() const;
	int get_bitdepth() const;
	qint64 get_readsource_id() const;
	qint64 get_sheet_id() const {return m_sheetId;}
	ReadSource* get_readsource() const;
	
	QDomNode get_dom_node() const;
	
	bool is_take() const;
	bool is_selected();
	bool is_locked() const {return m_isLocked;}
	bool has_sheet() const;
	bool is_readsource_invalid() const {return !m_isReadSourceValid;}
	bool is_smaller_then(APILinkedListNode* node) {return ((AudioClip*)node)->get_track_start_location() > get_track_start_location();}
	bool is_moving() const {return m_isMoving;}

	int recording_state() const;

        float calculate_normalization_factor(float targetdB = 0.0);

        void removed_from_track();


private:
        Sheet*                  m_sheet;
        AudioTrack* 		m_track;
	ReadSource*		m_readSource;
        WriteSource*		m_writer;
	APILinkedList		m_fades;
	Peak* 			m_peak;
	FadeCurve*		fadeIn;
	FadeCurve*		fadeOut;
	QDomNode		m_domNode;
	
	TimeRef 		m_trackStartLocation;
	TimeRef 		m_trackEndLocation;
	TimeRef 		m_sourceEndLocation;
	TimeRef 		m_sourceStartLocation;
	TimeRef			m_sourceLength;
	TimeRef 		m_length;

	bool 			m_isTake;
	bool			m_isLocked;
	bool			m_isReadSourceValid;
	bool			m_isMoving;
        bool                    m_syncDuringDrag;
	RecordingStatus		m_recordingStatus;
	
	qint64			m_readSourceId;
	qint64			m_sheetId;

	void create_fade_in();
	void create_fade_out();
	void init();
	void set_source_end_location(const TimeRef& location);
	void set_source_start_location(const TimeRef& location);
	void set_track_end_location(const TimeRef& location);
	void set_sources_active_state();
	void process_capture(nframes_t nframes);
		
	friend class ResourcesManager;

signals:
	void muteChanged();
	void lockChanged();
	void positionChanged();
	void fadeAdded(FadeCurve*);
	void fadeRemoved(FadeCurve*);
	void recordingFinished(AudioClip*);

public slots:
	void finish_recording();
	void finish_write_source();
	void set_left_edge(TimeRef newLeftLocation);
	void set_right_edge(TimeRef newRightLocation);
	void track_audible_state_changed();
	void toggle_mute();
	void toggle_lock();
	void set_gain(float g);
	
	Command* mute();
	Command* reset_fade_in();
	Command* reset_fade_out();
	Command* reset_fade_both();
        Command* normalize();
	Command* lock();

private slots:
	void private_add_fade(FadeCurve* fade);
	void private_remove_fade(FadeCurve* fade);
        void update_global_configuration();
};


inline qint64 AudioClip::get_readsource_id( ) const {return m_readSourceId;}

#endif
