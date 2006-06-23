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
 
    $Id: Curve.cpp,v 1.7 2006/06/23 11:22:39 r_sijrier Exp $
*/

#include "Curve.h"
#include <cmath>


#include "Song.h"
#include "Track.h"
#include "CurveNode.h"

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

using namespace std;


Curve::Curve()
		: ContextItem()
{
        PENTERCONS;
        changed = true;
        defaultValue = 1.0f;
	lookup_cache.left = -1;
	lookup_cache.range.first = nodes.end();
}

Curve::~Curve()
{
	foreach(CurveNode* node, nodes) {
		delete node;
	}
}

void Curve::add_node(double pos, double value)
{
        PENTER2;
        nodes.append(new CurveNode(pos, value) );
        emit stateChanged();
}

void Curve::solve ()
{
	uint32_t npoints;

	if (!changed) {
		return;
	}
	
	if ((npoints = nodes.size()) > 2) {
		
		/* Compute coefficients needed to efficiently compute a constrained spline
		   curve. See "Constrained Cubic Spline Interpolation" by CJC Kruger
		   (www.korf.co.uk/spline.pdf) for more details.
		*/

		double x[npoints];
		double y[npoints];
		uint32_t i;
		QList<CurveNode* >::iterator xx;

		for (i = 0, xx = nodes.begin(); xx != nodes.end(); ++xx, ++i) {
			x[i] = (double) (*xx)->when;
			y[i] = (double) (*xx)->value;
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

		for (i = 0, xx = nodes.begin(); xx != nodes.end(); ++xx, ++i) {
			
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

			cn->coeff[0] = y[i-1] - (b * x[i-1]) - (c * xim12) - (d * xim13);
			cn->coeff[1] = b;
			cn->coeff[2] = c;
			cn->coeff[3] = d;

			fplast = fpi;
		}
		
	}

	changed = false;
}


void Curve::get_vector (double x0, double x1, float *vec, int32_t veclen)
{
	double rx, dx, lx, hx, max_x, min_x;
	int32_t i;
	int32_t original_veclen;
	int32_t npoints;

	if ((npoints = nodes.size()) == 0) {
		for (i = 0; i < veclen; ++i) {
			vec[i] = defaultValue;
		}
		return;
	}

	/* nodes is now known not to be empty */

	max_x = nodes.back()->when;
	min_x = nodes.front()->when;

	lx = max (min_x, x0);

	if (x1 < 0) {
		x1 = nodes.back()->when;
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
			vec[i] = nodes.front()->value;
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

		val = nodes.back()->value;

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
 			vec[i] = nodes.front()->value;
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
 	
 		double slope = (nodes.back()->value - nodes.front()->value)/  
			(nodes.back()->when - nodes.front()->when);
 		double yfrac = dx*slope;
 
 		vec[0] = nodes.front()->value + slope * (lx - nodes.front()->when);
 
 		for (i = 1; i < veclen; ++i) {
 			vec[i] = vec[i-1] + yfrac;
 		}
 
 		return;
 	}
 
	if (changed) {
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

double Curve::multipoint_eval (double x)
{	
	std::pair<QList<CurveNode* >::iterator, QList<CurveNode* >::iterator> range;

	if ((lookup_cache.left < 0) ||
	    ((lookup_cache.left > x) || 
	     (lookup_cache.range.first == nodes.end()) || 
	     ((*lookup_cache.range.second)->when < x))) {
		
		Comparator cmp;
		CurveNode cn (x, 0.0);

		lookup_cache.range = equal_range (nodes.begin(), nodes.end(), &cn, cmp);
	}

	range = lookup_cache.range;

	/* EITHER 
	   
	   a) x is an existing control point, so first == existing point, second == next point

	   OR

	   b) x is between control points, so range is empty (first == second, points to where
	       to insert x)
	   
	*/

	if (range.first == range.second) {

		/* x does not exist within the list as a control point */
		
		lookup_cache.left = x;

		if (range.first == nodes.begin()) {
			/* we're before the first point */
			// return defaultValue;
			nodes.front()->value;
		}
		
		if (range.second == nodes.end()) {
			/* we're after the last point */
			return nodes.back()->value;
		}

		double x2 = x * x;
		CurveNode* cn = *range.second;

		return cn->coeff[0] + (cn->coeff[1] * x) + (cn->coeff[2] * x2) + (cn->coeff[3] * x2 * x);
	} 

	/* x is a control point in the data */
	/* invalidate the cached range because its not usable */
	lookup_cache.left = -1;
	return (*range.first)->value;
}

void Curve::set_range( double when )
{
	if (nodes.isEmpty() || nodes.last()->when == when) {
		return;
	}
	
	Q_ASSERT(when >= 0);
	
	double factor = when / nodes.last()->when;
	x_scale (factor);
	
	emit stateChanged();
}

void Curve::x_scale( double factor )
{
	foreach(CurveNode* node, nodes) {
		node->when = node->when * factor;
	}
	
// 	printf("Range is now %d\n", (int) nodes.last()->when);

	set_changed();
}

void Curve::set_changed( )
{
	lookup_cache.left = -1;
	changed = true;
}

double Curve::get_range( ) const
{
	if (nodes.isEmpty()) {
		return 0;
	}
	
	if (nodes.last()) {
		return nodes.last()->when;
	}
		
	return 0;
}

void Curve::clear( )
{
	nodes.clear();
	set_changed();
}

//eof
