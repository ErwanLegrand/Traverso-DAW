/*
Copyright (C) 2010 Remon Sijrier

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

#include "TSend.h"

#include "AudioBus.h"
#include "Project.h"
#include "ProjectManager.h"
#include "Utils.h"


TSend::TSend()
{
        m_type = POSTSEND;
}

TSend::TSend(AudioBus* bus)
{
        m_bus = bus;
        m_id = create_id();
        m_type = POSTSEND;
}


QDomNode TSend::get_state( QDomDocument doc)
{
        QDomElement node = doc.createElement("Send");

        node.setAttribute("id", m_id);
        node.setAttribute("bus", m_bus->get_id());
        node.setAttribute("busname", m_bus->get_name());
        if (m_type == POSTSEND) {
                node.setAttribute("type", QString("post"));
        } else {
                node.setAttribute("type", QString("pre"));
        }

        return node;
}


int TSend::set_state( const QDomNode & node )
{
        QDomElement e = node.toElement();

        m_id = e.attribute("id", "0").toLongLong();
        qint64 busId = e.attribute("bus", "0").toLongLong();
        QString type = e.attribute("type", "");

        if (type == "post" || type.isEmpty() || type.isNull()) {
                m_type = POSTSEND;
        } else if (type == "ppre") {
                m_type = PRESEND;
        } else {
                // default to post send if no type was stored
                m_type = POSTSEND;
        }

        Project* project = pm().get_project();
        if (!project) {
                return -1;
        }

        m_bus = project->get_bus(busId);


        return 1;
}

QString TSend::get_name() const
{
        return m_bus->get_name();
}
