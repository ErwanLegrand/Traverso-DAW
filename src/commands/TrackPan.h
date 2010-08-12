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

#ifndef TRACKPAN_H
#define TRACKPAN_H

#include "Command.h"

class Sheet;
class Track;

class TrackPan : public Command
{
	Q_OBJECT

public :
        TrackPan(Track* track, QVariantList args);

        int begin_hold();
        int finish_hold();
        int prepare_actions();
        int do_action();
        int undo_action();
        void cancel_action();

        int jog();

	void set_cursor_shape(int useX, int useY);
	
private :
	struct Data {
		QPoint	mousePos;
		int origX;
	};
	Data* d;
	
	float m_origPan;
	float m_newPan;
        Track* m_track;
	
public slots:
	void pan_left(bool autorepeat);
	void pan_right(bool autorepeat);

};

#endif


