/*
Copyright (C) 2005-2006 Remon Sijrier 

Original version from Ardour curve.cc, modified
in order to fit Traverso's lockless design

Copyright (C) 2001-2003 Paul Davis 

Contains ideas derived from "Constrained Cubic Spline Interpolation" 
by CJC Kruger (www.korf.co.uk/spline.pdf).

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

*/

#include "Curve.h"
#include <cmath>


#include "Song.h"
#include "Track.h"
#include "InputEngine.h"
#include "Utils.h"
#include <QStringList>
#include <QThread>
#include <AddRemove.h>
#include <Command.h>
#include <CommandGroup.h>
#include "Mixer.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

using namespace std;


class MoveNode : public Command
{

public:
	MoveNode(Curve* curve, CurveNode* node, double when, double val, const QString& des);
	
	int prepare_actions();
	int do_action();
        int undo_action();

private :
	CurveNode*	m_node;
	double		m_origWhen;
	double		m_origVal;
	double		m_newWhen;
	double		m_newVal;
};

	
MoveNode::MoveNode(Curve* curve, CurveNode* node, double when, double val, const QString& des)
	: Command(curve, des)
{
	m_node = node;
	m_origWhen = m_node->get_when();
	m_origVal = m_node->get_value();
	m_newWhen = when;
	m_newVal = val;
}

int MoveNode::prepare_actions()
{
	return 1;
}

int MoveNode::do_action()
{
	m_node->set_when_and_value(m_newWhen, m_newVal);
	return 1;
}

int MoveNode::undo_action()
{
	m_node->set_when_and_value(m_origWhen, m_origVal);
	return 1;
}



Curve::Curve(ContextItem* parent)
	: ContextItem(parent)
{
	PENTERCONS;
	m_id = create_id();
	init();
}

Curve::Curve(ContextItem* parent, const QDomNode node )
	: ContextItem(parent)
{
	init();
	set_state(node);
}

Curve::~Curve()
{
	foreach(CurveNode* node, m_nodes) {
		delete node;
	}
}

void Curve::init( )
{
	m_changed = true;
	m_lookup_cache.left = -1;
	m_lookup_cache.range.first = m_nodes.end();

	m_defaultValue = 1.0f;
	m_defaultInitialValue = 1.0f;	
	
	m_song = 0;
	
	connect(this, SIGNAL(nodePositionChanged()), this, SLOT(set_changed()));
}


QDomNode Curve::get_state(QDomDocument doc, const QString& name)
{
	PENTER3;
	QDomElement domNode = doc.createElement(name);
	
	QStringList nodesList;
	
	for (int i=0; i < m_nodes.size(); ++i) {
		CurveNode* cn = m_nodes.at(i);
		nodesList << QString::number(cn->when, 'g', 24).append(",").append(QString::number(cn->value));
	}
	
	if (m_nodes.size() == 0) {
		nodesList << "1," + QString::number(m_defaultValue);
	}
	
	domNode.setAttribute("nodes",  nodesList.join(";"));
	domNode.setAttribute("defaulvalue",  m_defaultValue);
	domNode.setAttribute("id",  m_id);
	
	
	return domNode;
}

int Curve::set_state( const QDomNode & node )
{
	PENTER;
	QDomElement e = node.toElement();
	
	QStringList nodesList = e.attribute( "nodes", "" ).split(";");
	m_defaultValue = e.attribute( "defaulvalue", "1.0" ).toDouble();
	m_id = e.attribute("id", "0" ).toLongLong();
	if (m_id == 0) {
		m_id = create_id();
	}
	
	for (int i=0; i<nodesList.size(); ++i) {
		QStringList whenValueList = nodesList.at(i).split(",");
		double when = whenValueList.at(0).toDouble();
		double value = whenValueList.at(1).toDouble();
		CurveNode* node = new CurveNode(this, when, value);
		private_add_node(node);
	}
	
	return 1;
}


