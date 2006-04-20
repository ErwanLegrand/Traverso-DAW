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
 
    $Id: TrackView.h,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include "ViewItem.h"
#include "Track.h"

class QPixmap;

class Track;
class AudioClip;
class SongView;
class PanelLed;
class BusSelector;
class AudioClipView;

class TrackView : public ViewItem
{
        Q_OBJECT

        int jogMode;
        static const int JOG_NONE = 0;
        static const int JOG_GAIN_AND_PAN = 1;

public:
        static const int TRACKPANELWIDTH = 180;
        static const int SPLITTERWIDTH = 5;
        static const int CLIPAREABASEX = TRACKPANELWIDTH + SPLITTERWIDTH;


        TrackView(ViewPort* vp, SongView* sv, Track* track);
        ~TrackView();

        QRect draw(QPainter& painter);
        int get_base_y() const
        {
                return m_track->get_baseY();
        }
        SongView* get_songview() const
        {
                return m_sv;
        }
        Track*	get_track() const
        {
                return m_track;
        }
        int cliparea_basex() const;
        int cliparea_width() const;

private:
        Track* m_track;
        SongView* m_sv;
        BusSelector* 		busSelector;
        QMenu* 			busInMenu;
        QMenu* 			busOutMenu;

        QString sid;
        QString sgain;
        QString span;
        QPixmap panelPixmap;
        int panelWidth;

        bool paintPanel;
        bool paintClipArea;

        int clipAreaWidth;

        int origX;
        int origY;
        float origPan;

        PanelLed* muteLed;
        PanelLed* soloLed;
        PanelLed* recLed;
        PanelLed* lockLed;


        void clear_clip_area(QPainter& p);

        void draw_panel_track_name();
        void draw_panel_leds(QPainter& p);
        void draw_panel_bus_in_out();
        void draw_panel_head();
        void draw_panel_pan();
        void draw_panel_gain();
        void resize_panel_pixmap();

        void jog_gain_pan(int mouseX, int mouseY);
        void touch_track(int trackNumber = -1, int xpos = -1);

        QList<AudioClipView* > audioClipViewList;

public slots:
        void add_new_audioclipview(AudioClip* clip);
        void remove_audioclipview(AudioClip* clip);
        void repaint_cliparea();
        void panel_info_changed();
        void height_changed();
        void schedule_for_repaint();
        void resize();
        void repaint_all_clips();
        void set_bus_in(QAction* action);
        void set_bus_out(QAction* action);

        Command* gain_and_pan();
        Command* jog_track_pan();
        Command* touch();
        Command* touch_and_center();
        Command* capture_from_channel_both();
        Command* capture_from_channel_left();
        Command* capture_from_channel_right();
        Command* select_bus_in();
        Command* select_bus_out();
};



#endif

//eof
