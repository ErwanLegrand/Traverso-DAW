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
 
    $Id: Gain.h,v 1.4 2006/11/08 14:52:11 r_sijrier Exp $
*/

#ifndef GAIN_H
#define GAIN_H

#include "Command.h"
#include <QPoint>

class ContextItem;


class Gain : public Command
{
public :
        Gain(ContextItem* context, const QString& des, float gain = -1.0);
        ~Gain();

        int begin_hold(int useX = 0, int useY = 0);
        int finish_hold();
        int prepare_actions();
        int do_action();
        int undo_action();

        int jog();
        
        void set_cursor_shape(int useX = 0, int useY = 0);

private :
	ContextItem*	gainObject;
        float 		origGain;
        float 		newGain;
        int 		origY;
	QPoint		mousePos;
};

#endif
 