int Curve::process(audio_sample_t* buffer, nframes_t pos, nframes_t nframes)
{
	if (m_nodes.isEmpty()) {
		return 0;
	}
	
	if ((pos + nframes) > get_range()) {
		if (m_nodes.last()->value == 1.0) {
			return 0;
		}
		Mixer::apply_gain_to_buffer(buffer, nframes, m_nodes.last()->value);
		return 1;
	}
	
	audio_sample_t mixdown[nframes];
	
	get_vector(pos, pos + nframes, mixdown, nframes);
	
	for (nframes_t n = 0; n < nframes; ++n) {
		buffer[n] *= mixdown[n];
	}
	
	return 1;
}


void Curve::solve ()
{
	uint32_t npoints;

	if (!m_changed) {
		printf("Curve::solve, no data change\n");
		return;
	}
	
	if ((npoints = m_nodes.size()) > 2) {
		
		/* Compute coefficients needed to efficiently compute a constrained spline
		curve. See "Constrained Cubic Spline Interpolation" by CJC Kruger
		(www.korf.co.uk/spline.pdf) for more details.
		*/

		double x[npoints];
		double y[npoints];
		uint32_t i;
		QList<CurveNode* >::iterator xx;

		for (i = 0, xx = m_nodes.begin(); xx != m_nodes.end(); ++xx, ++i) {
			x[i] = (double) (*xx)->when;
			y[i] = (double) (*xx)->value;
		}

		double lp0, lp1, fpone;

		lp0 =(x[1] - x[0])/(y[1] - y[0]);
		lp1 = (x[2] - x[1])/(y[2] - y[1]);

		if ( (lp0*lp1) < 0 ) {
			fpone = 0;
		} else {
			fpone = 2 / (lp1 + lp0);
		}

		double fplast = 0;

		for (i = 0, xx = m_nodes.begin(); xx != m_nodes.end(); ++xx, ++i) {
			
			CurveNode* cn = dynamic_cast<CurveNode*>(*xx);

			if (cn == 0) {
/*				qCritical  << _("programming error: ")
				<< X_("non-CurvePoint event found in event list for a Curve")
				<< endmsg;*/
				/*NOTREACHED*/
			}
			
			double xdelta;   /* gcc is wrong about possible uninitialized use */
			double xdelta2;  /* ditto */
			double ydelta;   /* ditto */
			double fppL, fppR;
			double fpi;

			if (i > 0) {
				xdelta = x[i] - x[i-1];
				xdelta2 = xdelta * xdelta;
				ydelta = y[i] - y[i-1];
			}

			/* compute (constrained) first derivatives */
			
			if (i == 0) {

				/* first segment */
				
				fplast = ((3 * (y[1] - y[0]) / (2 * (x[1] - x[0]))) - (fpone * 0.5));

				/* we don't store coefficients for i = 0 */

				continue;

			} else if (i == npoints - 1) {

				/* last segment */

				fpi = ((3 * ydelta) / (2 * xdelta)) - (fplast * 0.5);
				
			} else {

				/* all other segments */

				double slope_before = ((x[i+1] - x[i]) / (y[i+1] - y[i]));
				double slope_after = (xdelta / ydelta);

				if (slope_after * slope_before < 0.0) {
					/* slope m_changed sign */
					fpi = 0.0;
				} else {
					fpi = 2 / (slope_before + slope_after);
				}
				
			}

			/* compute second derivative for either side of control point `i' */
			
			fppL = (((-2 * (fpi + (2 * fplast))) / (xdelta))) +
				((6 * ydelta) / xdelta2);
			
			fppR = (2 * ((2 * fpi) + fplast) / xdelta) -
				((6 * ydelta) / xdelta2);
			
			/* compute polynomial coefficients */

			double b, c, d;

			d = (fppR - fppL) / (6 * xdelta);   
			c = ((x[i] * fppL) - (x[i-1] * fppR))/(2 * xdelta);
			
			double xim12, xim13;
			double xi2, xi3;
			
			xim12 = x[i-1] * x[i-1];  /* "x[i-1] squared" */
			xim13 = xim12 * x[i-1];   /* "x[i-1] cubed" */
			xi2 = x[i] * x[i];        /* "x[i] squared" */
			xi3 = xi2 * x[i];         /* "x[i] cubed" */
			
			b = (ydelta - (c * (xi2 - xim12)) - (d * (xi3 - xim13))) / xdelta;

			/* store */
			
			cn->coeff[0] = y[i-1] - (b * x[i-1]) - (c * xim12) - (d * xim13);
			cn->coeff[1] = b;
			cn->coeff[2] = c;
			cn->coeff[3] = d;

			fplast = fpi;
		}
		
	}

	m_changed = false;
}


