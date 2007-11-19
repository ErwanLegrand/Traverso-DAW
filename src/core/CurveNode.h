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

$Id: CurveNode.h,v 1.9 2007/11/19 11:18:53 r_sijrier Exp $
*/

#ifndef CURVENODE_H
#define CURVENODE_H

#include "APILinkedList.h"

class Curve;

class CurveNode : public APILinkedListNode
{

public:
	CurveNode(){};
	CurveNode(Curve* curve, double when, double  val)
		: m_curve(curve)
	{
		coeff[0] = coeff[1] = coeff[2] = coeff[3] = 0.0;
	
		this->when = when;
		this->value = val;
	}

	~CurveNode(){};
	
	void set_when(double when) {
		this->when = when;
	}
	
	void set_when_and_value(double when, double value);
	
	void set_relative_when_and_value(double relwhen, double value);
	
	double get_when() const {return when;}
	double get_value() const {return value;}
	
	bool is_smaller_then(APILinkedListNode* node) {return ((CurveNode*)node)->when > when;}

	Curve*	m_curve;
	
	// declaring friend class Curve seems not to make any difference 
	// when compiling on windows ? (not allowed to access compile error)
	double 	when;
	double 	value;
	
private:
	double  coeff[4];
/*	double 	when;
	double 	value;*/
	
	friend class Curve;
};


#endif
