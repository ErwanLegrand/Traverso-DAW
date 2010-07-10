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


#ifndef TBUS_TRACK_H
#define TBUS_TRACK_H

#include "Track.h"

class TBusTrack : public Track
{
        Q_OBJECT

public:
        TBusTrack(Sheet* sheet, const QString& name, int channelCount);
        TBusTrack(Sheet* sheet, const QDomNode node);
        ~TBusTrack();

        QDomNode get_state(QDomDocument doc, bool istemplate=false);
        virtual int set_state( const QDomNode & node );
        int process(nframes_t nframes);

protected:
        int m_channelCount;

private:
        void create_process_bus();
};

class MasterOutSubGroup : public TBusTrack
{
        Q_OBJECT

public:
        MasterOutSubGroup(Sheet* sheet, const QString& name) : TBusTrack(sheet, name, 2) {}
        MasterOutSubGroup(Sheet* sheet, const QDomNode node) : TBusTrack(sheet, node) {}
        ~MasterOutSubGroup() {}

//        void add_output_bus(const QString& name) {
//                // this should not happen, but just in case...
//                if (!(name == tr("Sheet Master")) || !(name == tr("Master"))) {
//                        Track::add_output_bus(name);
//                } else {
//                        // try to be 'smart' and pick a sane default.
//                        Track::add_output_bus("Playback 1");
//                }
//        }

        int set_state( const QDomNode& node ) {
                TBusTrack::set_state(node);

                // force proper values for the following parameters that
                // are fixed for Master Buses:
                m_channelCount = 2;
                return 1;
        }

};

#endif // TBUS_TRACK_H