void Curve::get_vector (double x0, double x1, float *vec, int32_t veclen)
{
	double rx, dx, lx, hx, max_x, min_x;
	int32_t i;
	int32_t original_veclen;
	int32_t npoints;
	
	if ((npoints = m_nodes.size()) == 0) {
		for (i = 0; i < veclen; ++i) {
			vec[i] = m_defaultValue;
		}
		return;
	}

	/* nodes is now known not to be empty */

	max_x = m_nodes.back()->when;
	min_x = m_nodes.front()->when;

	lx = max (min_x, x0);

	if (x1 < 0) {
		x1 = m_nodes.back()->when;
	}

	hx = min (max_x, x1);
	
	
	original_veclen = veclen;

	if (x0 < min_x) {

		/* fill some beginning section of the array with the 
		initial (used to be default) value 
		*/

		double frac = (min_x - x0) / (x1 - x0);
		int32_t subveclen = (int32_t) floor (veclen * frac);
		
		subveclen = min (subveclen, veclen);

		for (i = 0; i < subveclen; ++i) {
			vec[i] = m_nodes.front()->value;
		}

		veclen -= subveclen;
		vec += subveclen;
	}

	if (veclen && x1 > max_x) {

		/* fill some end section of the array with the default or final value */

		double frac = (x1 - max_x) / (x1 - x0);

		int32_t subveclen = (int32_t) floor (original_veclen * frac);

		float val;
		
		subveclen = min (subveclen, veclen);

		val = m_nodes.back()->value;

		i = veclen - subveclen;

		for (i = veclen - subveclen; i < veclen; ++i) {
			vec[i] = val;
		}

		veclen -= subveclen;
	}

	if (veclen == 0) {
		return;
	}

	if (npoints == 1 ) {
	
		for (i = 0; i < veclen; ++i) {
			vec[i] = m_nodes.front()->value;
		}
		return;
	}


	if (npoints == 2) {

		/* linear interpolation between 2 points */

		/* XXX I'm not sure that this is the right thing to
		do here. but its not a common case for the envisaged
		uses.
		*/
	
		if (veclen > 1) {
			dx = (hx - lx) / (veclen - 1) ;
		} else {
			dx = 0; // not used
		}
	
		double slope = (m_nodes.back()->value - m_nodes.front()->value) /
			(m_nodes.back()->when - m_nodes.front()->when );
		double yfrac = dx*slope;

		vec[0] = m_nodes.front()->value + slope * (lx - m_nodes.front()->when);

		for (i = 1; i < veclen; ++i) {
			vec[i] = vec[i-1] + yfrac;
		}

		return;
	}

	if (m_changed) {
		solve ();
	}

	rx = lx;

	if (veclen > 1) {

		dx = (hx - lx) / veclen;

		for (i = 0; i < veclen; ++i, rx += dx) {
			vec[i] = multipoint_eval (rx);
		}
	}
}

double Curve::multipoint_eval(double x)
{	
	std::pair<QList<CurveNode* >::iterator, QList<CurveNode* >::iterator> range;

	
	if ((m_lookup_cache.left < 0) ||
		((m_lookup_cache.left > x) || 
		(m_lookup_cache.range.first == m_nodes.end()) || 
		((*m_lookup_cache.range.second)->when < x))) {
		
		Comparator cmp;
		CurveNode cn (this, x, 0.0);

		m_lookup_cache.range = equal_range (m_nodes.begin(), m_nodes.end(), &cn, cmp);
	}

	range = m_lookup_cache.range;

	/* EITHER 
	
	a) x is an existing control point, so first == existing point, second == next point

	OR

	b) x is between control points, so range is empty (first == second, points to where
	to insert x)
	
	*/

	if (range.first == range.second) {

		/* x does not exist within the list as a control point */
		
		m_lookup_cache.left = x;

		if (range.first == m_nodes.begin()) {
			/* we're before the first point */
			// return m_defaultValue;
			m_nodes.front()->value;
		}
		
		if (range.second == m_nodes.end()) {
			/* we're after the last point */
			return m_nodes.back()->value;
		}

		double x2 = x * x;
		CurveNode* cn = *range.second;
		
		
		return cn->coeff[0] + (cn->coeff[1] * x) + (cn->coeff[2] * x2) + (cn->coeff[3] * x2 * x);
	} 

	/* x is a control point in the data */
	/* invalidate the cached range because its not usable */
	m_lookup_cache.left = -1;
	return (*range.first)->value;
}

