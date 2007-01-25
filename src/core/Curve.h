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

$Id: Curve.h,v 1.15 2007/01/25 12:15:58 r_sijrier Exp $
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
	void rt_get_vector (double x0, double x1, float *arg, int32_t veclen);
	
	QList<CurveNode* >* get_nodes() {return &m_data.nodes;}
	Song* get_song() const {return m_song;}
	
	// Set functions
	void set_range(double pos);
	
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
	
	
	struct CurveData {
		QList<CurveNode* >	nodes;
		LookupCache lookup_cache;
		bool isGui;
		bool changed;
	};
	
	CurveData m_data;
	CurveData m_rtdata;
	Song* m_song;
	
	double defaultValue;
	
	double multipoint_eval (CurveData* data, double x);
	
	void x_scale(double factor);
	void solve (CurveData* data);
	
	void init();
	
	void _get_vector (CurveData*, double x0, double x1, float *arg, int32_t veclen);
	
	friend class CurveNode;

protected slots:
	void set_changed();

private slots:
	void rt_private_add_node(CurveNode* node);
	void rt_private_remove_node(CurveNode* node);
	void rtdata_set_changed();
	void data_set_changed();
	
	void private_add_node(CurveNode* node);
	void private_remove_node(CurveNode* node);
	


signals :
	void stateChanged();
	void rangeChanged();
	void nodeAdded(CurveNode*);
	void nodeRemoved(CurveNode*);
};


inline double Curve::get_range( ) const
{
	if ( ! m_data.nodes.isEmpty()) {
		return m_data.nodes.last()->get_when();
	}
		
	return 0;
}

#endif
