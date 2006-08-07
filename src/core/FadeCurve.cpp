/*
Copyright (C) 2006 Remon Sijrier, Nicola Doebelin

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

$Id: FadeCurve.cpp,v 1.5 2006/08/07 21:32:27 r_sijrier Exp $
*/
 
#include "FadeCurve.h"

#include <QFile>
#include <cmath>

QStringList FadeCurve::defaultShapes = QStringList() << "Fastest" << "Fast" << "Linear"  << "Slow" << "Slowest";

FadeCurve::FadeCurve(QString type )
	: Curve(), m_sType(type)
{
	if (type == "FadeIn") {
		m_type = FadeIn;
	}
	if (type == "FadeOut") {
		m_type = FadeOut;
	}
		
	m_controlPoints.append(QPointF(0.0, 0.0));
	m_controlPoints.append(QPointF(0.25, 0.25));
	m_controlPoints.append(QPointF(0.75, 0.75));
	m_controlPoints.append(QPointF(1.0, 1.0));
	
	m_bendFactor = 0.5;
	m_strenghtFactor = 0.5;
	m_mode = m_raster = 0;
	m_bypass = false;
	
	connect(this, SIGNAL(stateChanged()), this, SLOT(solve_node_positions()));
	connect(this, SIGNAL(bendValueChanged()), this, SIGNAL(stateChanged()));
	connect(this, SIGNAL(strengthValueChanged()), this, SIGNAL(stateChanged()));
	connect(this, SIGNAL(modeChanged()), this, SIGNAL(stateChanged()));
}

FadeCurve::~ FadeCurve( )
{
}

QDomNode FadeCurve::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement(m_sType);
	node.setAttribute("bendfactor", m_bendFactor);
	node.setAttribute("strengthfactor", m_strenghtFactor);
	node.setAttribute("bypassed", m_bypass);
	node.setAttribute("range", get_range());
	node.setAttribute("mode", m_mode);
	node.setAttribute("raster", m_raster);
	
	QStringList controlPointsList;
	
	for (int i=0; i< m_controlPoints.size(); ++i) {
		QPointF point = m_controlPoints.at(i);
		
		controlPointsList << QString::number(point.x()).append(",").append(QString::number(point.y()));
	}
	
	node.setAttribute("controlpoints",  controlPointsList.join(";"));
	
	return node;
}

int FadeCurve::set_state( const QDomNode & node )
{
	QDomElement e = node.toElement();
	m_bendFactor = e.attribute( "bendfactor", "0.5" ).toDouble();
	m_strenghtFactor = e.attribute( "strengthfactor", "0.5" ).toDouble();
	m_bypass = e.attribute( "bypassed", "0" ).toInt();
	m_mode = e.attribute( "mode", "0" ).toInt();
	m_raster = e.attribute( "raster", "0" ).toInt();
	
	QStringList controlPointsList = e.attribute( "controlpoints", "0.0,0.0;0.25,0.25;0.75,0.75;1.0,1.0" ).split(";");
	
	for (int i=0; i<controlPointsList.size(); ++i) {
		QStringList xyList = controlPointsList.at(i).split(",");
		float x = xyList.at(0).toFloat();
		float y = xyList.at(1).toFloat();
		m_controlPoints[i] = QPointF(x,y);
	}
	
	
	// Populate the curve with 12 CurveNodes
	float f = 0.0;
	int nodecount = 11;
 	for (int i = 0; i <= nodecount; ++i) {
		QPointF p = get_curve_point(f);
		
		CurveNode* node = new CurveNode(this, p.x(), p.y());
		nodes.append(node);
		connect(node, SIGNAL(positionChanged()), this, SLOT(set_changed()));
		
// 		printf("adding node with x=%f, y=%f\n", p.x(), p.y());
		
		f += 1.0 / nodecount;
	}

	double range = e.attribute("range", "1").toDouble();
	range = (range == 0.0) ? 1 : range;
	set_range(range);
	
	return 1;
}

