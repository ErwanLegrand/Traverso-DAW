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

$Id: AudioClip.h,v 1.13 2006/06/16 20:18:51 r_sijrier Exp $
*/

#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include <QString>
#include <QList>
#include <QDomDocument>
#include <QPolygonF>

#include "ContextItem.h"
#include "defines.h"

#include "Curve.h"


class Song;
class AudioSource;
class ReadSource;
class WriteSource;
class Track;
class IEMessage;
class Peak;
class AudioBus;


class AudioClip : public ContextItem
{
	Q_OBJECT

public:

	AudioClip(Track* track, nframes_t aTrackInsertBlock, QString name);
	AudioClip(Track* track, const QDomNode& node);
	~AudioClip();


	enum FadeShape {
		Fastest,
		Fast,
		Linear,
		Slow,
		Slowest
	};
	
	void add_audio_source(ReadSource* source, int channel);
	int init_recording(QByteArray bus);
	int process(nframes_t nframes, audio_sample_t* channelBuffer, uint channel);
	
	void set_blur(bool stat);
	void set_gain(float g);
	void set_track_start_frame(nframes_t newTrackFirstFrame);
	void set_source_end_frame(nframes_t frame);
	void set_source_start_frame(nframes_t frame);
	void set_name(QString name);
	void set_fade_in(nframes_t b);
	void set_fade_out(nframes_t b);
	void set_track(Track* t);
	void set_fade_in_shape (FadeShape shape, nframes_t len);
	void set_fade_out_shape (FadeShape shape, nframes_t len);

	int set_selected(bool selected);
	int set_state( const QDomNode& node );

	AudioClip* prev_clip();
	AudioClip* next_clip();
	AudioClip* create_copy();
	Track* get_track() const;
	Song* get_song() const;
	Peak* get_peak_for_channel(int chan) const;
	QDomNode get_state(QDomDocument doc);
	Curve* get_fade_in() {return fadeIn;}
	Curve* get_fade_out() {return fadeOut;}
	Curve* get_gain_envelope() {return gainEnvelope;}
	
	float get_gain() const;
	float get_norm_factor() const;
	
	nframes_t get_length() const;
	nframes_t get_track_start_frame() const;
	nframes_t get_track_end_frame() const;
	nframes_t get_source_start_frame() const;
	nframes_t get_source_end_frame() const;
	nframes_t get_fade_in_frames() const;
	nframes_t get_fade_out_frames() const;
	nframes_t get_source_length() const;
	
	int get_baseY() const;
	int get_width() const;
	int get_height() const;
	int get_channels() const;
	int get_rate() const;
	int get_bitdepth() const;
	
	QString get_name() const;
	
	bool is_muted() const;
	bool is_take() const;
	bool is_selected() const;
	bool is_recording() const;

	static bool smaller(const AudioClip* left, const AudioClip* right )
	{
		return left->get_track_start_frame() < right->get_track_start_frame();
	}
	static bool greater(const AudioClip* left, const AudioClip* right )
	{
		return left->get_track_start_frame() > right->get_track_start_frame();
	}


private:
	Track* 			m_track;
	Song* 			m_song;
	AudioSource* 		audioSource;
	QList<ReadSource* > 	readSources;
	QList<WriteSource* >	writeSources;
	AudioBus*		captureBus;
	Curve*			fadeIn;
	Curve*			fadeOut;
	Curve*			gainEnvelope;
	FadeShape		fadeInShape;
	FadeShape		fadeOutShape;

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
	float	 		m_gain;
	float			m_normfactor;
	uint 			m_channels;
	int 			rate;
	int 			bitDepth;

	void init();
	void set_track_end_frame(nframes_t endFrame);
	void set_sources_active_state();
	void process_capture(nframes_t nframes, uint channel);
	
	void calculate_normalization_factor(float targetdB = 0.0);

signals:
	void stateChanged();
	void muteChanged(bool);
	void positionChanged();
	void trackEndFrameChanged();
	void gainChanged();

public slots:
	void finish_recording();
	void finish_write_source(WriteSource* source);
	void set_left_edge(nframes_t frame);
	void set_right_edge(nframes_t frame);
	void track_audible_state_changed(bool status);
	void toggle_mute();

	Command* drag_edge();
	Command* mute();
	Command* reset_gain();
	Command* reset_fade_in();
	Command* reset_fade_out();
	Command* reset_fade_both();
	Command* select();
	Command* remove_from_selection();
	Command* drag();
	Command* split();
	Command* copy();
	Command* add_to_selection();
	Command* gain();
        Command* clip_fade_in();
        Command* clip_fade_out();
        Command* normalize();
        Command* denormalize();

};


#endif
