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

$Id: Curve.cpp,v 1.26 2007/01/24 21:18:30 r_sijrier Exp $
*/

#include "Curve.h"
#include <cmath>


#include "Song.h"
#include "Track.h"
#include "CurveNode.h"
#include "InputEngine.h"
#include <QStringList>
#include <QThread>
#include <AddRemoveItemCommand.h>
#include <CommandGroup.h>

unsigned long threadid;

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

using namespace std;


Curve::Curve(ContextItem* parent, Song* song)
	: ContextItem(parent)
	, m_song(song)
{
	PENTERCONS;
	Q_ASSERT(m_song);
	init();
}

Curve::Curve(ContextItem* parent, Song* song, const QDomNode node )
	: ContextItem(parent)
	, m_song(song)
{
	Q_ASSERT(m_song);
	init();
	set_state(node);
}

Curve::~Curve()
{
	foreach(CurveNode* node, m_data.nodes) {
		delete node;
	}
}

void Curve::init( )
{
	threadid = QThread::currentThreadId ();
	m_rtdata.changed = true;
	m_rtdata.lookup_cache.left = -1;
	m_rtdata.lookup_cache.range.first = m_rtdata.nodes.end();
	m_rtdata.isGui = false;

	m_data.changed = true;
	m_data.lookup_cache.left = -1;
	m_data.lookup_cache.range.first = m_data.nodes.end();
	m_data.isGui = true;

	defaultValue = 1.0f;
}


QDomNode Curve::get_state( QDomDocument doc )
{
	PENTER3;
	QDomElement domNode = doc.createElement("Curve");
	
	QStringList nodesList;
	
	for (int i=0; i< m_data.nodes.size(); ++i) {
		CurveNode* cn = m_data.nodes.at(i);
		nodesList << QString::number(cn->get_when(), 'g', 24).append(",").append(QString::number(cn->get_value()));
	}
	
	if (m_data.nodes.size() == 0) {
		nodesList << "1,1";
	}
	
	domNode.setAttribute("nodes",  nodesList.join(";"));
	domNode.setAttribute("range", get_range());
	
	return domNode;
}

int Curve::set_state( const QDomNode & node )
{
	PENTER;
	QDomElement e = node.toElement();
	
	QStringList nodesList = e.attribute( "nodes", "" ).split(";");
	
	for (int i=0; i<nodesList.size(); ++i) {
		QStringList whenValueList = nodesList.at(i).split(",");
		double when = whenValueList.at(0).toDouble();
		double value = whenValueList.at(1).toDouble();
		CurveNode* node = new CurveNode(this, when, value); 
		ie().process_command( add_node(node, false) );
	}
	
	double range = e.attribute("range", "1").toDouble();
	range = (range == 0.0) ? 1 : range;
	
	set_range(range);
	
	return 1;
}

void Curve::solve (CurveData* data)
{
	printf("Curve::solve\n");
	uint32_t npoints;

	if (!data->changed) {
		printf("Curve::solve, no data change\n");
		return;
	}
	
	QList<CurveNode* >* nodes = &data->nodes;
	
	if ((npoints = nodes->size()) > 2) {
		
		/* Compute coefficients needed to efficiently compute a constrained spline
		curve. See "Constrained Cubic Spline Interpolation" by CJC Kruger
		(www.korf.co.uk/spline.pdf) for more details.
		*/

		double x[npoints];
		double y[npoints];
		uint32_t i;
		QList<CurveNode* >::iterator xx;

		for (i = 0, xx = nodes->begin(); xx != nodes->end(); ++xx, ++i) {
			x[i] = (double) (*xx)->get_when();
			y[i] = (double) (*xx)->get_value();
		}

		double lp0, lp1, fpone;

		lp0 =(x[1] - x[0])/(y[1] - y[0]);
		lp1 = (x[2] - x[1])/(y[2] - y[1]);

		if (lp0*lp1 < 0) {
			fpone = 0;
		} else {
			fpone = 2 / (lp1 + lp0);
		}

		double fplast = 0;

		for (i = 0, xx = nodes->begin(); xx != nodes->end(); ++xx, ++i) {
			
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
					/* slope changed sign */
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
			
			double* coeff;
			if (data->isGui) {
				coeff = cn->guiCoeff;
			} else {
				coeff = cn->rtCoeff;
			}

			coeff[0] = y[i-1] - (b * x[i-1]) - (c * xim12) - (d * xim13);
			coeff[1] = b;
			coeff[2] = c;
			coeff[3] = d;

			fplast = fpi;
		}
		
	}

	data->changed = false;
}


void Curve::get_vector (double x0, double x1, float *vec, int32_t veclen) {
#if defined(RT_THREAD_CHECK)
	Q_ASSERT_X(threadid == QThread::currentThreadId (), "Curve::get_vector", "Error, not accessed from GUI thread!!!!!");
#endif
	_get_vector(&m_data, x0, x1, vec, veclen);
}

