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

$Id: RemoveTrack.cpp,v 1.4 2006/07/03 17:51:56 r_sijrier Exp $
*/

#include "RemoveTrack.h"

#include <libtraversocore.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


RemoveTrack::RemoveTrack(Song* song)
		: Command(song)
{
	m_song = song;
}


RemoveTrack::~RemoveTrack()
{}

int RemoveTrack::prepare_actions()
{
	m_track = m_song->get_track_under_y(cpointer().y());
	
	if ( ! m_track) {
		return 0;
	}
	
	return 1;
}

int RemoveTrack::do_action()
{
	PENTER;
	
	m_song->remove_track(m_track);

	return 1;
}

int RemoveTrack::undo_action()
{
	PENTER;
	
	m_song->add_track(m_track);

	return 1;
}


// eof


 
 