void Curve::set_range(double when)
{
	if (m_nodes.isEmpty()) {
		printf("Curve::set_range: no nodes!!");
		return;
	}
	if (m_nodes.last()->when == when) {
// 		printf("Curve::set_range: new range == current range!\n");
		return;
	}
	
	if (when < 0.0 ) {
		printf("Curve::set_range: error, when < 0.0 !  (%f)\n", when);
		return;
	}
	
	Q_ASSERT(when >= 0.0);
	
	double factor = when / m_nodes.last()->when;
	
	if (factor == 1.0)
		return;
	
	x_scale (factor);
	
	set_changed();
}

void Curve::x_scale(double factor)
{
	Q_ASSERT(factor != 0.0);
	
	for (int i=0; i < m_nodes.size(); ++i) {
		CurveNode* node = m_nodes.at(i); 
		node->set_when(node->when * factor);
	}
}

void Curve::set_changed( )
{
	m_lookup_cache.left = -1;
	m_changed = true;
}


/**
 * Add a new Node to this Curve.
 * 
 * The returned Command object can be placed on the history stack,
 * to make un-redo possible (the default (??) when called from the InputEngine)
 *
 * Note: This function should only be called from the GUI thread!
 * 
 * @param node CurveNode to add to this Curve
 * @param historable Should the returned Command object be placed on the
 		history stack?
 * @return A Command object, if the call was generated from the InputEngine,
 	it can be leaved alone, if it was a direct call, use Command::process_command()
 	to do the actuall work!!
 */
Command* Curve::add_node(CurveNode* node, bool historable)
{
	PENTER2;
	
	AddRemove* cmd;
	cmd = new AddRemove(this, node, historable, m_song,
			"private_add_node(CurveNode*)", "nodeAdded(CurveNode*)",
			"private_remove_node(CurveNode*)", "nodeRemoved(CurveNode*)", 
			tr("Add CurveNode"));
	
	return cmd;
}


/**
 * Remove a  Node from this Curve.
 * 
 * The returned Command object can be placed on the history stack,
 * to make un-redo possible (the default (??) when called from the InputEngine)
 *
 * Note: This function should only be called from the GUI thread!
 * 
 * @param node CurveNode to be removed from this Curve
 * @param historable Should the returned Command object be placed on the
 		history stack?
 * @return A Command object, if the call was generated from the InputEngine,
 	it can be leaved alone, if it was a direct call, use Command::process_command()
 	to do the actuall work!!
 */
Command* Curve::remove_node(CurveNode* node, bool historable)
{
	PENTER2;
	
	if (m_nodes.size() == 1) {
		MoveNode* cmd;

		cmd = new MoveNode(this, node, 0.0f, m_defaultInitialValue, tr("Remove CurveNode"));

		return cmd;
	}

	
	AddRemove* cmd;
	
	cmd = new AddRemove(this, node, historable, m_song,
			"private_remove_node(CurveNode*)", "nodeRemoved(CurveNode*)", 
			"private_add_node(CurveNode*)", "nodeAdded(CurveNode*)", 
   			tr("Remove CurveNode"));
			
	return cmd;
}

void Curve::private_add_node( CurveNode * node )
{
	m_nodes.append(node);
	qSort(m_nodes.begin(), m_nodes.end(), smallerNode);
	
	set_changed();
}

void Curve::private_remove_node( CurveNode * node )
{
	m_nodes.removeAll(node);
	set_changed();
}

void Curve::set_song(Song * song)
{
	m_song = song; 
	set_history_stack(m_song->get_history_stack());
}
