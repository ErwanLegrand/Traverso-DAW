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
#include <AddRemoveItemCommand.h>


TimeLine::TimeLine(Song * song)
	: ContextItem(song)
	, m_song(song)
{
	set_history_stack(m_song->get_history_stack());
}

QDomNode TimeLine::get_state(QDomDocument doc)
{
	QDomElement domNode = doc.createElement("TimeLine");
	
	// TODO
	// This function, as well as the set_state() doesn't make much sense
	// although it works, it's better to add a get/set_state() function in Marker
	// and add a xml node Markers, and add all Marker nodes as childs.
	// For an example see Song::get/set_state() starting at:
	// QDomNode tracksNode = node.firstChildElement("Tracks");
	// and
	// QDomNode tracksNode = doc.createElement("Tracks");
	// This way, you can set/get whatever property you like for the Marker class
	// If it makes sense to have different lists of markers of a certain type,
	// well, I'll leave that to you :-)
	// You could for example have different xml nodes for the different marker types
	// or just one xml node, and use Marker::type() to sort on Marker type later in get_state()
	
	QStringList markerList;
	
	for (int i=0; i < m_markers.size(); ++i) {
		markerList << QString::number(m_markers.at(i)->get_when());
	}
	
	domNode.setAttribute("markers",  markerList.join(";"));
	
	return domNode;
}

int TimeLine::set_state(const QDomNode & node)
{
	QDomElement e = node.toElement();
	
	QStringList markerList = e.attribute( "markers", "" ).split(";");
	
	for (int i=0; i<markerList.size(); ++i) {
		nframes_t when = markerList.at(i).toUInt();
		Marker* marker =  new Marker(this, when);
		private_add_marker(marker);
	}
	
	return 1;
}


Command * TimeLine::add_marker(Marker* marker, bool historable)
{
	AddRemoveItemCommand* cmd;
	cmd = new AddRemoveItemCommand(this, marker, historable, m_song,
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
	AddRemoveItemCommand* cmd;
	cmd = new AddRemoveItemCommand(this, marker, historable, m_song,
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

//eof

