/*
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
 
    $Id: MultiMeter.h,v 1.2 2006/11/24 12:06:47 r_sijrier Exp $
*/

#ifndef MULTIMETER_H
#define MULTIMETER_H

#include "defines.h"
#include <RingBufferNPT.h>
#include <QObject>

class AudioBus;

struct MultiMeterData
{
	float 	r;
	float	levelLeft;
	float	levelRight;
};

class MultiMeter : public QObject
{
	Q_OBJECT
public:
	MultiMeter();
	~MultiMeter();

	void process(AudioBus* bus, nframes_t nframes);
	int get_data(float& r, float& direction);

private:
	RingBufferNPT<MultiMeterData>*	m_databuffer;
	MultiMeterData			m_history;
	float				m_avgLeft;
	float				m_avgRight;
	float				m_fract;
	
private slots:
	void calculate_fract();
};

#endif

/** EOF **/
