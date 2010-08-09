/*
    Copyright (C) 2008 Remon Sijrier
    Copyright (C) 2005-2006 Nicola Doebelin
 
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

#ifndef CORRELATIONMETERWIDGET_H
#define CORRELATIONMETERWIDGET_H

#include "MeterWidget.h"

class Command;
class QLinearGradient;
class QColor;


class CorrelationMeterWidget : public MeterWidget
{

public:
	CorrelationMeterWidget(QWidget* parent);
};

class CorrelationMeterView : public MeterView
{
	Q_OBJECT

	Q_CLASSINFO("set_mode", tr("Toggle display range"))

public:
        CorrelationMeterView(CorrelationMeterWidget* widget);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
	float		coeff;
	float		direction;
	int		range;
	QBrush		m_bgBrush;
	QLinearGradient	gradPhase;

	void save_configuration();
	void load_configuration();

private slots:
	void		update_data();
	void		load_theme_data();

public slots:
	Command*	set_mode();

};

#endif

