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

#include "Marker.h"

#include "TimeLine.h"
#include "Utils.h"

Marker::Marker(TimeLine* tl, nframes_t when, uint type)
	: ContextItem()
	, m_timeline(tl)
	, m_when(when) 
	, m_type(type)
{
	set_history_stack(m_timeline->get_history_stack());
	m_id = create_id();
}

Marker::Marker(TimeLine * tl, const QDomNode node)
	: ContextItem()
	, m_timeline(tl)
{
	set_history_stack(m_timeline->get_history_stack());
	set_state(node);
}

QDomNode Marker::get_state(QDomDocument doc)
{
	QDomElement domNode = doc.createElement("Marker");
	
	domNode.setAttribute("position",  m_when);
	domNode.setAttribute("description",  m_description);
	domNode.setAttribute("type",  m_type);
	domNode.setAttribute("id",  m_id);

	return domNode;
}

int Marker::set_state(const QDomNode & node)
{
	QDomElement e = node.toElement();

	m_description = e.attribute("description", "");
	m_type = e.attribute("type", "0").toUInt();
	m_when = e.attribute("position", "0").toUInt();
	m_id = e.attribute("id", "0").toLongLong();

	return 1;
}

void Marker::set_when(nframes_t when)
{
	m_when = when;
	emit positionChanged();
}

void Marker::set_description(const QString & des)
{
	m_description = des;
}

QString Marker::get_description() const
{
	return m_description;
}

//eof
