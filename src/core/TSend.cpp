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
        m_bus = 0;
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
        if (m_bus) {
                node.setAttribute("bus", m_bus->get_id());
                node.setAttribute("busname", m_bus->get_name());
        }
        if (m_type == POSTSEND) {
                node.setAttribute("type", QString("post"));
        } else {
                node.setAttribute("type", QString("pre"));
        }

        return node;
}


int TSend::set_state( const QDomNode & node )
{
        Project* project = pm().get_project();
        if (!project) {
                printf("TSend::set_state: Oh boy, no project?? Can't restore state without a project running!!\n");
                return -1;
        }

        QDomElement e = node.toElement();

        m_id = e.attribute("id", "0").toLongLong();
        qint64 busId = e.attribute("bus", "0").toLongLong();
        QString type = e.attribute("type", "");
        QString busName = e.attribute("busname", "No Busname in Project file");

        if (type == "post" || type.isEmpty() || type.isNull()) {
                m_type = POSTSEND;
        } else if (type == "pre") {
                m_type = PRESEND;
        } else {
                // default to post send if no type was stored
                m_type = POSTSEND;
        }


        m_bus = project->get_bus(busId);

        if (!m_bus) {
                printf("TSend::set_state: Project didn't return my Bus (%s)!\n", busName.toAscii().data());
                return -1;
        }


        return 1;
}

QString TSend::get_name() const
{
        if (!m_bus) {
                return "No Bus??";
        }
        return m_bus->get_name();
}

qint64 TSend::get_bus_id() const
{
        if (!m_bus) {
                return -1;
        }

        return m_bus->get_id();
}
