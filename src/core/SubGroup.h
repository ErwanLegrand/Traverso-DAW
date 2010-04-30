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
        virtual int set_state( const QDomNode & node );
        int process(nframes_t nframes);

protected:
        int m_channelCount;

private:
        void init();
};

class MasterOutSubGroup : public SubGroup
{
        Q_OBJECT

public:
        MasterOutSubGroup(Sheet* sheet) : SubGroup(sheet, tr("Master Out"), 2) {};
        MasterOutSubGroup(Sheet* sheet, const QDomNode node) : SubGroup(sheet, node) {
                m_name = tr("Master Out");
        };
        ~MasterOutSubGroup() {};

        void set_name(const QString& /*name*/) {
                // Master out can't and shouldn't be renamed!
                m_name = tr("Master Out");
        }

        void set_output_bus(const QString& name) {
                // this should not happen, but just in case...
                if (!(name == tr("Master Out"))) {
                        ProcessingData::set_output_bus(name);
                } else {
                        // try to be 'smart' and pick a sane default.
                        ProcessingData::set_output_bus("Playback 1");
                }
        }

        int set_state( const QDomNode& node ) {
                SubGroup::set_state(node);

                // force proper values for the following parameters that
                // are fixed for master out:
                m_name = tr("Master Out");
                m_channelCount = 2;
                return 1;
        }

};

#endif // SUBGROUP_H
