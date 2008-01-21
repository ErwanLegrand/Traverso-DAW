/*
Copyright (C) 2007 Remon Sijrier 

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

$Id: ArmTracks.cpp,v 1.4 2008/01/21 16:22:11 r_sijrier Exp $
*/

#include "ArmTracks.h"

#include <Track.h>
#include <Sheet.h>
#include <SheetView.h>
#include <TrackView.h>

#include <ContextPointer.h>


// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


ArmTracks::ArmTracks(SheetView* view)
	: Command(view->get_sheet(), tr("Arm Tracks"))
	, m_sv(view)
{
}


ArmTracks::~ArmTracks()
{}

int ArmTracks::prepare_actions()
{
	return -1;
}

int ArmTracks::begin_hold()
{
	m_sv->start_shuttle(true, true);
	return 1;
}

int ArmTracks::finish_hold()
{
	m_sv->start_shuttle(false);
	return 1;
}

int ArmTracks::do_action()
{
	return 1;
}

int ArmTracks::undo_action()
{
	return 1;
}

int ArmTracks::jog()
{
	TrackView* view = m_sv->get_trackview_under(cpointer().scene_pos());
	
	if ( ! view ) {
		return 0;
	}
	
	Track* track = view->get_track();
	
	if (! m_tracks.contains(track) ) {
		m_tracks.append(track);
		track->arm();
	}
	
	m_sv->update_shuttle_factor();
	
	return 1;
}

// eof

