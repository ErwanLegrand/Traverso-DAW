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

$Id: SpectralMeter.cpp,v 1.7 2008/02/07 12:52:57 n_doebelin Exp $

*/


#include "SpectralMeter.h"
#include <AudioBus.h>
#include <QVector>
#include <math.h>
#include <QDebug>

#include <Debugger.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

#define PI 3.141592653589
#define BUFFER_READOUT_TOLERANCE 2 // recommended: 1-10

SpectralMeter::SpectralMeter()
	: Plugin()
{
	m_frlen = 2048;
	m_windowingFunction = 1;
	m_bufferreadouts = 0;

	(int) init();

	// constructs a ringbuffer that can hold 16384 samples
	m_databufferL = new RingBufferNPT<float>(16384);
	m_databufferR = new RingBufferNPT<float>(16384);
}


SpectralMeter::~SpectralMeter()
{
	delete m_databufferL;
	delete m_databufferR;
}


QDomNode SpectralMeter::get_state( QDomDocument doc )
{
	QDomElement node = doc.createElement("Plugin");
	node.setAttribute("type", "SpectralMeterPlugin");
	node.setAttribute("bypassed", is_bypassed());
	return node;
}


int SpectralMeter::set_state(const QDomNode & node )
{
	QDomElement e = node.toElement();
	m_bypass = e.attribute( "bypassed", "0").toInt();
	
	return 1;
}


int SpectralMeter::init()
{
	fftsigl  = NDArray(m_frlen);		// array of input values (windowed samples)
	fftsigr  = NDArray(m_frlen);		// array of input values (windowed samples)
	fftspecl = NFFTWArray(m_frlen/2 + 1);	// array of output values (complex numbers)
	fftspecr = NFFTWArray(m_frlen/2 + 1);	// array of output values (complex numbers)
	pfegl = fftw_plan_dft_r2c_1d(m_frlen, fftsigl, fftspecl, FFTW_ESTIMATE);
	pfegr = fftw_plan_dft_r2c_1d(m_frlen, fftsigr, fftspecr, FFTW_ESTIMATE);

	win = NDArray(m_frlen);
	switch (m_windowingFunction)
	{
		case 0: // rectangle
			for (int i = 0; i < m_frlen; ++i) {
				win[i] = 1.0;
			}
			break;

		case 1: // hanning
			for (int i = 0; i < m_frlen; ++i) {
				win[i] = 0.5 - 0.5 * cos(2.0 * i * PI / m_frlen);
			}
			break;

		case 2: // hamming
			for (int i = 0; i < m_frlen; ++i) {
				win[i] = 0.54 - 0.46 * cos(2.0 * i * PI / m_frlen);
			}
			break;

		case 3: // blackman
			for (int i = 0; i < m_frlen; ++i) {
				win[i] = 0.42 - 0.5 * cos(2.0 * PI * i / m_frlen) +
					 0.08 * cos(4.0 * PI * i / m_frlen);
			}
			break;
	}
	// calculates the Hanning window (?)
/*	for (int i = 0; i < m_frlen; ++i) {
		win[i] = pow(sin(PI * i / m_frlen), 2);
	}
*/
	return 1;
}

int SpectralMeter::get_fr_size()
{
	return m_frlen;
}

void SpectralMeter::set_fr_size(int i)
{
	m_frlen = i;
}

void SpectralMeter::set_windowing_function(int i)
{
	m_windowingFunction = i;
}

int SpectralMeter::get_windowing_function()
{
	return m_windowingFunction;
}

void SpectralMeter::process(AudioBus* bus, unsigned long nframes)
{
	if ( is_bypassed() ) {
		return;
	}


	// The nframes is the amount of samples there are in the buffers
	// we have to process. No need to get the buffersize, we _have_ to
	// use the nframes variable !
	m_databufferL->write(bus->get_buffer(0, nframes), nframes);
	m_databufferR->write(bus->get_buffer(1, nframes), nframes);
}


QString SpectralMeter::get_name( )
{
	return QString(tr("SpectralMeter Meter"));
}

// writes the fft output into two qvector<float> (left and right channel).
int SpectralMeter::get_data(QVector<float> &specl, QVector<float> &specr)
{
	int readcount = m_databufferL->read_space();
	
	// If there is not enough new data for an FFT window in the ringbuffer,
	// decide if the cycle should be ignored or if the fft spectrum should
	// be filled with 0. Ignore it as long as the number of readouts is 
	// below the BUFFER_READOUT_TOLERANCE.
	if (readcount < m_frlen) {
		// add another 'if' to avoid unlimited growth of the variable
		if (m_bufferreadouts <= BUFFER_READOUT_TOLERANCE) {
			m_bufferreadouts++;
		}

		if (m_bufferreadouts >= BUFFER_READOUT_TOLERANCE) {
			// return spectra filled with 0	
			specl.clear();
			specr.clear();
			for (int i = 1; i < m_frlen/2 + 1; ++i) {
				specl.push_back(0.0);
				specr.push_back(0.0);
			}
			return -1; // return -1 to inform the receiver about silence
		} else {
			return 0;
		}
	} else {
		m_bufferreadouts = 0;
	}

	specl.clear();
	specr.clear();

	// Create an empty SpectralMeterData struct data,
	float left = 0.0;
	float right = 0.0;

	// read samples (in chunks of buffer_size) from the ringbuffer until the FFT window is filled
	m_databufferL->read(&left, 1);
	m_databufferR->read(&right, 1);

	for (int i = 0; i < m_frlen; ++i) {
		m_databufferL->read(&left, 1);
		m_databufferR->read(&right, 1);
		fftsigl[i] = (double)left * win[i];
		fftsigr[i] = (double)right * win[i];
	}

	// do the FFT calculations for the left and right channel
	fftw_execute(pfegl);
	fftw_execute(pfegr);

	float tmp;

	// send the fft spectrum to the caller
	for (int i = 1; i < m_frlen/2 + 1; ++i) {
		tmp = pow((float)fftspecl[i][0],2.0f) + pow((float)fftspecl[i][1],2.0f);
		specl.push_back(tmp);
		tmp = pow((float)fftspecr[i][0],2.0f) + pow((float)fftspecr[i][1],2.0f);
		specr.push_back(tmp);
	}

	return 1;
}

