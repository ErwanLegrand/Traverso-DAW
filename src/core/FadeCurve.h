/*
Copyright (C) 2006 Remon Sijrier 

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

$Id: FadeCurve.h,v 1.2 2006/08/03 15:14:26 r_sijrier Exp $
*/

#ifndef FADE_CURVE_H
#define FADE_CURVE_H

#include "Curve.h"

#include <QString>

class FadeCurve : public Curve
{
	Q_OBJECT
	
public:
	FadeCurve(QString type);
	~FadeCurve();
	
	enum FadeShape {
		Fastest,
		Fast,
		Linear,
		Slow,
		Slowest
	};
	
	enum CurveMode {
		None,
		Logarithmic,
		Exponential
	};
	
	QDomNode get_state(QDomDocument doc);
	int set_state( const QDomNode & node );
	
	float get_bend_factor() {return m_bendFactor;}
	float get_strenght_factor() {return m_strenghtFactor;}
	
	void set_shape(FadeShape shape);
	void set_shape(QString customShape);
	void set_bend_factor(float factor);
	void set_strength_factor(float factor);
	
	QString get_fade_type() const {return m_type;}
	
	bool is_bypassed() const {return m_bypass;}

private:
	float 		m_bendFactor;
	float 		m_strenghtFactor;
	bool		m_bypass;
	QString		m_type;
	FadeShape	m_shape;
	CurveMode	m_curvemode;
	
public slots:
	Command* toggle_bypass();
};

#endif

//eof

