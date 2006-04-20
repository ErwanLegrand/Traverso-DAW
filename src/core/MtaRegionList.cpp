/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: MtaRegionList.cpp,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#include "MtaRegionList.h"
#include "MtaRegion.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

MtaRegionList::MtaRegionList()
{
        list = (MtaRegion*) 0;
}

MtaRegionList::~MtaRegionList()
{
        // TODO delete all regions
}

int MtaRegionList::add_regions(QStringList regions)
{
        PENTER2;
        for ( QStringList::Iterator it = regions.begin(); it != regions.end(); ++it ) {
                QString regionDescription = *it;
                if (add_region(regionDescription)<0) {
                        return -1;
                }
        }
        return 1;
}


int MtaRegionList::add_region(QString regionDescription)
{
        PENTER2;
        if (regionDescription.indexOf("<region ")>=0) {
                int x5 = regionDescription.indexOf("begin=")+6;
                int x6 = regionDescription.indexOf(" ", x5+1);
                QString sbb=regionDescription.mid(x5,x6-x5);
                int x7 = regionDescription.indexOf("end=")+4;
                int x8 = regionDescription.indexOf(" ", x7+1);
                QString sbe=regionDescription.mid(x7,x8-x7);
                nframes_t bb = (nframes_t) sbb.toDouble();
                nframes_t be = (nframes_t) sbe.toDouble();
                MtaRegion* m = new MtaRegion(bb,be);
                add_region(m);
                return 1;
        }
        return -1;
}


void MtaRegionList::add_region(MtaRegion* m)
{
        PENTER;
        if (!list) {
                list=m;
                m->prev=0;
                m->next=0;
        } else {
                MtaRegion* r = list;
                MtaRegion* rx = 0;
                MtaRegion* rlast;
                while (r) {
                        rlast=r;
                        if (r->beginBlock > m->beginBlock) {
                                rx=r;
                                break;
                        }
                        r=r->next;
                }
                if (rx) {
                        m->prev=rx->prev;
                        m->next=rx;
                        if (rx->prev)
                                rx->prev->next=m;
                        rx->prev=m;
                } else {
                        rlast->next=m;
                        m->prev=rlast;
                        m->next=0;
                }
                if (!m->prev)
                        list=m;
        }
}

MtaRegion* MtaRegionList::head()
{
        return list;
}