void Curve::rt_get_vector (double x0, double x1, float *vec, int32_t veclen) {
	_get_vector(&m_rtdata, x0, x1, vec, veclen);
}

void Curve::_get_vector (CurveData* data, double x0, double x1, float *vec, int32_t veclen)
{
	double rx, dx, lx, hx, max_x, min_x;
	int32_t i;
	int32_t original_veclen;
	int32_t npoints;
	
	QList<CurveNode* >* nodes = &data->nodes;

	if ((npoints = nodes->size()) == 0) {
		for (i = 0; i < veclen; ++i) {
			vec[i] = defaultValue;
		}
		return;
	}

	/* nodes is now known not to be empty */

	max_x = nodes->back()->get_when();
	min_x = nodes->front()->get_when();

	lx = max (min_x, x0);

	if (x1 < 0) {
		x1 = nodes->back()->get_when();
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
			vec[i] = nodes->front()->get_value();
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

		val = nodes->back()->get_value();

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
			vec[i] = nodes->front()->get_value();
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
	
		double slope = (nodes->back()->get_value() - nodes->front()->get_value())/  
			(nodes->back()->get_when() - nodes->front()->get_when());
		double yfrac = dx*slope;

		vec[0] = nodes->front()->get_value() + slope * (lx - nodes->front()->get_when());

		for (i = 1; i < veclen; ++i) {
			vec[i] = vec[i-1] + yfrac;
		}

		return;
	}

	if (data->changed) {
		solve (data);
	}

	rx = lx;

	if (veclen > 1) {

		dx = (hx - lx) / veclen;

		for (i = 0; i < veclen; ++i, rx += dx) {
			vec[i] = multipoint_eval (data, rx);
		}
	}
}

double Curve::multipoint_eval (CurveData* data, double x)
{	
	std::pair<QList<CurveNode* >::iterator, QList<CurveNode* >::iterator> range;

	LookupCache* lookup_cache = &data->lookup_cache;
	QList<CurveNode* >* nodes = &data->nodes;
	
	if ((lookup_cache->left < 0) ||
		((lookup_cache->left > x) || 
		(lookup_cache->range.first == nodes->end()) || 
		((*lookup_cache->range.second)->get_when() < x))) {
		
		Comparator cmp;
		CurveNode cn (this, x, 0.0);

		lookup_cache->range = equal_range (nodes->begin(), nodes->end(), &cn, cmp);
	}

	range = lookup_cache->range;

	/* EITHER 
	
	a) x is an existing control point, so first == existing point, second == next point

	OR

	b) x is between control points, so range is empty (first == second, points to where
	to insert x)
	
	*/

	if (range.first == range.second) {

		/* x does not exist within the list as a control point */
		
		lookup_cache->left = x;

		if (range.first == nodes->begin()) {
			/* we're before the first point */
			// return defaultValue;
			nodes->front()->get_value();
		}
		
		if (range.second == nodes->end()) {
			/* we're after the last point */
			return nodes->back()->get_value();
		}

		double x2 = x * x;
		CurveNode* cn = *range.second;
		
		
		double* coeff;
		if (data->isGui) {
			coeff = cn->guiCoeff;
		} else {
			coeff = cn->rtCoeff;
		}

		return coeff[0] + (coeff[1] * x) + (coeff[2] * x2) + (coeff[3] * x2 * x);
	} 

	/* x is a control point in the data */
	/* invalidate the cached range because its not usable */
	lookup_cache->left = -1;
	return (*range.first)->get_value();
}

void Curve::set_range( double when )
{
	if (m_data.nodes.isEmpty()) {
		PMESG("no nodes!!");
		return;
	}
	if (m_data.nodes.last()->get_when() == when) {
		PMESG("new range == current range!");
		return;
	}
	
	Q_ASSERT(when >= 0);
	
	double factor = when / m_data.nodes.last()->get_when();
	
	x_scale (factor);
	
	set_changed();
	
	emit rangeChanged();
}

// This function is called from within the GUI thread,
// the rt-audio thread accesses the same node data,
// so the multipoint_eval() could potentially create
// slightly wrong values when scaling the curve during playback.
void Curve::x_scale(double factor )
{
	for (int i=0; i<m_data.nodes.size(); ++i) {
		CurveNode* node = m_data.nodes.at(i); 
		node->set_when(node->get_when() * factor);
	}
}

void Curve::set_changed( )
{
	m_data.lookup_cache.left = -1;
	m_data.changed = true;
	
	THREAD_SAVE_CALL(this, 0, rt_set_changed());
}

