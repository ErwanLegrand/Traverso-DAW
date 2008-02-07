/*Copyright (C) 2006 Remon Sijrier

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

$Id: CorrelationMeter.cpp,v 1.5 2008/02/07 15:33:17 n_doebelin Exp $

*/


#include "CorrelationMeter.h"
#include <AudioBus.h>
#include <AudioDevice.h>
#include <Debugger.h>
#include <math.h>
#include <limits.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"
		
#define BUFFER_READOUT_TOLERANCE 2  // recommended: 1-10
#define RINGBUFFER_SIZE 150
#define METER_COLLAPSE_SPEED 0.05


CorrelationMeter::CorrelationMeter()
	: Plugin()
{
	// constructs a ringbuffer that can hold 150 CorrelationMeterData structs
	m_databuffer = new RingBufferNPT<CorrelationMeterData>(RINGBUFFER_SIZE);
	
	// Initialize member variables, that need to be initialized
	calculate_fract();
	// With memset, we're able to very efficIf there is no new data in the ringbuffer, fill it with default values,
	// otherwise the meter will stop working between clipsiently set all bytes of an array
	// or struct to zero
	memset(&m_history, 0, sizeof(CorrelationMeterData));
	
	m_bufferreadouts = 0;

	connect(&audiodevice(), SIGNAL(driverParamsChanged()), this, SLOT(calculate_fract()));
}


CorrelationMeter::~CorrelationMeter()
{
	delete m_databuffer;
}


QDomNode CorrelationMeter::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("Plugin");
	node.setAttribute("type", "CorrelationMeterPlugin");
	node.setAttribute("bypassed", is_bypassed());
	return node;
}


int CorrelationMeter::set_state(const QDomNode & node )
{
	QDomElement e = node.toElement();
	m_bypass = e.attribute( "bypassed", "0").toInt();
	
	return 1;
}


int CorrelationMeter::init()
{
	return 1;
}


void CorrelationMeter::process(AudioBus* bus, unsigned long nframes)
{
	if ( is_bypassed() ) {
		return;
	}
	
	// check if audiobus is stereo (2 channels)
	// if not, do nothing
	if (bus->get_channel_count() != 2)
		return;

	// The nframes is the amount of samples there are in the buffers
	// we have to process. No need to get the buffersize, we _have_ to
	// use the nframes variable !
	audio_sample_t* bufferLeft = bus->get_buffer(0, nframes);
	audio_sample_t* bufferRight = bus->get_buffer(1, nframes);


	// Variables we need to calculate the correlation and avarages/levels
	float a1, a2, a1a2 = 0, a1sq = 0, a2sq = 0, r, levelLeft = 0, levelRight = 0;
	
	// calculate coefficient
	for (uint i = 0; i < nframes; ++i) {
		a1 = bufferLeft[i];
		a2 = bufferRight[i];

		a1a2 += a1 * a2;
		a1sq += a1 * a1;
		a2sq += a2 * a2;

		levelLeft += a1sq;
		levelRight += a2sq;
	}

	// We have all data to calculate the correlation coefficient
	// for the processed buffer (but check for division by 0 first)
	if ((a1sq == 0.0) || (a2sq == 0.0)) {
		r = 1.0;
	} else {
		r = a1a2 / (sqrtf(a1sq) * sqrtf(a2sq));
	}

	// calculate RMS of the levels
	levelLeft = sqrtf(levelLeft / nframes);
	levelRight = sqrtf(levelRight / nframes);

	// And we store this in a CorrelationMeterData struct
	// and write this struct into the data ringbuffer,
	// to be processed later in get_data()
	// levelLeft and levelRight are also needed to calculate the
	// correct direction in get_data(), so we store that too!
	CorrelationMeterData data;
	data.r = r;
	data.levelLeft = levelLeft;
	data.levelRight = levelRight;

	// The ringbuffer::write function acts like it's appending the data
	// to the end of the buffer.
	// The amount of CorrelationMeterData structs we want to write is 1, and 
	// we have to provide a pointer to the data we want to write, which
	// is done by dereferencing (the & in front of) data.
	//
	// This would also have worked (since it's essentially the same):
	// CorrelationMeterData* datatowrite = &data;
	// m_databuffer->write(datatowrite, 1);
	// 
	// If we want to write more then 1 CorrelationMeterData struct, we have to 
	// place them into an array, but well, there's only one now :-)
	m_databuffer->write(&data, 1);
}


