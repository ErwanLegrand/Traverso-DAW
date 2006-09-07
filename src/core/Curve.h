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

$Id: Curve.h,v 1.11 2006/09/07 09:36:52 r_sijrier Exp $
*/

#ifndef CURVE_H
#define CURVE_H

#include "ContextItem.h"
#include <QString>
#include <QList>
#include <QDomDocument>

#include "CurveNode.h"


class Curve : public ContextItem
{
	Q_OBJECT
	
public:
	Curve(ContextItem* parent);
	Curve(ContextItem* parent, const QDomNode node);
	~Curve();

	virtual QDomNode get_state(QDomDocument doc);
	virtual int set_state( const QDomNode& node );
	
	void add_node(double pos, double value);
	void clear();
	
	// Get functions
	double get_range() const;
	
	void get_vector (double x0, double x1, float *arg, int32_t veclen);
	
	QList<CurveNode* >* get_nodes() {return &nodes;}
	
	// Set functions
	void set_range(double pos);
	
protected:
	QList<CurveNode* >	nodes;

private :
	
	struct LookupCache {
	    double left;  /* leftmost x coordinate used when finding "range" */
	    std::pair<QList<CurveNode* >::iterator, QList<CurveNode* >::iterator> range;
	};
	
	struct Comparator {
		bool operator() (const CurveNode* a, const CurveNode* b) { 
			return a->get_when() < b->get_when();
		}
	};
	
	
	LookupCache lookup_cache;
	
	bool changed;
	double defaultValue;
	
	double multipoint_eval (double x);
	
	void x_scale(double factor);
	void solve ();
	
	void init();
	
	friend class CurveNode;

public slots:
	void private_add_node(CurveNode* node);
	void private_clear();

private slots:
	void set_changed();


signals :
	void stateChanged();
	void clear_Signal();
};


#endif
