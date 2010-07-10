/*
Copyright (C) 2010 Remon Sijrier

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

#ifndef SUB_GROUP_VIEW_H
#define SUB_GROUP_VIEW_H

#include "TrackView.h"

class TBusTrack;

class TBusTrackView : public TrackView
{
        Q_OBJECT

public:
        TBusTrackView(SheetView* sv, TBusTrack* group);
        ~TBusTrackView() {}

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        void load_theme_data();
};


#endif

//eof