/**
 * Compute the correlation coefficient of the stereo master output data
 * of the active sheet, and the direction. When there is new data, the new
 * data will be assigned to the \a r and \a direction variables, else no 
 * data will be assigned to those variables
 * 
 * r: linear correlation coefficient (1.0: complete positive correlation,
 * 0.0: uncorrelated, -1.0: complete negative correlation).
 *
 * @returns 0 if no new data was available, > 0 when new data was available
 *	The new data will be assigned to \a r and \a direction
 **/
int CorrelationMeter::get_data(float& r, float& direction)
{
	// RingBuffer::read_space() tells us how many data
	// of type T (CorrelationMeterData in this case) has been written 
	// to the buffer since last time we checked.
	int readcount = m_databuffer->read_space();

	// If there is no new data in the buffer, this may have 2 reasons:
	// 
	// 1) too fast readout, buffer is not ready again
	// 2) no data available because no data is played back (e.g. between clips)
	// 
	// We want to distinguish the two cases, because the behavour of the meter
	// should be different. In case 1) we just ignore the update and do nothing, 
	// the next cycle will probably have data available again. In case 2) we
	// want to use dummy values instead, because that's what the meter should
	// display if silence is played. The trick to achieve this is to ignore a 
	// certain number of buffer readouts (defined in BUFFER_READOUT_TOLERANCE).
	// If more readouts occur in a row, we assume that silence is played back,
	// and start collapsing the meter to r = 1.0 in the center.

 	if (readcount == 0) {
		// add another 'if' to avoid unlimited growth of the variable
		if (m_bufferreadouts < RINGBUFFER_SIZE) {
			m_bufferreadouts++;
		}

		// check if dummy values should be stored in the buffer, or
		// if the readout should be ignored
		if (m_bufferreadouts >= BUFFER_READOUT_TOLERANCE) {
			CorrelationMeterData dummydata;
			dummydata.r = 1.0;
			dummydata.levelLeft = 0.0;
			dummydata.levelRight = 0.0;
			for (int i = 0; i < int(METER_COLLAPSE_SPEED / m_fract); ++i) {
				m_databuffer->write(&dummydata, 1);
			}
 		} else {
			return 0;
		}

	} else {
		m_bufferreadouts = 0;
	}

	
	// We need to know the 'history' of these variables to get a smooth
	// and consistent (independend of buffersizes) stereometer behaviour.
	// So we get it from our history struct.
	r = m_history.r;
	float levelLeft = m_history.levelLeft;
	float levelRight = m_history.levelRight;
	
	// Create an empty CorrelationMeterData struct data,
	CorrelationMeterData data;
	
	for (int i=0; i<readcount; ++i) {
		// which we fill by reading from the databuffer.
		m_databuffer->read(&data, 1);
		
		// Calculate the new correlation variable, and merge the old one.
		// Assign it to r itself, this spares a temp. variable for r ;-)
		r = data.r * m_fract + r * (1.0 - m_fract);
	
		// Same for levelLeft/Right
		levelLeft = data.levelLeft * m_fract + levelLeft * (1.0 - m_fract);
		levelRight = data.levelRight * m_fract + levelRight * (1.0 - m_fract);
	}
	
	// Now that we truely have taken into account all the levelLeft/Right data
	// for all buffers that have been processed since last call to get_data()
	// we now can calculate the direction variable.
	if ( ! ((levelLeft + levelRight) == 0.0) ) {
		float vl = levelLeft / (levelLeft + levelRight);
		float vr = levelRight / (levelLeft + levelRight);
	
		direction = vr - vl;
	} else {
		direction = 0;
	}

	// Store the calculated variables in the history struct, to be used on
	// next call of this function
	m_history.r = r;
	m_history.levelLeft = levelLeft;
	m_history.levelRight = levelRight;

	return readcount;
}

void CorrelationMeter::calculate_fract( )
{
	m_fract = ((float) audiodevice().get_buffer_size()) / (audiodevice().get_sample_rate());
}



QString CorrelationMeter::get_name( )
{
	return QString(tr("Correlation Meter"));
}

