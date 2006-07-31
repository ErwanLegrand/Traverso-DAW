/*
Copyright (C) 2006 Remon Sijrier

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

$Id: PluginSlider.h,v 1.1 2006/07/31 13:24:46 r_sijrier Exp $
*/


#ifndef PLUGIN_SLIDER_H
#define PLUGIN_SLIDER_H

#include <QSlider>
#include <QObject>


class PluginSlider : public QSlider
{
	Q_OBJECT
	
public:
	PluginSlider(enum Qt::Orientation);
	~PluginSlider(){};
	
	void set_maximum(float max);
	void set_minimum(float min);
	void set_value(float value);
	
private slots:
	void value_changed(int value);

signals:
	void sliderValueChanged(double value);
	
};
 
#endif

//eof
