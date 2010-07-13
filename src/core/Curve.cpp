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


#include "Sheet.h"
#include "Track.h"
#include "InputEngine.h"
#include "Utils.h"
#include <QStringList>
#include <QThread>
#include <AddRemove.h>
#include <Command.h>
#include <CommandGroup.h>
#include "Mixer.h"
#include "Information.h"

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
	APILinkedListNode* node = m_nodes.first();
	while (node) {
		CurveNode* q = (CurveNode*)node;
		node = node->next;
		delete q;
	}
}

void Curve::init( )
{
	QObject::tr("Curve");
	QObject::tr("CurveNode");
	m_changed = true;
	m_lookup_cache.left = -1;
	m_defaultValue = 1.0f;
        m_session = 0;
	
	connect(this, SIGNAL(nodePositionChanged()), this, SLOT(set_changed()));
}


QDomNode Curve::get_state(QDomDocument doc, const QString& name)
{
	PENTER3;
	QDomElement domNode = doc.createElement(name);
	
	QStringList nodesList;
	
	apill_foreach(CurveNode* cn, CurveNode, m_nodes) {
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

int Curve::process(
	audio_sample_t** buffer,
	const TimeRef& startlocation,
	const TimeRef& endlocation,
	nframes_t nframes,
	uint channels,
	float makeupgain
	)
{
	// Do nothing if there are no nodes!
	if (m_nodes.isEmpty()) {
		return 0;
	}
	
	// Check if we are beyond the last node and only apply gain if != 1.0
	if (endlocation > qint64(get_range())) {
		float gain = ((CurveNode*)m_nodes.last())->value * makeupgain;
		
		if (gain == 1.0f) {
			return 0;
		}
		
		for (uint chan=0; chan<channels; ++chan) {
			Mixer::apply_gain_to_buffer(buffer[chan], nframes, gain);
		}
		
		return 1;
	}
	
	// Calculate the vector, an apply to the buffer including the makeup gain.
        get_vector(startlocation.universal_frame(), endlocation.universal_frame(), m_session->mixdown, nframes);
	
	for (uint chan=0; chan<channels; ++chan) {
		for (nframes_t n = 0; n < nframes; ++n) {
                        buffer[chan][n] *= (m_session->mixdown[n] * makeupgain);
		}
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

		CurveNode* cn;
		i = 0;
		for(APILinkedListNode* node = m_nodes.first(); node!=0; node = node->next, ++i) {
			cn = (CurveNode*)node;
			x[i] = cn->when;
			y[i] = cn->value;
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

		i = 0;
		for(APILinkedListNode* node = m_nodes.first(); node!=0; node = node->next, ++i) {
			
			cn = (CurveNode*)node;
			
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

				if ((slope_after * slope_before) < 0.0) {
					/* slope m_changed sign */
					fpi = 0.0;
				} else {
					fpi = 2 / (slope_before + slope_after);
				}
				
			}

			/* compute second derivative for either side of control point `i' */
			double fractal = ((6 * ydelta) / xdelta2); // anyone knows a better name for it?
			fppL = (((-2 * (fpi + (2 * fplast))) / (xdelta))) + fractal;
			fppR = (2 * ((2 * fpi) + fplast) / xdelta) - fractal;
			
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
	
	CurveNode* lastnode = (CurveNode*)m_nodes.last();
	CurveNode* firstnode = (CurveNode*)m_nodes.first();

	max_x = lastnode->when;
	min_x = firstnode->when;

	lx = max (min_x, x0);

	if (x1 < 0) {
		x1 = lastnode->when;
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

//                printf("filling first %d samples %f\n", subveclen, firstnode->value);

		for (i = 0; i < subveclen; ++i) {
			vec[i] = firstnode->value;
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

		val = lastnode->value;

		i = veclen - subveclen;

//                printf("filling last %d samples %f\n", subveclen, firstnode->value);

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
			vec[i] = firstnode->value;
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
	
		double slope = (lastnode->value - firstnode->value) /
			(lastnode->when - firstnode->when );
		double yfrac = dx*slope;

		vec[0] = firstnode->value + slope * (lx - firstnode->when);

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
	if ((m_lookup_cache.left < 0) ||
		((m_lookup_cache.left > x) || (m_lookup_cache.range.second->when < x))) {
		
		CurveNode cn (this, x, 0.0);

		APILinkedListNode* first = m_nodes.first();
		APILinkedListNode* last = m_nodes.last();
		APILinkedListNode* middle;
		bool validrange = false;
		int len = m_nodes.size() - 1;
		int half;
		
		while (len > 0) {
			half = len >> 1;
			middle = first;
			// advance middle by half
			int n = half;
			while (n--) {
				middle = middle->next;
			}
			//start compare
			if (((CurveNode*)middle)->when < cn.when) {
				first = middle;
				first = first->next;
				len = len - half - 1;
			} else if (cn.when < ((CurveNode*)middle)->when) {
				len = half;
			} else {
				// lower bound (using first/middle)
				APILinkedListNode* lbfirst = first;
				APILinkedListNode* lblast = middle;
				APILinkedListNode* lbmiddle;
				// start distance.
				int lblen = 0;
				APILinkedListNode* temp = lbfirst;
				while (temp != lblast) {
					temp = temp->next;
					++lblen;
				}
				
				int lbhalf;
				while (lblen > 0) {
					lbhalf = lblen >> 1;
					lbmiddle = lbfirst;
					// advance middle by half
					n = lbhalf;
					while (n--) {
						lbmiddle = lbmiddle->next;
					}
					if (((CurveNode*)lbmiddle)->when < cn.when) {
						lbfirst = lbmiddle;
						lbfirst = lbfirst->next;
						lblen = lblen - lbhalf - 1;
					} else {
						lblen = lbhalf;
					}
				}
				m_lookup_cache.range.first = (CurveNode*)lbfirst;
				

				// upper bound
				APILinkedListNode* ubfirst = middle->next;
				APILinkedListNode* ublast = last;
				APILinkedListNode* ubmiddle;
				// start distance.
				int ublen = 0;
				temp = ubfirst;
				while (temp != ublast) {
					temp = temp->next;
					++ublen;
				}
				
				int ubhalf;
				while (ublen > 0) {
					ubhalf = ublen >> 1;
					ubmiddle = ubfirst;
					// advance middle by half
					n = ubhalf;
					while (n--) {
						ubmiddle = ubmiddle->next;
					}
					
					if (cn.when < ((CurveNode*)ubmiddle)->when) {
						ublen = ubhalf;
					} else {
						ubfirst = ubmiddle;
						ubfirst = ubfirst->next;
						ublen = ublen - ubhalf - 1;
					}
				}
				m_lookup_cache.range.second = (CurveNode*)ubfirst;
				
				validrange = true;
				break;
			}
		}
		if (!validrange) {
			m_lookup_cache.range.first = (CurveNode*)first;
			m_lookup_cache.range.second = (CurveNode*)first;
		}
	}
	
	/* EITHER 
	
	a) x is an existing control point, so first == existing point, second == next point

	OR

	b) x is between control points, so range is empty (first == second, points to where
	to insert x)
	
	*/

	if (m_lookup_cache.range.first == m_lookup_cache.range.second) {

		/* x does not exist within the list as a control point */
		
		m_lookup_cache.left = x;

		double x2 = x * x;
		CurveNode* cn = m_lookup_cache.range.second;
		
		return cn->coeff[0] + (cn->coeff[1] * x) + (cn->coeff[2] * x2) + (cn->coeff[3] * x2 * x);
	} 

	/* x is a control point in the data */
	/* invalidate the cached range because its not usable */
	m_lookup_cache.left = -1;
	return m_lookup_cache.range.first->value;
}

void Curve::set_range(double when)
{
	if (m_nodes.isEmpty()) {
		printf("Curve::set_range: no nodes!!");
		return;
	}
	
	CurveNode* lastnode = (CurveNode*)m_nodes.last();
	
	if (lastnode->when == when) {
// 		printf("Curve::set_range: new range == current range!\n");
		return;
	}
	
	if (when < 0.0 ) {
		printf("Curve::set_range: error, when < 0.0 !  (%f)\n", when);
		return;
	}
	
	Q_ASSERT(when >= 0.0);
	
	double factor = when / lastnode->when;
	
	if (factor == 1.0)
		return;
	
	x_scale (factor);
	
	set_changed();
}

void Curve::x_scale(double factor)
{
	Q_ASSERT(factor != 0.0);
	
	apill_foreach(CurveNode* node, CurveNode, m_nodes) {
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
	
	apill_foreach(CurveNode* cn, CurveNode, m_nodes) {
		if (node->when == cn->when && node->value == cn->value) {
			info().warning(tr("There is allready a node at this exact position, not adding a new node"));
			delete node;
			return 0;
		}
	}

	
	AddRemove* cmd;
        cmd = new AddRemove(this, node, historable, m_session,
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
	
	if (m_nodes.first() == node) {
		MoveNode* cmd;

		cmd = new MoveNode(this, node, 0.0f, 1.0f, tr("Remove CurveNode"));

		return cmd;
	}

	
	AddRemove* cmd;
	
        cmd = new AddRemove(this, node, historable, m_session,
			"private_remove_node(CurveNode*)", "nodeRemoved(CurveNode*)", 
			"private_add_node(CurveNode*)", "nodeAdded(CurveNode*)", 
   			tr("Remove CurveNode"));
			
	return cmd;
}

void Curve::private_add_node( CurveNode * node )
{
	m_nodes.add_and_sort(node);
	set_changed();
}

void Curve::private_remove_node( CurveNode * node )
{
	m_nodes.remove(node);
	set_changed();
}

void Curve::set_sheet(TSession * sheet)
{
        m_session = sheet;
        set_history_stack(m_session->get_history_stack());
}

