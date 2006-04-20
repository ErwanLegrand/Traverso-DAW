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
 
    $Id: AudioClip.h,v 1.2 2006/04/20 17:11:49 r_sijrier Exp $
*/

#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include <QString>
#include <QList>
#include <QDomDocument>

#include "ContextItem.h"
#include "defines.h"


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


        //  a TODO ? OK, this is a bit large to place here, but it gives a 30% speedup since it's called from
        // Mixer::mix_track_portion. Perhaps this should be moved in there?
        float get_fade_factor_for(nframes_t pos) // pos : position in the track
        {
                float fi,fo;
                if (fadeInBlocks==0)
                        fi = 1.0;
                else
                {
                        if (pos < trackStartFrame) // that happens in the edge, the current playing pos , when rounded to locate the clip, may lead to a position "considered" inside the clip, but it is not.
                                fi = 0.0;
                        else if (pos > trackStartFrame + fadeInBlocks)
                                fi = 1.0;
                        else
                                fi = (float)((double)(pos - trackStartFrame)/fadeInBlocks);
                }
                if (fadeOutBlocks==0)
                        fo = 1.0;
                else
                {
                        if (pos> trackEndFrame)
                                fo = 0.0;
                        else if ( pos < trackEndFrame - fadeOutBlocks)
                                fo = 1.0;
                        else
                                fo = (float)((double)(trackEndFrame-pos)/fadeOutBlocks);
                }
                // TODO calculate cross-fades-gains
                return fi*fo;
        }

        void add_audio_source(ReadSource* source, int channel);
        void set_blur(bool stat);
        void set_gain(float g);
        void set_muted(bool b);
        void set_track_first_block(nframes_t newTrackFirstBlock);
        void set_last_source_block(nframes_t block);
        void set_first_source_block(nframes_t block);
        void set_name(QString name)
        {
                m_name = name;
        }
        int set_selected();
        int set_state( const QDomNode& node );
        void set_fade_in(nframes_t b);
        void set_fade_out(nframes_t b);
        void set_track(Track* t)
        {
                m_track = t;
        }

        AudioClip* prev_clip();
        AudioClip* next_clip();
        AudioClip* create_copy();
        AudioSource* get_audio_source();
        Track* get_parent_track() const
        {
                return m_track;
        }
        Song* get_parent_song() const
        {
                return m_song;
        }
        Peak* get_peak_for_channel(int chan);
        QDomNode get_state(QDomDocument doc);
        float get_gain() const
        {
                return m_gain;
        }
        nframes_t get_length();
        nframes_t get_track_first_block() const
        {
                return trackStartFrame;
        }
        nframes_t get_track_last_block() const
        {
                return trackEndFrame;
        }
        nframes_t get_source_first_block() const
        {
                return sourceStartFrame;
        }
        nframes_t get_source_last_block() const
        {
                return sourceEndFrame;
        }
        nframes_t get_fade_in_blocks() const
        {
                return fadeInBlocks;
        }
        nframes_t get_fade_out_blocks() const
        {
                return fadeOutBlocks;
        }
        nframes_t get_source_length() const
        {
                return sourceLength;
        }
        int get_baseY();
        int get_width();
        int get_height();
        int get_channels();
        int get_rate() const
        {
                return rate;
        }
        int get_bitdepth() const
        {
                return bitDepth;
        }
        QString get_name() const
        {
                return m_name;
        }
        bool is_muted()
        {
                return isMuted;
        };
        bool is_take()
        {
                return isTake;
        };
        bool is_selected() const
        {
                return isSelected;
        }
        bool is_recording() const
        {
                return isRecording;
        }


        static bool smaller(const AudioClip* left, const AudioClip* right )
        {
                return left->get_track_first_block() < right->get_track_first_block();
        }
        static bool greater(const AudioClip* left, const AudioClip* right )
        {
                return left->get_track_first_block() > right->get_track_first_block();
        }

        int process(nframes_t nframes);
        int init_recording(QByteArray bus);

private:
        Track* 				m_track;
        Song* 				m_song;
        AudioSource* 			audioSource;
        QList<ReadSource* > 	readSources;
        QList<WriteSource* >	writeSources;
        audio_sample_t* 		mixdown;
        AudioBus*				captureBus;

        QString m_name;
        nframes_t trackStartFrame;
        nframes_t trackEndFrame;
        nframes_t sourceEndFrame;
        nframes_t sourceStartFrame;
        nframes_t	sourceLength;
        nframes_t fadeOutBlocks;
        nframes_t fadeInBlocks;
        nframes_t m_length;

        bool isSelected;
        bool isTake;
        bool isMuted;
        bool isRecording;
        float m_gain;
        int blockSize;
        uint m_channels;
        int rate;
        int bitDepth;

        void init();
        void set_track_end_frame(nframes_t endFrame);
        void set_sources_active_state();

signals:
        void stateChanged();
        void muteChanged(bool);
        void trackStartFrameChanged();
        void edgePositionChanged();

public slots:
        void finish_recording();
        void finish_write_source(WriteSource* source);
        void process_capture(nframes_t nframes);
        void set_left_edge(nframes_t block);
        void set_right_edge(nframes_t block);
        void resize_buffer();
        void track_mute_changed(bool mute);

        Command* drag_edge();
        Command* mute();
        Command* reset_gain();
        Command* reset_fade_in();
        Command* reset_fade_out();
        Command* reset_fade_both();
        Command* select();
        Command* deselect();
        Command* drag();
        Command* split();
        Command* copy();
        Command* add_to_selection();
        Command* gain();

};


#endif
