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

$Id: CurveNode.cpp,v 1.4 2006/10/17 00:07:43 r_sijrier Exp $
*/

#include "CurveNode.h"
#include "Curve.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


void CurveNode::set_relative_when( double when )
{
	m_when = when * m_curve->get_range();
	emit positionChanged();
}

void CurveNode::set_value( double value )
{
	m_value = value;
	emit positionChanged();
}

//eof
