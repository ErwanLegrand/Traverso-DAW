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
 
    $Id: MtaRegion.h,v 1.1 2006/04/20 14:51:39 r_sijrier Exp $
*/

#ifndef MTAREGION_H
#define MTAREGION_H

#include "defines.h"

class MtaRegion
{
public :
        MtaRegion(nframes_t begin, nframes_t end);
        ~MtaRegion();

        nframes_t beginBlock;
        nframes_t endBlock;

        MtaRegion* next;
        MtaRegion* prev;
};

#endif
