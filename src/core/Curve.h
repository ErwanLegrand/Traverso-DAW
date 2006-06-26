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

$Id: Curve.h,v 1.4 2006/06/26 23:57:48 r_sijrier Exp $
*/

#ifndef CURVE_H
#define CURVE_H

#include "ContextItem.h"
#include <QString>
#include <QList>

#include "CurveNode.h"


class Curve : public ContextItem
{
	Q_OBJECT
	
public:
	Curve();
	~Curve();

	void add_node(double pos, double value);
	void clear();
	void x_scale(double factor);
	void solve ();
	void get_vector (double x0, double x1, float *arg, int32_t veclen);
	
	void set_range(double pos);
	
	double get_range() const;

	QList<CurveNode* >	nodes;

private :
	
	struct LookupCache {
	    double left;  /* leftmost x coordinate used when finding "range" */
	    std::pair<QList<CurveNode* >::iterator, QList<CurveNode* >::iterator> range;
	};
	
	struct Comparator {
		bool operator() (const CurveNode* a, const CurveNode* b) { 
			return a->when < b->when;
		}
	};
	
	LookupCache lookup_cache;
	
	bool changed;
	double defaultValue;
	
	double multipoint_eval (double x);
	void set_changed();
	
	friend class CurveNode;

private slots:
	void thread_save_add_node(QObject* node);
	void thread_save_clear(QObject* obj);

signals :
	void stateChanged();
};


#endif
