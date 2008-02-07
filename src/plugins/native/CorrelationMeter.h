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

$Id: CorrelationMeter.h,v 1.2 2008/02/07 11:46:09 n_doebelin Exp $
*/


#ifndef CORRELATION_METER_H
#define CORRELATION_METER_H

#include "Plugin.h"
#include "defines.h"
#include <RingBufferNPT.h>
#include <QObject>

class AudioBus;

struct CorrelationMeterData
{
	float 	r;
	float	levelLeft;
	float	levelRight;
};

class CorrelationMeter : public Plugin
{
	Q_OBJECT

public:
	CorrelationMeter();
	~CorrelationMeter();

	int init();
	QDomNode get_state(QDomDocument doc);
	int set_state(const QDomNode & node );
	void process(AudioBus* bus, unsigned long nframes);
	QString get_name();
	
	int get_data(float& r, float& direction);

private:
	RingBufferNPT<CorrelationMeterData>*	m_databuffer;
	CorrelationMeterData			m_history;
	float					m_fract;
	int					m_bufferreadouts;
	
private slots:
	void calculate_fract();


};


#endif
 