void FadeCurve::set_shape(QString shapeName)
{
	QDomDocument doc("FadeShapes");
	
	if (defaultShapes.contains(shapeName)) {
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
	} else {
		// Load from custom saved fades
	}
	

	QDomElement root = doc.documentElement();
	QDomNode node = root.firstChildElement(m_sType);
	
	QDomElement fadeElement = node.firstChildElement(shapeName);
	
	if (fadeElement.isNull()) {
		printf("%s does not exist?????\n", shapeName.toAscii().data());
		return;
	}
	
	set_bend_factor(fadeElement.attribute( "bendfactor", "0.5" ).toDouble());
	set_strength_factor(fadeElement.attribute( "strengthfactor", "0.5" ).toDouble());
	
	QStringList controlPointsList = fadeElement.attribute( "controlpoints", "" ).split(";");
	
	for (int i=0; i<controlPointsList.size(); ++i) {
		QStringList xyList = controlPointsList.at(i).split(",");
		float x = xyList.at(0).toFloat();
		float y = xyList.at(1).toFloat();
		m_controlPoints[i] = QPointF(x,y);
	}
	
	emit stateChanged();
}

void FadeCurve::solve_node_positions( )
{
	// calculate control points values
	if (m_mode == 0) {
		m_controlPoints[1] = QPointF(m_strenghtFactor * (1.0 - m_bendFactor), m_strenghtFactor * m_bendFactor);
		m_controlPoints[2] = QPointF(1.0 - (m_strenghtFactor * m_bendFactor), 1.0 - (m_strenghtFactor * (1.0 - m_bendFactor)));
	}
	if (m_mode == 1) {
		m_controlPoints[1] = QPointF(m_strenghtFactor * (1.0 - m_bendFactor), m_strenghtFactor * m_bendFactor);
		m_controlPoints[2] = QPointF(1.0 - (m_strenghtFactor * (1.0 - m_bendFactor)), 1.0 - (m_strenghtFactor * m_bendFactor));
	}


	// calculate curve nodes values
	float f = 0.0;
 	for (int i = 1; i < (nodes.size() -1); i++) {
		f += 1.0/ (nodes.size() - 1);
		
		CurveNode* node = nodes.at(i);
		QPointF p = get_curve_point(f);
		
		node->set_relative_when(p.x());
		node->set_value(p.y());
		
	}
}

QPointF FadeCurve::get_curve_point( float f)
{
	float x = m_controlPoints.at(0).x() * pow((1.0 - f), 3.0)
		+ 3 * m_controlPoints.at(1).x() * f * pow((1.0 - f), 2.0)
		+ 3 * m_controlPoints.at(2).x() * pow(f, 2.0) * (1.0 - f)
		+ m_controlPoints.at(3).x() * pow(f, 3.0);

	float y = m_controlPoints.at(0).y() * pow((1.0 - f), 3.0)
		+ 3 * m_controlPoints.at(1).y() * f * pow((1.0 - f), 2.0)
		+ 3 * m_controlPoints.at(2).y() * pow(f, 2.0) * (1.0 - f)
		+ m_controlPoints.at(3).y() * pow(f, 3.0);

	if (m_type == FadeOut) {
		y = 1.0 - y;
	}
	
	return QPointF(x, y);
}


void FadeCurve::set_bend_factor( float factor )
{
	if (factor > 1.0)
		factor = 1.0;
	if (factor < 0.0)
		factor = 0.0;
		
	m_bendFactor = factor;
	
	emit bendValueChanged();
}

void FadeCurve::set_strength_factor( float factor )
{
	if (factor > 1.0)
		factor = 1.0;
	if (factor < 0.0)
		factor = 0.0;
	
	m_strenghtFactor = factor;
	
	emit strengthValueChanged();
}

QList< QPointF > FadeCurve::get_control_points( )
{
	return m_controlPoints;
}

Command* FadeCurve::set_mode( )
{
	if (m_mode < 1) {
		m_mode++;
	} else {
		m_mode = 0;
	}

	emit modeChanged();
	return 0;
}

Command * FadeCurve::reset( )
{
	set_bend_factor(0.5);
	set_strength_factor(0.5);
	
	return 0;
}

Command * FadeCurve::toggle_bypass( )
{
	m_bypass = !m_bypass;
	
	emit stateChanged();
	return 0;
}

Command * FadeCurve::toggle_raster( )
{
	m_raster = ! m_raster;
	
	emit rasterChanged();
	return 0;
}


//eof
