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
 
    $Id: VUMeterOverLed.h,v 1.3 2006/11/13 20:17:27 n_doebelin Exp $
*/

#ifndef VUMETEROVERLED_H
#define VUMETEROVERLED_H

#include <QWidget>
#include <QColor>

class VUMeterOverLed : public QWidget
{
	Q_OBJECT
	
public:

        VUMeterOverLed(QWidget* parent);

public slots:
/**
 * Switches the LED indicator on and off. Connect this slot with the signal
 * VUMeterLevel::activate_over_led(bool).
 *
 * @param b new state of the LED indicator. 
 */
	void set_active(bool b);

protected:
        void paintEvent( QPaintEvent* e);


private:
        bool isActive;
};

#endif

