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
 
    $Id: TimeLineView.h,v 1.3 2007/02/08 20:51:38 r_sijrier Exp $
*/

#ifndef TIME_LINE_VIEW_H
#define TIME_LINE_VIEW_H

#include "ViewItem.h"

class SongView;

class TimeLineView : public ViewItem
{
        Q_OBJECT

public:
        TimeLineView(SongView* view);
        ~TimeLineView();
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void load_theme_data() {update();}

public slots:
        void hzoom_changed( );
};

#endif

//eof
