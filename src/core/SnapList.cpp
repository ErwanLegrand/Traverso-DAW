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

$Id: SnapList.cpp,v 1.3 2006/11/09 15:45:42 r_sijrier Exp $
*/

#include "SnapList.h"
#include "Peak.h"
#include "Song.h"
#include "AudioClip.h"
#include "AudioClipManager.h"
#include "ContextPointer.h"

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
	connect(m_song, SIGNAL(workingPosChanged()), this, SLOT(mark_dirty()));
}

void SnapList::mark_dirty()
{
	m_isDirty = true;
}


void SnapList::update_snaplist()
{
	xposList.clear();
	
	int scalefactor = Peak::zoomStep[m_song->get_hzoom()];

	// collects all clip boundaries and adds them to the snap list
	QList<AudioClip* >* acList = m_song->get_audioclip_manager()->get_clip_list();

	for( int i = 0; i < acList->size(); i++ ) {

		AudioClip* clip = acList->at(i);

		int startx = clip->get_track_start_frame() / scalefactor;
		int endx = clip->get_track_end_frame() / scalefactor;

		if (startx > endx) {
			PERROR("clip xstart > xend, this must be a programming error!");
			continue;  // something wrong, ignore this clip
		}
		if (startx >= m_rangeStart) {
	 		xposList.append(startx);
		}
		if (endx <= m_rangeEnd) {
			xposList.append(endx);
		}
	}

	// Be able to snap to trackstart
	if (m_rangeStart == 0) {
		xposList.append(0);
	}

	printf("workingframe xpos is %d\n", m_song->get_working_frame() / scalefactor);
	xposList.append(m_song->get_working_frame() / scalefactor);
	
	// if there are more items to be added to the list (e.g. markers, cursor),
	
	xposLut.clear();
	xposBool.clear();

	
	// sort the list
	qSort(xposList);
	
// 	int range = m_rangeEnd - m_rangeStart;

	// create a linear lookup table
	for (int i = m_rangeStart; i <= m_rangeEnd; ++i) {
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
		int ls = -SNAP_WIDTH;

		if (i > 0) {
			if (xposList.at(i-1) - xposList.at(i) > 2*ls) {
				ls = (xposList.at(i-1) - xposList.at(i)) / 2;
			}
		}

		for (int j = ls; j <= SNAP_WIDTH; j++) {
			int pos = xposList.at(i) + j; // index in the LUT

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
int SnapList::get_snap_value(int i)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	// catch dangerous values:
	if (i < 0) { 
		return i;
	}

	if (xposLut.isEmpty()) {
		return i;
	}

	if (i >= xposLut.size()) {
		return i;
	}

	return xposLut.at(i);
}

// returns true if i is inside a snap area, else returns false
bool SnapList::is_snap_value(int i)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	// need to catch values outside the LUT. Return false in that case
	if (i < 0) {
		return false;
	}

	if (i >= xposBool.size()) {
		return false;
	}

	return xposBool.at(i);
}

// returns the difference between the unsnapped and snapped location.
// The return value is negative if the supplied value is < snapped value
int SnapList::get_snap_diff(int i)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	// need to catch values outside the LUT. Return 0 in that case
	if (i < 0) {
		return 0;
	}

	if (i >= xposLut.size()) {
		return 0;
	}

	return i - xposLut.at(i);
}


/* EOF */
