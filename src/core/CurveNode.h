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

$Id: CurveNode.h,v 1.6 2007/01/24 21:18:30 r_sijrier Exp $
*/

#ifndef CURVENODE_H
#define CURVENODE_H

#include "defines.h"

#include "ContextItem.h"
#include "Debugger.h"
class Curve;

class CurveNode : public ContextItem
{

	Q_OBJECT

public:
	CurveNode(Curve* curve, double pos, double  val)
		: m_curve(curve)
	{
		rtCoeff[0] = rtCoeff[1] = rtCoeff[2] = rtCoeff[3] = 0.0;
		guiCoeff[0] = guiCoeff[1] = guiCoeff[2] = guiCoeff[3] = 0.0;
	
		m_when = pos;
		m_value = val;
	}

	~CurveNode(){};
	
	void set_when(double when) {m_when = when;}
	
	void set_when_and_value(double when, double value) {
		m_when = when;
		m_value = value;
		emit positionChanged();
	}
	
	void set_relative_when_and_value(double relwhen, double value);
	
	double get_when() const {return m_when;}
	double get_value() const {return m_value;}

	double rtCoeff[4];
	double guiCoeff[4];
	
private:
	Curve*	m_curve;
	double 	m_when;
	double 	m_value;
	
	
signals:
	void positionChanged();
};


#endif
