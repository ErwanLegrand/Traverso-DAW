/*
    Copyright (C) 2006 Nicola Doebelin
 
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
 
    $Id: VUMeterRuler.h,v 1.3 2006/11/13 20:17:27 n_doebelin Exp $
*/

#ifndef VUMETERRULER_H
#define VUMETERRULER_H

#include <QWidget>
#include <QVector>

class VUMeterRuler : public QWidget
{
	Q_OBJECT
	
public:

        VUMeterRuler(QWidget* parent);

/**
 * Sets the offset from the top to the first tick mark (+6.0 dB) in pixels.
 * Set this value to the height of the 'over' LED (VUMeterOverLed) in order
 * to position the first tick mark at the top of the VUMeterLevel.
 *
 * @param i offset from top in pixels
 */
	void setYOffset(int);

protected:
        void paintEvent( QPaintEvent* e);


private:
	std::vector<int>	presetMark;
	std::vector<int>	lineMark;
	int			yOffset;

};

#endif

