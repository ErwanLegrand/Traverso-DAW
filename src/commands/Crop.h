/*
    Copyright (C) 2009 Remon Sijrier

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

#ifndef CROP_H
#define CROP_H

#include "Command.h"

class AudioClipView;
class Track;
class AudioClip;
class QGraphicsRectItem;

class Crop : public Command
{
        Q_OBJECT

public :
        Crop(AudioClipView* cv);
        ~Crop();

        int begin_hold();
        int finish_hold();
        int prepare_actions();
        int do_action();
        int undo_action();
        void cancel_action();

        int jog();

private:
        AudioClipView* m_cv;
        Track* m_track;
        AudioClip* m_clip;
        AudioClip* leftClip;
        AudioClip* rightClip;
        QGraphicsRectItem* m_selection;
        int x1;
        int x2;

public slots:
        void adjust_left(bool autorepeat);
        void adjust_right(bool autorepeat);
};

#endif


