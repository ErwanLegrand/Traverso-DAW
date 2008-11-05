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

#ifndef TIME_LINE_H
#define TIME_LINE_H

#include "ContextItem.h"
#include <QDomNode>
#include <QList>
#include "defines.h"

class Sheet;
class Marker;
class Snappable;
class Command;

class TimeLine : public ContextItem
{
	Q_OBJECT
public:
	TimeLine(Sheet* sheet);
	~TimeLine() {};
	
	QDomNode get_state(QDomDocument doc);
	int set_state(const QDomNode& node);
	
	QList<Marker*> get_markers() const {return m_markers;}
	Sheet *get_sheet() const {return m_sheet;}
	
	Marker* get_marker(qint64 id);
	Marker* get_end_marker();
	bool get_end_location(TimeRef& location);
	bool get_start_location(TimeRef& location);
	bool has_end_marker();

	Command* add_marker(Marker* marker, bool historable=true);
	Command* remove_marker(Marker* marker, bool historable=true);

private:
	Sheet* m_sheet;
	QList<Marker*> m_markers;
	void index_markers();

private slots:
	void private_add_marker(Marker* marker);
	void private_remove_marker(Marker* marker);
	void marker_position_changed();

signals:
	void markerAdded(Marker*);
	void markerRemoved(Marker*);
	void markerPositionChanged();
};

#endif

//eof
