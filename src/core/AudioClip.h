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

$Id: AudioClip.h,v 1.35 2007/03/22 18:17:59 r_sijrier Exp $
*/

#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include <QString>
#include <QList>
#include <QDomNode>

#include "ContextItem.h"
#include "defines.h"


class Song;
class AudioSource;
class ReadSource;
class WriteSource;
class Track;
class Peak;
class AudioBus;
class FadeCurve;
class Curve;

class AudioClip : public ContextItem
{
	Q_OBJECT
	Q_CLASSINFO("mute", tr("Mute"))
	Q_CLASSINFO("reset_fade_in", tr("In: Reset"))
	Q_CLASSINFO("reset_fade_out", tr("Out: Reset"))
	Q_CLASSINFO("reset_fade_both", tr("Reset both"))
	Q_CLASSINFO("clip_fade_in", tr("In: Range"))
	Q_CLASSINFO("clip_fade_out", tr("Out: Range"))
	Q_CLASSINFO("normalize", tr("Normalize"))
	Q_CLASSINFO("denormalize", tr("Normalize: reset"))

public:

	AudioClip(const QString& name);
	AudioClip(const QDomNode& node);
	~AudioClip();

	void create_fade_in();
	void create_fade_out();
	
	void set_audio_source(ReadSource* source);
	int init_recording(QByteArray bus);
	int process(nframes_t nframes, audio_sample_t* channelBuffer, uint channel);
	
	void set_blur(bool stat);
	void set_track_start_frame(nframes_t newTrackFirstFrame);
	void set_name(const QString& name);
	void set_fade_in(nframes_t b);
	void set_fade_out(nframes_t b);
	void set_track(Track* track);
	void set_song(Song* song);
	void set_snappable(bool snap);

	int set_selected(bool selected);
	int set_state( const QDomNode& node );
	int get_ref_count() const;

	AudioClip* prev_clip();
	AudioClip* next_clip();
	AudioClip* create_copy();
	Track* get_track() const;
	Song* get_song() const;
	Peak* get_peak_for_channel(int chan) const;
	QDomNode get_state(QDomDocument doc);
	FadeCurve* get_fade_in();
	FadeCurve* get_fade_out();
	Curve* get_gain_envelope() {return gainEnvelope;}
	
	float get_norm_factor() const;
	
	nframes_t get_length() const;
	nframes_t get_track_start_frame() const;
	nframes_t get_track_end_frame() const;
	nframes_t get_source_start_frame() const;
	nframes_t get_source_end_frame() const;
	nframes_t get_source_length() const;
	
	int get_channels() const;
	int get_rate() const;
	int get_bitdepth() const;
	qint64 get_readsource_id() const;
	
	QString get_name() const;
	
	bool is_muted() const;
	bool is_take() const;
	bool is_selected() const;
	bool is_recording() const;
	bool is_snappable() const {return m_isSnappable;}

	static bool smaller(const AudioClip* left, const AudioClip* right )
	{
		return left->get_track_start_frame() < right->get_track_start_frame();
	}
	static bool greater(const AudioClip* left, const AudioClip* right )
	{
		return left->get_track_start_frame() > right->get_track_start_frame();
	}

	QDomNode		m_domNode;

private:
	Track* 			m_track;
	Song* 			m_song;
	AudioSource* 		audioSource;
	ReadSource*		m_readSource;
	QList<WriteSource* >	writeSources;
	QList<FadeCurve* >	m_fades;
	AudioBus*		captureBus;
	FadeCurve*		fadeIn;
	FadeCurve*		fadeOut;
	Curve*			gainEnvelope;

	QString 		m_name;
	nframes_t 		trackStartFrame;
	nframes_t 		trackEndFrame;
	nframes_t 		sourceEndFrame;
	nframes_t 		sourceStartFrame;
	nframes_t		sourceLength;
	nframes_t 		m_length;

	bool 			isSelected;
	bool 			isTake;
	bool 			isMuted;
	bool 			isRecording;
	bool			m_isSnappable;
	float	 		m_gain;
	float			m_normfactor;
	
	qint64		m_readSourceId;
	int		m_refcount;

	void init();
	void set_source_end_frame(nframes_t frame);
	void set_source_start_frame(nframes_t frame);
	void set_track_end_frame(nframes_t endFrame);
	void set_sources_active_state();
	void process_capture(nframes_t nframes, uint channel);
	int ref() {return m_refcount++;}
	
	
	void calculate_normalization_factor(float targetdB = 0.0);
	
	friend class ResourcesManager;

signals:
	void stateChanged();
	void muteChanged();
	void positionChanged();
	void trackEndFrameChanged();
	void gainChanged();
	void fadeAdded(FadeCurve*);
	void fadeRemoved(FadeCurve*);

public slots:
	void finish_recording();
	void finish_write_source(WriteSource* source);
	void set_left_edge(long frame);
	void set_right_edge(long frame);
	void track_audible_state_changed();
	void toggle_mute();
	void set_gain(float g);

	float get_gain() const;
	
	Command* mute();
	Command* reset_fade_in();
	Command* reset_fade_out();
	Command* reset_fade_both();
        Command* clip_fade_in();
        Command* clip_fade_out();
        Command* normalize();
        Command* denormalize();

private slots:
	void private_add_fade(FadeCurve* fade);
	void private_remove_fade(FadeCurve* fade);

};


inline qint64 AudioClip::get_readsource_id( ) const {return m_readSourceId;}


#endif
