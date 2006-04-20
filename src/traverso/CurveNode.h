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
 
    $Id: CurveNode.h,v 1.1 2006/04/20 14:54:03 r_sijrier Exp $
*/

#ifndef CURVENODE_H
#define CURVENODE_H

#include "defines.h"

class QWidget;
class Curve;

class CurveNode
{
public:
        CurveNode(Curve* pParentCurve, nframes_t pPos, float pValue);
        ~CurveNode();

        void toggle_highlight();
        void draw();

        void set_pos(nframes_t pPos);
        void set_value(float pValue);

        void setup();

        CurveNode* prev;
        CurveNode* next;

        bool isHighLighted;

        Curve* parentCurve;

        nframes_t pos;
        float value;

};




#endif