void Curve::rt_set_changed( )
{
	m_rtdata.lookup_cache.left = -1;
	m_rtdata.changed = true;
}


static bool smallerNode(const CurveNode* left, const CurveNode* right )
{
        return left->get_when() < right->get_when();
}


/**
 * Add a new Node to this Curve.
 * 
 * It'll update the CurveData for both the gui and rt-audio thread.
 * The returned Command object can be placed on the history stack,
 * to make un-redo possible (the default (??) when called from the InputEngine)
 *
 * Note: This function should only be called from the GUI thread!
 * 
 * @param node CurveNode to add to this Curve
 * @param historable Should the returned Command object be placed on the
 		history stack?
 * @return A Command object, if the call was generated from the InputEngine,
 	it can be leaved alone, if it was a direct call, use ie().process_command()
 	to do the actuall work!!
 */
Command* Curve::add_node(CurveNode* node, bool historable)
{
	PENTER2;
	// Since we have to update 2 CurveData sets, create a AddAddRemoveItemCommand
	// for each set, and place them in a CommandGroup!
	CommandGroup* group = new CommandGroup(this, tr("Add CurveNode"), historable);
	
	AddRemoveItemCommand* cmd;
	
	cmd = new AddRemoveItemCommand(this, node, historable, m_song,
			"rt_private_add_node(CurveNode*)", "",
			"rt_private_remove_node(CurveNode*)", "nodeRemoved(CurveNode*)", 
			"");
	
	group->add_command(cmd);
	
	cmd = new AddRemoveItemCommand(this, node, historable, m_song,
			"private_add_node(CurveNode*)", "nodeAdded(CurveNode*)",
			"private_remove_node(CurveNode*)", "nodeRemoved(CurveNode*)", 
			"");
	
	
	group->add_command(cmd);
	
	connect(node, SIGNAL(positionChanged()), this, SLOT(set_changed()));
	
	return group;
}


/**
 * Remove a  Node from this Curve.
 * 
 * It'll update the CurveData for both the gui and rt-audio thread.
 * The returned Command object can be placed on the history stack,
 * to make un-redo possible (the default (??) when called from the InputEngine)
 *
 * Note: This function should only be called from the GUI thread!
 * 
 * @param node CurveNode to be removed from this Curve
 * @param historable Should the returned Command object be placed on the
 		history stack?
 * @return A Command object, if the call was generated from the InputEngine,
 	it can be leaved alone, if it was a direct call, use ie().process_command()
 	to do the actuall work!!
 */
Command* Curve::remove_node(CurveNode* node, bool historable)
{
	PENTER2;
	// Since we have to update 2 CurveData sets, create a AddAddRemoveItemCommand
	// for each set, and place them in a CommandGroup!
	CommandGroup* group = new CommandGroup(this, tr("Remove CurveNode"), historable);
	
	AddRemoveItemCommand* cmd;
	cmd = new AddRemoveItemCommand(this, node, historable, m_song,
			"rt_private_remove_node(CurveNode*)", "", 
			"rt_private_add_node(CurveNode*)", "nodeAdded(CurveNode*)", 
			"");
			
	group->add_command(cmd);
	
	cmd = new AddRemoveItemCommand(this, node, historable, m_song,
			"private_remove_node(CurveNode*)", "nodeRemoved(CurveNode*)", 
			"private_add_node(CurveNode*)", "nodeAdded(CurveNode*)", 
			"");
	
	group->add_command(cmd);
			
	return group;
}

// Will be called from within the audio thread
// (dispatched from the tsar event dispatcher, by means
// of the AddRemoveAddRemoveItemCommand->do/undo_action()
void Curve::rt_private_add_node(CurveNode* node )
{
	m_rtdata.nodes.append(node);
	qSort(m_rtdata.nodes.begin(), m_rtdata.nodes.end(), smallerNode);
	
	rt_set_changed();
}

// Will be called from within the audio thread
// (dispatched from the tsar event dispatcher, by means
// of the AddRemoveAddRemoveItemCommand->do/undo_action()
void Curve::rt_private_remove_node(CurveNode* node )
{
	m_rtdata.nodes.removeAll(node);
	
	rt_set_changed();
}

// Will be called from within the audio thread
// (dispatched from the tsar event dispatcher, by means
// of the AddRemoveAddRemoveItemCommand->do/undo_action()
void Curve::private_add_node( CurveNode * node )
{
	m_data.nodes.append(node);
	qSort(m_data.nodes.begin(), m_data.nodes.end(), smallerNode);
	
	set_changed();
}

// Will be called from within the audio thread
// (dispatched from the tsar event dispatcher, by means
// of the AddRemoveAddRemoveItemCommand->do/undo_action()
void Curve::private_remove_node( CurveNode * node )
{
	m_data.nodes.removeAll(node);
	set_changed();
}


//eof
