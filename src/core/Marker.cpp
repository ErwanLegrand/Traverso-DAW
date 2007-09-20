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
#include "Song.h"

#include "TimeLine.h"
#include "Utils.h"

Marker::Marker(TimeLine* tl, nframes_t when, Type type)
	: ContextItem()
	, Snappable()
	, m_timeline(tl)
	, m_when(when)
	, m_type(type)
{
	set_history_stack(m_timeline->get_history_stack());
	m_id = create_id();

	set_snap_list(m_timeline->get_song()->get_snap_list());

	m_description = "";
	m_performer = "";
	m_composer = "";
	m_arranger = "";
	m_message = "";
	m_isrc = "";
	m_preemph = 0;
	m_copyprotect = 0;
}

Marker::Marker(TimeLine * tl, const QDomNode node)
	: ContextItem()
	, Snappable()
	, m_timeline(tl)
{
	set_snap_list(m_timeline->get_song()->get_snap_list());
	set_history_stack(m_timeline->get_history_stack());
	set_state(node);
}

QDomNode Marker::get_state(QDomDocument doc)
{
	QDomElement domNode = doc.createElement("Marker");
	
	domNode.setAttribute("position",  m_when.universal_frame());
	domNode.setAttribute("description",  m_description);
	domNode.setAttribute("id",  m_id);
	domNode.setAttribute("performer", m_performer);
	domNode.setAttribute("composer", m_composer);
	domNode.setAttribute("songwriter", m_songwriter);
	domNode.setAttribute("arranger", m_arranger);
	domNode.setAttribute("message", m_message);
	domNode.setAttribute("isrc", m_isrc);
	domNode.setAttribute("preemphasis", m_preemph);
	domNode.setAttribute("copyprotection", m_copyprotect);

	switch (m_type) {
		case CDTRACK:
			domNode.setAttribute("type",  "CDTRACK");
			break;
		case ENDMARKER:
			domNode.setAttribute("type",  "ENDMARKER");
			break;
		case TEMP_CDTRACK:
			domNode.setAttribute("type",  "TEMP_CDTRACK");
			break;
		case TEMP_ENDMARKER:
			domNode.setAttribute("type",  "TEMP_ENDMARKER");
			break;
	}

	return domNode;
}

int Marker::set_state(const QDomNode & node)
{
	QDomElement e = node.toElement();

	m_description = e.attribute("description", "");
	QString tp = e.attribute("type", "CDTRACK");
	bool ok;
	m_when = e.attribute("position", "0").toLongLong(&ok);
	m_id = e.attribute("id", "0").toLongLong();
	m_performer = e.attribute("performer", "");
	m_composer = e.attribute("composer", "");
	m_songwriter = e.attribute("songwriter", "");
	m_arranger = e.attribute("arranger", "");
	m_message = e.attribute("message", "");
	m_isrc = e.attribute("isrc", "");
	m_preemph = e.attribute("preemphasis", "0").toInt();
	m_copyprotect = e.attribute("copyprotection", "0").toInt();

	if (tp == "CDTRACK") m_type = CDTRACK;
	if (tp == "ENDMARKER") m_type = ENDMARKER;
	if (tp == "TEMP_CDTRACK") m_type = TEMP_CDTRACK;
	if (tp == "TEMP_ENDMARKER") m_type = TEMP_ENDMARKER;

	return 1;
}

void Marker::set_when(const TimeRef& when)
{
	m_when = when;
	emit positionChanged(this);
}

void Marker::set_description(const QString &s)
{
	m_description = s;
	emit descriptionChanged();
}

void Marker::set_performer(const QString &s)
{
	m_performer = s;
}

void Marker::set_composer(const QString &s)
{
	m_composer = s;
}

void Marker::set_songwriter(const QString &s)
{
	m_songwriter = s;
}

void Marker::set_arranger(const QString &s)
{
	m_arranger = s;
}

void Marker::set_message(const QString &s)
{
	m_message = s;
}

void Marker::set_isrc(const QString &s)
{
	m_isrc = s;
}

void Marker::set_preemphasis(bool b)
{
	m_preemph = b;
}

void Marker::set_copyprotect(bool b)
{
	m_copyprotect = b;
}

bool Marker::get_preemphasis()
{
	return m_preemph;
}

bool Marker::get_copyprotect()
{
	return m_copyprotect;
}

//eof
