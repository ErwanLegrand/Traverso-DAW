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

$Id: Curve.h,v 1.16 2007/02/23 13:49:53 r_sijrier Exp $
*/

#ifndef CURVE_H
#define CURVE_H

#include "ContextItem.h"
#include <QString>
#include <QList>
#include <QDomDocument>

#include "CurveNode.h"

class Song;

class Curve : public ContextItem
{
	Q_OBJECT
	
public:
	Curve(ContextItem* parent, Song* song);
	Curve(ContextItem* parent, Song* song, const QDomNode node);
	~Curve();

	virtual QDomNode get_state(QDomDocument doc);
	virtual int set_state( const QDomNode& node );
	
	Command* add_node(CurveNode* node, bool historable=true);
	Command* remove_node(CurveNode* node, bool historable=true);
	
	// Get functions
	double get_range() const;
	
	void get_vector (double x0, double x1, float *arg, int32_t veclen);
	
	QList<CurveNode* >* get_nodes() {return &m_nodes;}
	Song* get_song() const {return m_song;}
	
	// Set functions
	virtual void set_range(double when);
	

protected:
	struct LookupCache {
		double left;  /* leftmost x coordinate used when finding "range" */
		std::pair<QList<CurveNode* >::iterator, QList<CurveNode* >::iterator> range;
	};
	
	QList<CurveNode* > m_nodes;
	LookupCache m_lookup_cache;
	bool m_changed;

private :
	
	
	struct Comparator {
		bool operator() (const CurveNode* a, const CurveNode* b) { 
			return a->when < b->when;
		}
	};
	
	Song* m_song;
	
	double m_defaultValue;
	
	double multipoint_eval (double x);
	
	void x_scale(double factor);
	void solve ();
	
	void init();
	
	friend class CurveNode;

protected slots:
	void set_changed();

private slots:
	void private_add_node(CurveNode* node);
	void private_remove_node(CurveNode* node);
	


signals :
	void stateChanged();
	void nodeAdded(CurveNode*);
	void nodeRemoved(CurveNode*);
	void nodePositionChanged();
};


inline double Curve::get_range( ) const
{
	if ( ! m_nodes.isEmpty()) {
		return m_nodes.last()->when;
	}
		
	return 0;
}

#endif
