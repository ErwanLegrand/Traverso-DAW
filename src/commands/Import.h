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
 
    $Id: Import.h,v 1.3 2007/01/16 20:21:08 r_sijrier Exp $
*/

#ifndef IMPORT_H
#define IMPORT_H

#include "Command.h"

class QString;
class AudioClip;

class Import : public Command
{
public :
        Import(Track* track);
        Import(Track* track, const QString& fileName);
        ~Import();

        int prepare_actions();
        int do_action();
        int undo_action();

private :
        Track* 		m_track;
        AudioClip*	m_clip;
        QString 	m_fileName;
};

#endif

