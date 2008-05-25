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
 
    $Id: Import.h,v 1.10 2008/05/25 13:59:10 n_doebelin Exp $
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
	Import(Track* track, const TimeRef& length, bool silent = false);
        Import(Track* track, const QString& fileName);
        Import(Track* track, const QString& fileName, const TimeRef& position);
        ~Import();

        int prepare_actions();
        int do_action();
        int undo_action();
	
	int create_readsource();
	void create_audioclip();
	void set_track(Track* track);
	void set_position(const TimeRef& position);
	ReadSource* readsource() {return m_source;};

private :
        Track* 		m_track;
        AudioClip*	m_clip;
	ReadSource* 	m_source;
        QString 	m_fileName;
	QString		m_name;
	bool		m_silent;
	TimeRef		m_initialLength;
	bool		m_hasPosition;
	TimeRef		m_position;

	void init(Track* track, const QString& filename);
};

#endif

