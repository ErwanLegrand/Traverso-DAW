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

$Id: CurveNode.cpp,v 1.1 2006/05/10 11:02:03 r_sijrier Exp $
*/

#include "CurveNode.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


CurveNode::CurveNode(double pos, double  val)
{
	PENTERCONS;
	coeff[0] = coeff[1] = coeff[2] = coeff[3] = 0.0;
	
	when = pos;
	value = val;
}

CurveNode::~CurveNode()
{
	PENTERDES;
}


//eof
