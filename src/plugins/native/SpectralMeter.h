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

$Id: SpectralMeter.h,v 1.2 2006/12/09 08:44:54 n_doebelin Exp $
*/


#ifndef SPECTRAL_METER_H
#define SPECTRAL_METER_H

#include "Plugin.h"
#include "defines.h"
#include <fftw3.h>
#include <RingBufferNPT.h>
#include <QVector>

class AudioBus;

struct SpectralMeterData
{
	audio_sample_t*	bufferLeft;
	audio_sample_t*	bufferRight;
};

class SpectralMeter : public Plugin
{
public:
	SpectralMeter();
	~SpectralMeter();

	int init();
	QDomNode get_state(QDomDocument doc);
	int set_state(const QDomNode & node );
	void process(AudioBus* bus, unsigned long nframes);
	QString get_name();

	int get_data(QVector<float>& specl, QVector<float>& specr);

private:
	// FFTW globals
	fftw_plan pfegl, pfegr;
	fftw_complex *fftspecl,*fftspecr;
	double *fftsigl,*fftsigr,*win;
	RingBufferNPT<float>*	m_databufferL;
	RingBufferNPT<float>*	m_databufferR;

	float   *NFArray(int size){
		float *p;
		p = (float *)calloc(size,sizeof(*p));
		return p;
	}
	double  *NDArray(int size){
		double *p;
		p = (double *)calloc(size,sizeof(*p));
		return p;
	}
	int     *NIArray(int size){
		int *p;
		p = (int *)calloc(size,sizeof(*p));
		return p;
	}
	fftw_complex  *NFFTWArray(int size){
		fftw_complex *p;
		p = (fftw_complex *)fftw_malloc(size*sizeof(*p));
		return p;
	}
};


#endif
