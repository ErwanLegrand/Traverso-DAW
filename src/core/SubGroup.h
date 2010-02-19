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


#ifndef SUBGROUP_H
#define SUBGROUP_H

#include "Track.h"

class SubGroup : public Track
{
        Q_OBJECT

public:
        SubGroup(Sheet* sheet, const QString& name, int channelCount);
        SubGroup(Sheet* sheet, const QDomNode node);
        ~SubGroup();

        QDomNode get_state(QDomDocument doc, bool istemplate=false);
        int set_state( const QDomNode & node );
        void set_height(int h);
        int process(nframes_t nframes);

private:
        void init();

        int m_channelCount;
};

#endif // SUBGROUP_H
