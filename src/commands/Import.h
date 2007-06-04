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
 
    $Id: Import.h,v 1.6 2007/06/04 22:44:25 benjie Exp $
*/

#ifndef IMPORT_H
#define IMPORT_H

#include "Command.h"

class QString;
class AudioClip;
class ReadSource;

class Import : public Command
{
public :
	Import(const QString& fileName);
        Import(Track* track, bool silent = false, nframes_t length = 0);
        Import(Track* track, const QString& fileName);
        Import(Track* track, const QString& fileName, nframes_t position);
        ~Import();

        int prepare_actions();
        int do_action();
        int undo_action();
	
	int create_readsource();
	void create_audioclip();
	void set_track(Track* track);
	void set_position(nframes_t position);

private :
        Track* 		m_track;
        AudioClip*	m_clip;
	ReadSource* 	m_source;
        QString 	m_fileName;
	QString		m_name;
	bool		m_silent;
	nframes_t	m_initialLength;
	bool		m_hasPosition;
	nframes_t	m_position;
};

#endif

