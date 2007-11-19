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

$Id: FadeCurve.cpp,v 1.31 2007/11/19 11:18:53 r_sijrier Exp $
*/
 
#include "FadeCurve.h"

#include <QFile>
#include <cmath>
#include "Song.h"
#include "Fade.h"
#include "AudioClip.h"
#include "Command.h"
#include "CommandGroup.h"
#include <AddRemove.h>
#include "AudioDevice.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


QStringList FadeCurve::defaultShapes = QStringList() << "Fastest" << "Fast" << "Linear"  << "Slow" << "Slowest";

FadeCurve::FadeCurve(AudioClip* clip, Song* song, QString type )
	: Curve(clip)
	, m_clip(clip)
	, m_sType(type)
{
	Q_UNUSED(song);
	
	if (type == "FadeIn") {
		m_type = FadeIn;
	}
	if (type == "FadeOut") {
		m_type = FadeOut;
	}
	
	m_song = m_clip->get_song();
		
	m_controlPoints.append(QPointF(0.0, 0.0));
	m_controlPoints.append(QPointF(0.25, 0.25));
	m_controlPoints.append(QPointF(0.75, 0.75));
	m_controlPoints.append(QPointF(1.0, 1.0));
	
	m_bendFactor = 0.5;
	m_strenghtFactor = 1;
	m_mode = 2;
	m_raster = 0;
	m_bypass = false;
	
	init();
	
	connect(this, SIGNAL(stateChanged()), this, SLOT(solve_node_positions()));
	connect(this, SIGNAL(bendValueChanged()), this, SIGNAL(stateChanged()));
	connect(this, SIGNAL(strengthValueChanged()), this, SIGNAL(stateChanged()));
	connect(this, SIGNAL(modeChanged()), this, SIGNAL(stateChanged()));
}

FadeCurve::~ FadeCurve( )
{
	PENTERDES;
}

void FadeCurve::init()
{
	// Populate the curve with 12 CurveNodes
	float f = 0.0;
	int nodecount = 11;
 	for (int i = 0; i <= nodecount; ++i) {
		QPointF p = get_curve_point(f);
		
		CurveNode* node = new CurveNode(this, p.x(), p.y());
		AddRemove* cmd = (AddRemove*) add_node(node, false);
		cmd->set_instantanious(true);
		Command::process_command(cmd);
		
// 		printf("adding node with x=%f, y=%f\n", p.x(), p.y());
		
		f += 1.0 / nodecount;
	}
	
	set_range(1.0);
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
	
	
	QString rangestring = e.attribute("range", "1");
	double range;
	if (rangestring == "nan" || rangestring == "inf") {
		printf("FadeCurve::set_state: stored range was not a number!\n");
		range = 1;
	} else {
		range = rangestring.toDouble();
	}
	
	range = (range < 1.0) ? 1 : range;
	
	set_range(range);
	
	solve_node_positions();	
	
	emit stateChanged();
	
	return 1;
}


