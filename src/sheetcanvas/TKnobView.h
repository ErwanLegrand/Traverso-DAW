/*
Copyright (C) 2011 Remon Sijrier

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

#ifndef TKNOBVIEW_H
#define TKNOBVIEW_H


#include "ViewItem.h"

class Track;

class TKnobView : public ViewItem
{
	Q_OBJECT

public:
	TKnobView(ViewItem* parent, Track* track);
	TKnobView(){}


	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void set_width(int width);
	void load_theme_data();

	void update_angle();

	double max_value() const {return m_maxValue;}
	double min_value() const {return m_minValue;}

	virtual double get_value() const = 0;
	Track* get_track() const {return m_track;}

public slots:
	TCommand* pan_left();
	TCommand* pan_right();

protected:
	Track*		m_track;

private:
	double		m_angle;
	double		m_nTurns;
	double		m_minValue;
	double		m_maxValue;
	double		m_totalAngle;
	QLinearGradient	m_gradient2D;
};

class TPanKnobView : public TKnobView
{
	Q_OBJECT

public:

	TPanKnobView(ViewItem* parent, Track* track);

	double get_value() const;

private slots:
	void track_pan_changed();
};

#endif // TKNOBVIEW_H
