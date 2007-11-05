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

*/

#include "TimeLine.h"

#include "Song.h"
#include "Marker.h"
#include <AddRemove.h>
#include "AudioDevice.h"


TimeLine::TimeLine(Song * song)
	: ContextItem(song)
	, m_song(song)
{
	set_history_stack(m_song->get_history_stack());
}

QDomNode TimeLine::get_state(QDomDocument doc)
{
	QDomElement domNode = doc.createElement("TimeLine");
	QDomNode markersNode = doc.createElement("Markers");
	domNode.appendChild(markersNode);

	for (int i = 0; i < m_markers.size(); ++i) {
		markersNode.appendChild(m_markers.at(i)->get_state(doc));
	}

	return domNode;
}

int TimeLine::set_state(const QDomNode & node)
{
	m_markers.clear();

	QDomNode markersNode = node.firstChildElement("Markers");
	QDomNode markerNode = markersNode.firstChild();

	while (!markerNode.isNull()) {
		Marker* marker = new Marker(this, markerNode);
		m_markers.append(marker);
		markerNode = markerNode.nextSibling();
	}

	return 1;
}

Command * TimeLine::add_marker(Marker* marker, bool historable)
{
	AddRemove* cmd;
	cmd = new AddRemove(this, marker, historable, m_song,
		"private_add_marker(Marker*)", "markerAdded(Marker*)",
		"private_remove_marker(Marker*)", "markerRemoved(Marker*)",
  		tr("Add Marker"));
	
	// Bypass the real time thread save logic in tsar, since a Marker doesn't have YET
	// anything to do with audio processing routines.
	// WARNING: this should change as soon as Markers modify anything related to audio 
	// processing objects!!!!!!
	cmd->set_instantanious(true);

	return cmd;
}

Command* TimeLine::remove_marker(Marker* marker, bool historable)
{
	AddRemove* cmd;
	cmd = new AddRemove(this, marker, historable, m_song,
		"private_remove_marker(Marker*)", "markerRemoved(Marker*)",
		"private_add_marker(Marker*)", "markerAdded(Marker*)",
  		tr("Remove Marker"));
	
	// Bypass the real time thread save logic in tsar, since a Marker doesn't have YET
	// anything to do with audio processing routines.
	// WARNING: this should change as soon as Markers modify anything related to audio 
	// processing objects!!!!!!
	cmd->set_instantanious(true);

	return cmd;
}

void TimeLine::private_add_marker(Marker * marker)
{
	m_markers.append(marker);
}

void TimeLine::private_remove_marker(Marker * marker)
{
	m_markers.removeAll(marker);
}

Marker * TimeLine::get_marker(qint64 id)
{
	// What about using a QHash instead of QList for storing our markers ?
	// Then this function would be as simple as return m_markers.value(id);
	// And most likely faster too :-)
	// On the other hand, QList concumes less memory, and when marker count
	// keeps below a certain value (< 100 or so), it perhaps doesn't make much sense
	// The get_state() would use the foreach macro, as used below, to make things real easy ;-)
	
	foreach(Marker* marker, m_markers) {
		if (marker->get_id() == id) {
			return marker;
		}
	}
	
	return 0;
}

bool TimeLine::get_end_position(TimeRef& pos)
{
	foreach(Marker* marker, m_markers) {
		if (marker->get_type() == Marker::ENDMARKER) {
			pos = marker->get_when();
			return true;
		}
	}

	return false;
}

bool TimeLine::has_end_marker()
{
	foreach(Marker* marker, m_markers) {
		if (marker->get_type() == Marker::ENDMARKER) {
			return true;
		}
	}

	return false;
}

