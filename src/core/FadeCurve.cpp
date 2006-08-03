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

$Id: FadeCurve.cpp,v 1.2 2006/08/03 15:14:26 r_sijrier Exp $
*/
 
#include "FadeCurve.h"

#include <QFile>

FadeCurve::FadeCurve(QString type )
	: Curve(), m_type(type)
{
	m_bendFactor = 0.5;
	m_strenghtFactor = 0.5;
	m_curvemode = Logarithmic;
	m_bypass = false;
	
	set_shape( Fastest );
}

FadeCurve::~ FadeCurve( )
{
}

QDomNode FadeCurve::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement(m_type);
	node.setAttribute("bendfactor", m_bendFactor);
	node.setAttribute("strengthfactor", m_strenghtFactor);
	node.setAttribute("bypassed", m_bypass);
	
	QDomNode curvenode = Curve::get_state(doc);
	
	node.appendChild(curvenode);
	
	return node;
}

int FadeCurve::set_state( const QDomNode & node )
{
	QDomElement e = node.toElement();
	m_bendFactor = e.attribute( "bendfactor", "0.5" ).toDouble();
	m_strenghtFactor = e.attribute( "strengthfactor", "0.5" ).toDouble();
	m_bypass = e.attribute( "bypassed", "0" ).toInt();
	
	QDomElement curveElement = node.firstChildElement("Curve");
	Curve::set_state(curveElement);
	
	return 1;
}

void FadeCurve::set_shape( FadeShape shape )
{
	QDomDocument doc("FadeInShapes");
	QFile file(":/fadeshapes");
	
	if (!file.open(QIODevice::ReadOnly)) {
		printf("Could not open fadeshapes file!!\n");
		return;
	}
	if (!doc.setContent(&file)) {
		file.close();
		printf("Could not set QDomDocument content!\n");
		return;
	}
	file.close();

	QDomElement root = doc.documentElement();
	QDomNode node = root.firstChild();
	
	QDomElement fadeElement = node.firstChildElement("Slow");
	if (fadeElement.isNull()) {
		printf("Fastest does not exist\n");
		return;
	}
	
	Curve::set_state(fadeElement);
	
	m_shape = shape;
}

void FadeCurve::set_shape( QString customShape )
{
	// load custom shape
}

void FadeCurve::set_bend_factor( float factor )
{
	if (factor > 1.0)
		factor = 1.0;
	if (factor < 0.0)
		factor = 0.0;
		
	m_bendFactor = factor;
	emit stateChanged();
}

void FadeCurve::set_strength_factor( float factor )
{
	if (factor > 1.0)
		factor = 1.0;
	if (factor < 0.0)
		factor = 0.0;
	
	m_strenghtFactor = factor;
	emit stateChanged();
}

Command * FadeCurve::toggle_bypass( )
{
	m_bypass = !m_bypass;
	return 0;
}

//eof
