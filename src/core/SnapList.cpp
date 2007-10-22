/*
Copyright (C) 2006-2007 Nicola Doebelin, Remon Sijrier

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

//#define debugsnaplist

#if defined (debugsnaplist)
#define SLPRINT(args...) printf(args)
#else
#define SLPRINT(args...)
#endif

static const int SNAP_WIDTH = 10;

SnapList::SnapList(Song* song) 
	: QObject(song)
	, m_song(song)
{
	m_isDirty = true;
	m_rangeStart = TimeRef();
	m_rangeEnd = TimeRef();
	m_scalefactor = 1;
}

void SnapList::mark_dirty(Snappable *item)
{
	if (item->is_snappable()) {
		m_isDirty = true;
	}
}

void SnapList::update_snaplist()
{
	m_xposList.clear();
	m_xposLut.clear();
	m_xposBool.clear();
	
	// collects all clip boundaries and adds them to the snap list
	QList<AudioClip* >* acList = m_song->get_audioclip_manager()->get_clip_list();
	
	SLPRINT("acList size is %d\n", acList->size());

	// Be able to snap to trackstart
	if (m_rangeStart == qint64(0)) {
		m_xposList.append(TimeRef());
	}

	for( int i = 0; i < acList->size(); i++ ) {

		AudioClip* clip = acList->at(i);
		if ( ! clip->is_snappable()) {
			continue;
		}

		TimeRef startlocation = clip->get_track_start_location();
		TimeRef endlocation = clip->get_track_end_location();

		if (startlocation > endlocation) {
			PERROR("clip xstart > xend, this must be a programming error!");
			continue;  // something wrong, ignore this clip
		}
		if (startlocation >= m_rangeStart && startlocation <= m_rangeEnd) {
	 		m_xposList.append(startlocation);
		}
		if (endlocation >= m_rangeStart && endlocation <= m_rangeEnd) {
			m_xposList.append(endlocation);
		}
	}

	// add all on-screen markers
	QList<Marker*> markerList = m_song->get_timeline()->get_markers();
	for (int i = 0; i < markerList.size(); ++i) {
		if (markerList.at(i)->is_snappable() && markerList.at(i)->get_when() >= m_rangeStart && markerList.at(i)->get_when() <= m_rangeEnd) {
			m_xposList.append(markerList.at(i)->get_when());
		}
	}

	// add the working cursor's position
	TimeRef worklocation = m_song->get_work_location();
	//printf("worklocation xpos is %d\n",  worklocation / m_scalefactor);
	if (m_song->get_work_snap()->is_snappable() && worklocation >= m_rangeStart && worklocation <= m_rangeEnd) {
		m_xposList.append(m_song->get_work_location());
	}
	

	// sort the list
	qSort(m_xposList);

	int range = (int)((m_rangeEnd - m_rangeStart) / m_scalefactor);

	// create a linear lookup table
	for (int i = 0; i <= range; ++i) {
		m_xposLut.push_back(TimeRef());
		m_xposBool.push_back(false);
	}

	TimeRef lastVal;
	long lastIndex = -1;
	// now modify the regions around snap points in the lookup table
	for (int i = 0; i < m_xposList.size(); i++) {
		if (lastIndex > -1 && m_xposList.at(i) == lastVal) {
			continue;  // check for duplicates and skip them
		}

		// check if neighbouring snap regions overlap.
		// if yes, reduce SNAP_WIDTH to keep the border in the middle
		int ls = -SNAP_WIDTH;

		if (lastIndex > -1) {
			if ( (m_xposList.at(i) - lastVal) < (2 * SNAP_WIDTH * m_scalefactor) ) {
				ls = - (int) ((m_xposList.at(i) / m_scalefactor - lastVal / m_scalefactor) / 2);
			}
		}

		for (int j = ls; j <= SNAP_WIDTH; j++) {
			int pos = (int)((m_xposList.at(i) - m_rangeStart) / m_scalefactor + j); // index in the LUT

			if (pos < 0) {
				continue;
			}

			if (pos >= m_xposLut.size()) {
				break;
			}

			m_xposLut[pos] = m_xposList.at(i);
			m_xposBool[pos] = true;
		}

		lastVal = m_xposList.at(i);
		lastIndex = i;
	}
	
	m_isDirty = false;
}


// public function that checks if there is a snap position
// within +-SNAP_WIDTH of the supplied value i
TimeRef SnapList::get_snap_value(const TimeRef& pos)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	int i = (int)((pos - m_rangeStart) / m_scalefactor);
	SLPRINT("get_snap_value:: i is %d\n", i);
	
	// catch dangerous values:
	if (i < 0) { 
		SLPRINT("get_snap_value:: i < 0\n");
		return pos;
	}

	if (m_xposLut.isEmpty()) {
		SLPRINT("get_snap_value:: empty lut\n");
		return pos;
	}

	if (i >= m_xposLut.size()) {
		SLPRINT("get_snap_value:: i > m_xposLut.size()\n");
		return pos;
	}
	
	if (is_snap_value(pos)) {
		SLPRINT("get_snap_value returns: %d (was %d)\n", m_xposLut.at(i), pos);
		return m_xposLut.at(i);
	}
	
	
	SLPRINT("get_snap_value returns: %d (was %d)\n", pos, pos);
	return pos;
}

// returns true if i is inside a snap area, else returns false
bool SnapList::is_snap_value(const TimeRef& pos)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	int i = (int)((pos - m_rangeStart) / m_scalefactor);
	SLPRINT("is_snap_value:: i is %d\n", i);
	
	// need to catch values outside the LUT. Return false in that case
	if (i < 0) {
		return false;
	}

	if (i >= m_xposBool.size()) {
		return false;
	}

	SLPRINT("is_snap_value returns: %d\n", m_xposBool.at(i));
	return m_xposBool.at(i);
}

// returns the difference between the unsnapped and snapped location.
// The return value is negative if the supplied value is < snapped value
qint64 SnapList::get_snap_diff(const TimeRef& pos)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	int i = (int)((pos - m_rangeStart) / m_scalefactor);
	SLPRINT("get_snap_diff:: i is %d\n", i);
	
	// need to catch values outside the LUT. Return 0 in that case
	if (i < 0) {
		return 0;
	}

	if (i >= m_xposLut.size()) {
		return 0;
	}

	SLPRINT("get_snap_diff returns: %d\n", m_xposLut.at(i));
	return (pos - m_xposLut.at(i)).universal_frame();
}

void SnapList::set_range(const TimeRef& start, const TimeRef& end, int scalefactor)
{
 	SLPRINT("setting xstart %d, xend %d scalefactor %d\n", start, end, scalefactor);

	if (m_rangeStart == start && m_rangeEnd == end && m_scalefactor == scalefactor) {
		return;
	}

	m_rangeStart = start;
	m_rangeEnd = end;
	m_scalefactor = scalefactor;
	m_isDirty = true;
};

TimeRef SnapList::next_snap_pos(const TimeRef& pos)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	int index = (int)(pos / m_scalefactor);
	
	TimeRef newpos = pos;
	
	if (index > m_xposLut.size()) {
		index = m_xposLut.size() - 1;
	}
	
	for (; index<m_xposLut.size(); ++index) {
		TimeRef snap = m_xposLut.at(index);
		if (snap > pos) {
			newpos = snap;
			break;
		}
	}
	
	return newpos;
}

TimeRef SnapList::prev_snap_pos(const TimeRef& pos)
{
	if (m_isDirty) {
		update_snaplist();
	}
	
	if (! m_xposLut.size()) {
		return pos;
	}
	
	TimeRef newpos = pos;
	
	int index = (int)(pos / m_scalefactor);
	if (index > m_xposLut.size()) {
		index = m_xposLut.size() - 1;
	}
	
	do {
		TimeRef snap = m_xposLut.at(index);
		if (snap < pos && snap != TimeRef(qint64(0))) {
			newpos = snap;
			break;
		}
		index--;
	} while (index >= 0);
	
	if (index == -1) {
		return TimeRef();
	}
	
	if (newpos == pos) {
		return pos;
	}
	
	return newpos;
}

