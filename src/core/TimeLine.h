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
#include "defines.h"

class Song;
class Marker;
class Command;

class TimeLine : public ContextItem
{
	Q_OBJECT
public:
	TimeLine(Song* song);
	~TimeLine() {};
	
	QDomNode get_state(QDomDocument doc);
	int set_state(const QDomNode& node);
	
	QList<Marker*> get_markers() const {return m_markers;}
	Song *get_song() const {return m_song;}
	
	Marker* get_marker(qint64 id);
	bool get_end_position(nframes_t &pos);

	Command* add_marker(Marker* marker, bool historable=true);
	Command* remove_marker(Marker* marker, bool historable=true);

private:
	Song* m_song;
	QList<Marker*> m_markers;

private slots:
	void private_add_marker(Marker* marker);
	void private_remove_marker(Marker* marker);

signals:
	void markerAdded(Marker*);
	void markerRemoved(Marker*);
};

#endif

//eof
