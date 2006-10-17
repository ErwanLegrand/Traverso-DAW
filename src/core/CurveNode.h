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

$Id: CurveNode.h,v 1.4 2006/10/17 00:07:43 r_sijrier Exp $
*/

#ifndef CURVENODE_H
#define CURVENODE_H

#include "defines.h"

#include <QObject>

class Curve;

class CurveNode : public QObject
{

	Q_OBJECT

public:
	CurveNode(Curve* curve, double pos, double  val)
		: m_curve(curve)
	{
		coeff[0] = coeff[1] = coeff[2] = coeff[3] = 0.0;
	
		m_when = pos;
		m_value = val;
	}

	~CurveNode(){};
	
	void set_relative_when(double when);
	void set_value(double value);

	void set_when(double when) {m_when = when;}
	
	double get_when() const {return m_when;}
	double get_value() const {return m_value;}

	double coeff[4];
	
private:
	Curve*	m_curve;
	double 	m_when;
	double 	m_value;
	
	
signals:
	void positionChanged();
};


#endif
