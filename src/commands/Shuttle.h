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
 
    $Id: Shuttle.h,v 1.1 2006/04/20 14:51:13 r_sijrier Exp $
*/

#ifndef SHUTTLE_H
#define SHUTTLE_H

#include <QTimer>

#include "Command.h"

class SongView;
class ViewPort;

class Shuttle : public Command
{
public :
        Shuttle(SongView* sv, ViewPort* vp);
        ~Shuttle();

        int begin_hold();
        int finish_hold();
        int prepare_actions();
	int do_action();
	int undo_action();

        int jog();

private :
        QTimer		m_timer;
        SongView*	m_sv;
        ViewPort*		m_vp;
        int			shuttleFactor;
};

#endif


