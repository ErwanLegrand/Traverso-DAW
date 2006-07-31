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
 
    $Id: SongView.h,v 1.5 2006/07/31 13:27:27 r_sijrier Exp $
*/

#ifndef SONGVIEW_H
#define SONGVIEW_H

#include <libtraversocore.h>
#include <libtraverso.h>
#include "ViewItem.h"

class TrackView;
class LocatorView;
class Cursor;


class QPixmap;

class SongView : public ViewItem
{
        Q_OBJECT

        static const int SHUTTLE_SENSIBILITY = 80; // 1-100  : will be user configurable
        static const int VERTICAL_SCROLL_SENSIBILITY = 80; // 1-100  : will be user configurable
        static const int MOUSE_SENSIBILITY = 6;

        static const int MAX_CURSORS = 20;

        static const int CURSOR_FLOAT = 0;
        static const int CURSOR_FLOAT_OVER_CLIP = 1;
        static const int CURSOR_FLOAT_OVER_TRACK = 2;
        static const int CURSOR_FLOAT_OVER_SPLITTER = 3;
        static const int CURSOR_HOLD_UD = 4;
        static const int CURSOR_HOLD_LR = 5;
        static const int CURSOR_HOLD_LRUD = 6;
        static const int CURSOR_DRAG = 7;
        static const int CURSOR_SELECT = 8;
        static const int CURSOR_MAGIC_ZOOM = 9;
        static const int CURSOR_FLOAT_OVER_PLUGIN = 10;


public :

        SongView(Song* song, ViewPort* vp);
        ~SongView();


        int currentCursorMapIndex;
        QCursor cursorMap[MAX_CURSORS];

        void mouseMoveEvent(QMouseEvent* e);
        QRect draw(QPainter& painter);

        // Set functions

        // Get functions
        Song* get_song() const
        {
                return m_song;
        }
        Cursor* get_cursor() const
        {
                return m_cursor;
        }
        int cliparea_width() const;

        nframes_t xpos_to_frame(int xpos)
        {
                return m_song->get_first_visible_frame() + xpos * Peak::zoomStep[m_song->get_hzoom()];
        }
        int frame_to_xpos(nframes_t frame)
        {
                return ((frame - m_song->get_first_visible_frame())  / Peak::zoomStep[m_song->get_hzoom()]);
        }
        bool is_pointed() const
        {
                return true;
        }

        void clear_root_space(QPainter& painter);

        int shuttleFactor;

private:
        Song* m_song;
        Cursor* m_cursor;
        LocatorView* m_locator;

        QList<ViewItem* > viewItems;
        QList<ViewItem* > trackViewList;

        bool paintSplitter;
        bool paintTrackViews;

        void paint_splitter(QPainter& p);

        int jogZoomTotalX;
        int jogZoomTotalY;
        int lastJogZoomXFactor;
        int baseJogZoomXFactor;
        int verticalJogZoomLastY;
        int scrollAmount;
        int verticalScrollAmount;
        int verticalScrollTotalHeight;
        int origX;
        int origY;
        int mtaBaseY;


public slots:
        void add_new_trackview(Track* track);
        void remove_trackview(Track* track);
        void set_context();
        void schedule_for_repaint();
        void update_shuttle();
        void resize();

        Command* hzoom_out();
        Command* hzoom_in();
        Command* vzoom_out();
        Command* vzoom_in();
        Command* jog_vertical_scroll();
        Command* zoom();
        Command* center();
        Command* scroll_right();
        Command* scroll_left();
        Command* scroll_down();
        Command* scroll_up();
        Command* shuttle();
        Command* vertical_scroll();
        Command* goto_begin();
        Command* goto_end();
};

#endif
