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

#include "TSession.h"
#include "Marker.h"
#include "Export.h"
#include "Utils.h"
#include <AddRemove.h>
#include "AudioDevice.h"

static bool smallerMarker(const Marker* left, const Marker* right )
{
	return left->get_when() < right->get_when();
}

TimeLine::TimeLine(TSession * sheet)
	: ContextItem(sheet)
	, m_sheet(sheet)
{
	QObject::tr("TimeLine");
}

QDomNode TimeLine::get_state(QDomDocument doc)
{
	QDomElement domNode = doc.createElement("TimeLine");
	QDomNode markersNode = doc.createElement("Markers");
	domNode.appendChild(markersNode);

	foreach (Marker *marker, m_markers) {
		markersNode.appendChild(marker->get_state(doc));
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
		connect(marker, SIGNAL(positionChanged()), this, SLOT(marker_position_changed()));
		m_markers.append(marker);
		markerNode = markerNode.nextSibling();
	}

	index_markers();

	return 1;
}

TCommand * TimeLine::add_marker(Marker* marker, bool historable)
{
	connect(marker, SIGNAL(positionChanged()), this, SLOT(marker_position_changed()));
	
	AddRemove* cmd;
	cmd = new AddRemove(this, marker, historable, m_sheet,
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

TCommand* TimeLine::remove_marker(Marker* marker, bool historable)
{
	AddRemove* cmd;
	cmd = new AddRemove(this, marker, historable, m_sheet,
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
	index_markers();
}

void TimeLine::private_remove_marker(Marker * marker)
{
	m_markers.removeAll(marker);
	index_markers();
}

Marker * TimeLine::get_marker(qint64 id)
{
	foreach(Marker* marker, m_markers) {
		if (marker->get_id() == id) {
			return marker;
		}
	}
	
	return 0;
}

bool TimeLine::get_end_location(TimeRef& location)
{
	foreach(Marker* marker, m_markers) {
		if (marker->get_type() == Marker::ENDMARKER) {
			location = marker->get_when();
			return true;
		}
	}

	return false;
}

bool TimeLine::get_start_location(TimeRef & location)
{
	if (m_markers.size() > 0) {
		location = m_markers.first()->get_when();
		return true;
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


Marker* TimeLine::get_end_marker()
{
	foreach(Marker* marker, m_markers) {
		if (marker->get_type() == Marker::ENDMARKER) {
			return marker;
		}
	}

	return (Marker*)0;
}

void TimeLine::marker_position_changed()
{
	index_markers();

	emit markerPositionChanged();
	
	// FIXME This is not a fix to let the sheetview scrollbars 
	// know that it's range possably has to be recalculated!!!!!!!!!!!!!!
	emit m_sheet->lastFramePositionChanged();
}

void TimeLine::index_markers()
{
	qSort(m_markers.begin(), m_markers.end(), smallerMarker);
	// let the markers know about their position (index)
	for (int i = 0; i < m_markers.size(); i++) {
		m_markers.at(i)->set_index(i+1);
	}	
}

// returns all markers of type CDTRACK
// sets 'endmarker' to true if an endmarker is present, else to false.
QList<Marker*> TimeLine::get_cd_layout(bool & endmarker)
{
        QList<Marker*> list;
        endmarker = false;

        foreach(Marker* marker, m_markers) {
                if (marker->get_type() == Marker::CDTRACK) {
                        list.append(marker);
                }

                if (marker->get_type() == Marker::ENDMARKER) {
                        list.append(marker);
                        endmarker = true;
                }
        }

        return list;
}


// formatting the track names in a separate function to guarantee that
// the file names of exported tracks and the entry in the TOC file always
// match
QString TimeLine::format_cdtrack_name(Marker *marker, int i)
{
        QString name;
        QString song = marker->get_description();
        QString performer = marker->get_performer();

        name = QString("%1").arg(i, 2, 10, QChar('0'));

        if (!performer.isEmpty()) {
                name += "-" + performer;
        }

        if (!song.isEmpty()) {
                name += "-" + song;
        }

        name.replace(QRegExp("\\s"), "_");
        return name;
}

// creates a valid list of markers for CD export. Takes care of special cases
// such as if no markers are present, or if an end marker is missing.
QList<Marker*> TimeLine::get_cdtrack_list(ExportSpecification *spec)
{
        bool endmarker;
        QList<Marker*> lst = get_cd_layout(endmarker);

        // make sure there are at least a start- and end-marker in the list
        if (lst.size() == 0) {
                lst.push_back(new Marker(this, spec->startLocation, Marker::CDTRACK));
        }

        if (!endmarker) {
                TimeRef endlocation = qMax(spec->endLocation, lst.last()->get_when());
                lst.push_back(new Marker(this, endlocation, Marker::ENDMARKER));
        }

        return lst;
}


QString TimeLine::get_cdrdao_tracklist(ExportSpecification* spec, bool pregap)
{
        QString output;

        QList<Marker*> mlist = get_cdtrack_list(spec);

//	TimeRef start;

        for(int i = 0; i < mlist.size()-1; ++i) {

                Marker* startmarker = mlist.at(i);
                Marker* endmarker = mlist.at(i+1);

                output += "TRACK AUDIO\n";

                if (startmarker->get_copyprotect()) {
                        output += "  NO COPY\n";
                } else {
                        output += "  COPY\n";
                }

                if (startmarker->get_preemphasis()) {
                        output += "  PRE_EMPHASIS\n";
                }

                output += "  CD_TEXT {\n    LANGUAGE 0 {\n";
                output += "      TITLE \"" + startmarker->get_description() + "\"\n";
                output += "      PERFORMER \"" + startmarker->get_performer() + "\"\n";
                output += "      ISRC \"" + startmarker->get_isrc() + "\"\n";
                output += "      ARRANGER \"" + startmarker->get_arranger() + "\"\n";
                output += "      SONGWRITER \"" + startmarker->get_songwriter() + "\"\n";
                output += "      MESSAGE \"" + startmarker->get_message() + "\"\n    }\n  }\n";

                // add some stuff only required for the first track (e.g. pre-gap)
                if ((i == 0) && pregap) {
                        //if (start == 0) {
                                // standard pregap, because we have a track marker at the beginning
                                output += "  PREGAP 00:02:00\n";
                        //} else {
                        //	// no track marker at the beginning, thus use the part from 0 to the first
                        //	// track marker for the pregap
                        //	output += "  START " + frame_to_cd(start, m_project->get_rate()) + "\n";
                        //	start = 0;
                        //}
                }

                TimeRef length = cd_to_timeref(timeref_to_cd(endmarker->get_when())) - cd_to_timeref(timeref_to_cd(startmarker->get_when()));

//		QString s_start = timeref_to_cd(start);
                QString s_length = timeref_to_cd(length);

//		output += "  FILE \"" + spec->name + "." + spec->extraFormat["filetype"] + "\" " + s_start + " " + s_length + "\n\n";
                output += "  FILE \"" + format_cdtrack_name(startmarker, i+1) + "." + spec->extraFormat["filetype"] + "\" 0 " + s_length + "\n\n";
//		start += length;

                // check if the second marker is of type "Endmarker"
                if (endmarker->get_type() == Marker::ENDMARKER) {
                        break;
                }
        }

        return output;
}
