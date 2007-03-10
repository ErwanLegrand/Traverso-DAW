/*
Copyright (C) 2006 Nicola Doebelin

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

$Id: SnapList.cpp,v 1.6 2007/03/10 21:57:01 n_doebelin Exp $
*/

#include "SnapList.h"
#include "Peak.h"
#include "Song.h"
#include "AudioClip.h"
#include "AudioClipManager.h"
#include "ContextPointer.h"
#include "TimeLine.h"
#include "Marker.h"

#include <QString>

#include <Debugger.h>

static const int SNAP_WIDTH = 10;

SnapList::SnapList(Song* song) 
	: QObject(song)
	, m_song(song)
{
	m_isDirty = true;
	m_rangeStart = 0;
	m_rangeEnd = 900;
	m_scalefactor = 1;
	connect(m_song, SIGNAL(workingPosChanged()), this, SLOT(mark_dirty()));
}

void SnapList::mark_dirty()
{
	m_isDirty = true;
}


void SnapList::update_snaplist()
{
	xposList.clear();
	xposLut.clear();
	xposBool.clear();
	
	// collects all clip boundaries and adds them to the snap list
	QList<AudioClip* >* acList = m_song->get_audioclip_manager()->get_clip_list();
	
// 	printf("acList size is %d\n", acList->size());

	for( int i = 0; i < acList->size(); i++ ) {

		AudioClip* clip = acList->at(i);
		if ( ! clip->is_snappable()) {
			continue;
		}

		nframes_t startframe = clip->get_track_start_frame();
		nframes_t endframe = clip->get_track_end_frame();

		if (startframe > endframe) {
			PERROR("clip xstart > xend, this must be a programming error!");
			continue;  // something wrong, ignore this clip
		}
		if (startframe >= m_rangeStart) {
	 		xposList.append(startframe);
		}
		if (endframe <= m_rangeEnd) {
			xposList.append(endframe);
		}
	}

	// Be able to snap to trackstart
	xposList.append(0);

	// add all markers
	QList<Marker*> markerList = m_song->get_timeline()->get_markers();
	for (int i = 0; i < markerList.size(); ++i) {
		xposList.append(markerList.at(i)->get_when());
	}

	// add the working cursor's position
	nframes_t workingframe = m_song->get_working_frame();
	printf("workingframe xpos is %d\n",  workingframe / m_scalefactor);
	if (workingframe >= m_rangeStart && workingframe <= m_rangeEnd) {
		xposList.append(m_song->get_working_frame());
	}
	

	// sort the list
	qSort(xposList);
	
	int range = (m_rangeEnd - m_rangeStart) / m_scalefactor;

	// create a linear lookup table
	for (int i = 0; i <= range; ++i) {
		xposLut.push_back(i);
		xposBool.push_back(false);
	}

	int lastVal = -1;
	// now modify the regions around snap points in the lookup table
	for (int i = 0; i < xposList.size(); i++) {
		if (xposList.at(i) == lastVal) {
			continue;  // check for duplicates and skip them
		}

		// check if neighbouring snap regions overlap.
		// if yes, reduce SNAP_WIDTH to keep the border in the middle
		int ls = - (SNAP_WIDTH * m_scalefactor);

		if (i > 0) {
			if ( (xposList.at(i-1) - xposList.at(i)) > (2 * ls) ) {
				ls = (xposList.at(i-1) - xposList.at(i)) / 2;
			}
		}

		for (int j = ls; j <= (SNAP_WIDTH * m_scalefactor); j+=m_scalefactor) {
			int pos = (xposList.at(i) + j) / m_scalefactor; // index in the LUT

			if (pos < 0) {
				continue;
			}

			if (pos >= xposLut.size()) {
				break;
			}

			xposLut[pos] = xposList.at(i);
			xposBool[pos] = true;
		}

		lastVal = xposList.at(i);
	}
	
	m_isDirty = false;
}


// public function that checks if there is a snap position
// within +-SNAP_WIDTH of the supplied value i
int SnapList::get_snap_value(nframes_t pos)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	int i = (pos - m_rangeStart) / m_scalefactor;
// 	printf("get_snap_value:: i is %d\n", i);
	
	// catch dangerous values:
	if (i < 0) { 
// 		printf("get_snap_value:: i < 0\n");
		return pos;
	}

	if (xposLut.isEmpty()) {
// 		printf("get_snap_value:: empty lut\n");
		return pos;
	}

	if (i >= xposLut.size()) {
// 		printf("get_snap_value:: i > xposLut.size()\n");
		return pos;
	}
	
	if (is_snap_value(pos)) {
// 		printf("get_snap_value returns: %d (was %d)\n", xposLut.at(i), pos);
		return xposLut.at(i);
	}
	
	
// 	printf("get_snap_value returns: %d (was %d)\n", pos, pos);
	return pos;
}
// returns true if i is inside a snap area, else returns false
bool SnapList::is_snap_value(nframes_t pos)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	int i = (pos - m_rangeStart) / m_scalefactor;
// 	printf("is_snap_value:: i is %d\n", i);
	
	// need to catch values outside the LUT. Return false in that case
	if (i < 0) {
		return false;
	}

	if (i >= xposBool.size()) {
		return false;
	}

// 	printf("is_snap_value returns: %d\n", xposBool.at(i));
	return xposBool.at(i);
}

// returns the difference between the unsnapped and snapped location.
// The return value is negative if the supplied value is < snapped value
int SnapList::get_snap_diff(nframes_t pos)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	int i = (pos - m_rangeStart) / m_scalefactor;
// 	printf("get_snap_diff:: i is %d\n", i);
	
	// need to catch values outside the LUT. Return 0 in that case
	if (i < 0) {
		return 0;
	}

	if (i >= xposLut.size()) {
		return 0;
	}

// 	printf("get_snap_diff returns: %d\n", xposLut.at(i));
	return pos - xposLut.at(i);
}


/* EOF */
