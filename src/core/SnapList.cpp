/*
Copyright (C) 2006-2008 Nicola Doebelin, Remon Sijrier

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
#include "TSession.h"
#include "Sheet.h"
#include "AudioClip.h"
#include "AudioClipManager.h"
#include "Config.h"
#include "ContextPointer.h"
#include "TimeLine.h"
#include "Utils.h"
#include "Marker.h"

#include <QString>

#include <Debugger.h>

//#define debugsnaplist

#if defined(debugsnaplist)
#define SLPRINT(args...) printf(args)
#else
#define SLPRINT(args...);
#endif

SnapList::SnapList(TSession* sheet)
	: m_sheet(sheet)
{
	m_isDirty = true;
	m_rangeStart = TimeRef();
	m_rangeEnd = TimeRef();
	m_scalefactor = 1;
}

void SnapList::mark_dirty()
{
// 	printf("mark_dirty()\n");
	m_isDirty = true;
	m_wasDirty = true;
}

void SnapList::update_snaplist()
{
	m_xposList.clear();
	m_xposLut.clear();
	m_xposBool.clear();
	
	// collects all clip boundaries and adds them to the snap list
        QList<AudioClip* > acList;
        Sheet* sheet = qobject_cast<Sheet*>(m_sheet);
        if (sheet) {
                acList.append(sheet->get_audioclip_manager()->get_clip_list());
        }
	
	SLPRINT("acList size is %d\n", acList.size());

	// Be able to snap to trackstart
        if (m_rangeStart == TimeRef()) {
		m_xposList.append(TimeRef());
	}

	for( int i = 0; i < acList.size(); i++ ) {

		AudioClip* clip = acList.at(i);
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
	QList<Marker*> markerList = m_sheet->get_timeline()->get_markers();
	for (int i = 0; i < markerList.size(); ++i) {
		if (markerList.at(i)->is_snappable() && markerList.at(i)->get_when() >= m_rangeStart && markerList.at(i)->get_when() <= m_rangeEnd) {
			m_xposList.append(markerList.at(i)->get_when());
		}
	}

	// add the working cursor's position
	TimeRef worklocation = m_sheet->get_work_location();
	//printf("worklocation xpos is %d\n",  worklocation / m_scalefactor);
	if (m_sheet->get_work_snap()->is_snappable() && worklocation >= m_rangeStart && worklocation <= m_rangeEnd) {
		m_xposList.append(m_sheet->get_work_location());
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
		// if yes, reduce snap-range to keep the border in the middle
		int snaprange = config().get_property("Snap", "range", 10).toInt();
		int ls = - snaprange;

		if (lastIndex > -1) {
			if ( (m_xposList.at(i) - lastVal) < (2 * snaprange * m_scalefactor) ) {
				ls = - (int) ((m_xposList.at(i) / m_scalefactor - lastVal / m_scalefactor) / 2);
			}
		}

		for (int j = ls; j <= snaprange; j++) {
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
// within +- snap-range of the supplied value i
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
                SLPRINT("get_snap_value returns: %s (was %s)\n", timeref_to_ms_3(m_xposLut.at(i)).toAscii().data(), timeref_to_ms_3(pos).toAscii().data());
		return m_xposLut.at(i);
	}
	
	
        SLPRINT("get_snap_value returns: %s (was %s)\n", timeref_to_ms_3(pos).toAscii().data(), timeref_to_ms_3(pos).toAscii().data());
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

        SLPRINT("get_snap_diff returns: %s\n", timeref_to_ms_3(m_xposLut.at(i)).toAscii().data());
	return (pos - m_xposLut.at(i)).universal_frame();
}

void SnapList::set_range(const TimeRef& start, const TimeRef& end, int scalefactor)
{
        SLPRINT("setting xstart %s, xend %s scalefactor %d\n", timeref_to_ms_3(start).toAscii().data(), timeref_to_ms_3(end).toAscii().data(), scalefactor);

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

        SLPRINT("next_snap_pos: index %d\n", index);
	
	if (pos < TimeRef()) {
		PERROR("pos < 0");
		return TimeRef();
	}
	
        if (index >= m_xposLut.size()) {
                SLPRINT("index > m_xposLut.size() (index is %d)\n", index);
		index = m_xposLut.size() - 1;
	}
	
	TimeRef newpos = pos;
	
        // TODO: find out why using the found index above doesn't work
        // whem moving the workcursor. Using linear search seems to work ok
        for (int i=0; i<m_xposLut.size(); ++i) {
		TimeRef snap = m_xposLut.at(i);
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
	
	if (pos < TimeRef()) {
		PERROR("pos < 0");
		return TimeRef();
	}
	
	if (! m_xposLut.size()) {
		return pos;
	}
	
	int index = (int)(pos / m_scalefactor);
	
        if (index >= m_xposLut.size()) {
		index = m_xposLut.size() - 1;
	}
	
	TimeRef newpos = pos;
	
	do {
		TimeRef snap = m_xposLut.at(index);
		if (snap < pos && snap != TimeRef()) {
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


TimeRef SnapList::calculate_snap_diff(TimeRef leftlocation, TimeRef rightlocation)
{
	// "nframe_t" domain, but must be signed ints because they can become negative
	qint64 snapStartDiff = 0;
	qint64 snapEndDiff = 0;
	qint64 snapDiff = 0;
	
	// check if there is anything to snap
	bool start_snapped = false;
	bool end_snapped = false;
	if (is_snap_value(leftlocation)) {
		start_snapped = true;
	}
	
	if (is_snap_value(rightlocation)) {
		end_snapped = true;
	}

	if (start_snapped) {
		snapStartDiff = get_snap_diff(leftlocation);
		snapDiff = snapStartDiff; // in case both ends snapped, change this value later, else leave it
	}

	if (end_snapped) {
		snapEndDiff = get_snap_diff(rightlocation); 
		snapDiff = snapEndDiff; // in case both ends snapped, change this value later, else leave it
	}

	// If both snapped, check which one is closer. Do not apply this check if one of the
	// ends hasn't snapped, because it's diff value will be 0 by default and will always
	// be smaller than the actually snapped value.
	if (start_snapped && end_snapped) {
		if (abs(snapEndDiff) > abs(snapStartDiff))
			snapDiff = snapStartDiff;
		else
			snapDiff = snapEndDiff;
	}
	
	return TimeRef(snapDiff);
}

bool SnapList::was_dirty()
{
	bool ret = m_wasDirty;
	m_wasDirty = false;
	return ret;
}
