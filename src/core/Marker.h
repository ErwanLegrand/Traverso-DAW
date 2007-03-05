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


#ifndef MARKER_H
#define MARKER_H

#include "ContextItem.h"
#include "defines.h"
#include <QDomNode>

class TimeLine;

class Marker : public ContextItem
{
	Q_OBJECT
	
public:
	Marker(TimeLine* tl, nframes_t when);
	~Marker() {};
	
	QDomNode get_state(QDomDocument doc);
	int set_state(const QDomNode& node);
	
	void set_description(const QString& des);
	void set_when (nframes_t when);
	
	nframes_t get_when() const {return m_when;}
	QString get_description() const {return m_description;}
	
	
private:
	TimeLine* m_timeline;
	nframes_t m_when;
	QString m_description;
	
signals:
	void positionChanged();

};

#endif

//eof