void FadeCurve::process(audio_sample_t** mixdown, nframes_t nframes, uint channels)
{

	if (is_bypassed() || (get_range() < 16)) {
		return;
	}
	
	
	TimeRef faderange = TimeRef(get_range());
	TimeRef fadepos;
	
	if (m_type == FadeIn) {
		if( !( m_song->get_transport_location() < (m_clip->get_track_start_location() + faderange) ) ) {
			return;
		}
		
		fadepos = m_song->get_transport_location() - m_clip->get_track_start_location();
	} else {
		if( !(m_song->get_transport_location() > (m_clip->get_track_end_location() - faderange)) ) {
			return;
		}
		
		fadepos = m_song->get_transport_location() - (m_clip->get_track_end_location() - faderange);
	}

	TimeRef range(nframes, audiodevice().get_sample_rate());
	TimeRef limit(std::min (range.universal_frame(), faderange.universal_frame()));
	
	nframes_t framerange = limit.to_frame(audiodevice().get_sample_rate());
	
	get_vector(fadepos.universal_frame(), (fadepos + limit).universal_frame(), m_song->gainbuffer, framerange);

	for (uint chan=0; chan<channels; ++chan) {
		for (nframes_t frame = 0; frame < framerange; ++frame) {
			mixdown[chan][frame] *= m_song->gainbuffer[frame];
		}
	}
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
	
	CommandGroup* group = new CommandGroup(this, tr("Fade Shape"));
	
	group->add_command(new FadeBend(this, fadeElement.attribute( "bendfactor", "0.5" ).toDouble()));
	group->add_command(new FadeStrength(this, fadeElement.attribute( "strengthfactor", "0.5" ).toDouble()));
	
	Command::process_command(group);
	
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
// 	printf("FadeCurve::solve_node_positions()\n");
	// calculate control points values
	if (m_mode == 0) { // bended
		if (m_type == FadeIn) {
			m_controlPoints[1] = QPointF(m_strenghtFactor * (1.0 - m_bendFactor), m_strenghtFactor * m_bendFactor);
			m_controlPoints[2] = QPointF(1.0 - (m_strenghtFactor * m_bendFactor), 1.0 - (m_strenghtFactor * (1.0 - m_bendFactor)));
		}
		if (m_type == FadeOut) {
			m_controlPoints[1] = QPointF(m_strenghtFactor * m_bendFactor, m_strenghtFactor * (1.0 - m_bendFactor));
			m_controlPoints[2] = QPointF(1.0 - (m_strenghtFactor * (1.0 - m_bendFactor)), 1.0 - (m_strenghtFactor * m_bendFactor));
		}
	}
	if (m_mode == 1) { // s-shape
		m_controlPoints[1] = QPointF(m_strenghtFactor * (1.0 - m_bendFactor), m_strenghtFactor * m_bendFactor);
		m_controlPoints[2] = QPointF(1.0 - (m_strenghtFactor * (1.0 - m_bendFactor)), 1.0 - (m_strenghtFactor * m_bendFactor));
	}
	if (m_mode == 2) { // long
		if (m_type == FadeIn) {
			m_controlPoints[1] = QPointF(m_strenghtFactor * (1.0 - m_bendFactor), m_strenghtFactor * m_bendFactor);
			m_controlPoints[2] = QPointF(1.0, 1.0);
		}
		if (m_type == FadeOut) {
			m_controlPoints[1] = QPointF(0.0, 0.0);
			m_controlPoints[2] = QPointF(1.0 - (m_strenghtFactor * (1.0 - m_bendFactor)), 1.0 - (m_strenghtFactor * m_bendFactor));
		}
	}


	// calculate curve nodes values
	float f = 0.0;
	APILinkedList list = get_nodes();
	int listsize = list.size();
	if (listsize > 0) {
		APILinkedListNode* node = list.first()->next;
		
		while (node) {
			f += 1.0 / (listsize - 1);
			QPointF p = get_curve_point(f);
			((CurveNode*)node)->set_relative_when_and_value(p.x(), p.y());
			node = node->next;
		}
	}
	
	set_changed();
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

void FadeCurve::set_range(double when)
{
	Curve::set_range(when);
	emit rangeChanged();
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
	if (m_mode < 2) {
		return new FadeMode(this, m_mode, m_mode+1);
	} else {
		return new FadeMode(this, m_mode, 0);
	}
}

void FadeCurve::set_mode(int m)
{
	m_mode = m;

	emit modeChanged();
}

Command* FadeCurve::reset( )
{
	set_bend_factor(0.5);
	set_strength_factor(0.5);
	
	return 0;
}

Command* FadeCurve::toggle_bypass( )
{
	m_bypass = !m_bypass;
	
	emit stateChanged();
	return 0;
}

Command* FadeCurve::toggle_raster( )
{
	m_raster = ! m_raster;
	
	emit rasterChanged();
	return 0;
}

