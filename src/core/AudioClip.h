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

*/

#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include <QString>
#include <QList>
#include <QDomNode>

#include "ContextItem.h"
#include "AudioProcessingItem.h"
#include "SnapList.h"
#include "Snappable.h"
#include "defines.h"
#include "GainEnvelope.h"


class Sheet;
class ReadSource;
class WriteSource;
class Track;
class Peak;
class AudioBus;
class FadeCurve;
class PluginChain;

class AudioClip : public ContextItem, public AudioProcessingItem, public Snappable
{
	Q_OBJECT
	Q_CLASSINFO("mute", tr("Mute"))
	Q_CLASSINFO("reset_fade_in", tr("In: Remove"))
	Q_CLASSINFO("reset_fade_out", tr("Out: Remove"))
	Q_CLASSINFO("reset_fade_both", tr("Both: Remove"))
	Q_CLASSINFO("normalize", tr("Normalize"))
	Q_CLASSINFO("denormalize", tr("Normalize: Reset"))
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
	int init_recording(QByteArray bus);
	int process(nframes_t nframes);
	
	void set_track_start_location(const TimeRef& location);
	void set_name(const QString& name);
	void set_fade_in(double range);
	void set_fade_out(double range);
	void set_track(Track* track);
	void set_sheet(Sheet* sheet);

	void set_selected(bool selected);
	void set_as_moving(bool moving);
	int set_state( const QDomNode& node );

	AudioClip* create_copy();
	Track* get_track() const;
	Sheet* get_sheet() const;
	Peak* get_peak() const {return m_peak;}
	QDomNode get_state(QDomDocument doc);
	FadeCurve* get_fade_in() const;
	FadeCurve* get_fade_out() const;
	PluginChain* get_plugin_chain() const {return m_pluginChain;}
	
	TimeRef& get_length() const;
	TimeRef& get_source_length() const;
	TimeRef& get_track_start_location() const {return m_trackStartLocation;}
	TimeRef& get_track_end_location() const {return m_trackEndLocation;}
	TimeRef& get_source_start_location() const {return m_sourceStartLocation;}
	TimeRef& get_source_end_location() const {return m_sourceEndLocation;}
	
	int get_channels() const;
	int get_rate() const;
	int get_bitdepth() const;
	qint64 get_readsource_id() const;
	qint64 get_sheet_id() const {return m_sheetId;}
	ReadSource* get_readsource() const;
	
	QString get_name() const;
	QDomNode get_dom_node() const;
	
	bool is_take() const;
	bool is_selected() const;
	bool is_locked() const {return m_isLocked;}
	bool has_sheet() const;
	bool is_readsource_invalid() const {return !m_isReadSourceValid;}
	bool is_smaller_then(APILinkedListNode* node) {return ((AudioClip*)node)->get_track_start_location() > get_track_start_location();}
	bool is_moving() const {return m_isMoving;}

	int recording_state() const;

private:
	Track* 			m_track;
	Sheet* 			m_sheet;
	ReadSource*		m_readSource;
	WriteSource*		m_recorder;
	APILinkedList		m_fades;
	Peak* 			m_peak;
	AudioBus*		m_captureBus;
	FadeCurve*		fadeIn;
	FadeCurve*		fadeOut;
	QDomNode		m_domNode;
	
	QString 		m_name;
	QByteArray		m_captureBusName;
	
	mutable TimeRef 	m_trackStartLocation;
	mutable TimeRef 	m_trackEndLocation;
	mutable TimeRef 	m_sourceEndLocation;
	mutable TimeRef 	m_sourceStartLocation;
	mutable TimeRef		m_sourceLength;
	mutable TimeRef 	m_length;

	int 			m_isSelected;
	bool 			m_isTake;
	bool			m_isLocked;
	bool			m_isReadSourceValid;
	bool			m_isMoving;
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
	
	
	void calculate_normalization_factor(float targetdB = 0.0);
	
	friend class ResourcesManager;

signals:
	void stateChanged();
	void muteChanged();
	void lockChanged();
	void positionChanged(Snappable*);
	void trackEndLocationChanged();
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

	float get_gain() const;
	
	Command* mute();
	Command* reset_fade_in();
	Command* reset_fade_out();
	Command* reset_fade_both();
        Command* normalize();
        Command* denormalize();
	Command* lock();

private slots:
	void private_add_fade(FadeCurve* fade);
	void private_remove_fade(FadeCurve* fade);
	void get_capture_bus();

};


inline qint64 AudioClip::get_readsource_id( ) const {return m_readSourceId;}

inline float AudioClip::get_gain( ) const
{
	return m_fader->get_gain();
}


#endif
