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

    $Id: SpectralMeterWidget.cpp,v 1.1 2006/12/09 08:44:54 n_doebelin Exp $
*/

#include <libtraverso.h>

#include "SpectralMeterWidget.h"
#include <PluginChain.h>
#include <CorrelationMeter.h>
#include "ProjectManager.h"
#include "Project.h"
#include <AudioDevice.h>
#include "InputEngine.h"
#include "Song.h"

#include <QPainter>
#include <QColor>
#include <QGradient>
#include <QPixmap>
#include <QFontMetrics>
#include <QTimer>
#include <QLinearGradient>
#include <QColor>
#include <QPointF>
#include <QDebug>
#include <QPen>
#include <QVector>
#include <QString>
#include <math.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

#define SMOOTH_FACTOR 0.97
#define DB_FLOOR -140.0

static const float DEFAULT_VAL = -999.0f;

SpectralMeterWidget::SpectralMeterWidget(QWidget* parent)
	: QWidget(parent)
	, m_meter(0)
{
	setMinimumWidth(40);
	setMinimumHeight(10);
	
	// We paint all our pixels ourselves, so no need to let Qt
	// erase and fill it for us prior to the paintEvent.
	// @ Nicola : This is where the high load comes from!
        setAttribute(Qt::WA_OpaquePaintEvent);
	
	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));

	connect(&timer, SIGNAL(timeout()), this, SLOT(update_data()));

	num_bands = 32;
	upper_freq = 20000;
	lower_freq = 20;
	upper_db = 0;
	lower_db = -90;
	sample_rate = audiodevice().get_sample_rate();
}

void SpectralMeterWidget::paintEvent( QPaintEvent *  )
{
	PENTER3;

	QPainter painter(this);
	painter.fillRect(0, 0, width(), height(), Qt::black);

	if (!m_spectrum.size()) {
		return;
	}

	int w = int((float)width() / (float)num_bands) - 2;
	int d = int((float)width() / (2.0f * num_bands));

	QPen pen(Qt::green);
	pen.setWidth(w);
	painter.setPen(pen);

	QPointF pt;

	for (uint i = 0; i < num_bands; ++i) {
		pt.setX(d + i * width() / num_bands);
		pt.setY(m_spectrum.at(i) * height()/(float)lower_db);

		painter.drawLine(QPointF(pt.x(), height()), pt);
	}
}

void SpectralMeterWidget::resizeEvent( QResizeEvent *  )
{
	PENTER3;
}

void SpectralMeterWidget::update_data()
{
	if (!m_meter) {
		return;
	}

	// if no data was available, return, so we _only_ update the widget when
	// it needs to be!
 	if (m_meter->get_data(specl, specr) == 0) {
 		return;
 	}

	reduce_bands();
	update();
}

void SpectralMeterWidget::set_project(Project *project)
{
	if (project) {
		connect(project, SIGNAL(currentSongChanged(Song *)), this, SLOT(set_song(Song*)));
	} else {
		timer.stop();
	}
}

void SpectralMeterWidget::set_song(Song *song)
{
	PluginChain* chain = song->get_plugin_chain();
	
	foreach(Plugin* plugin, chain->get_plugin_list()) {
		// Nicola: qobject_cast didn't have the behaviour I thought
		// it would have, so I switched it to dynamic_cast!
		m_meter = dynamic_cast<SpectralMeter*>(plugin);
		
		if (m_meter) {
			printf("using existing meter\n");
			timer.start(40);
			return;
		}
	}
	
	m_meter = new SpectralMeter();
	m_meter->init();
	ie().process_command( chain->add_plugin(m_meter, false) );
	timer.start(40);
}

void SpectralMeterWidget::reduce_bands()
{
	uint s = specl.size();			// number of frequencies
	float f = 4.0f * log10(2.0f / (2.0f * s));	// a constant factor for conversion to dB

	// create a vector for the frequency bands, containing very small default values
	if (m_spectrum.size() != num_bands) {
		update_band_lut();
		m_spectrum.fill(DEFAULT_VAL, num_bands);
	}

	// used for smooth falloff
	float hist = DB_FLOOR + (m_spectrum.at(0) - DB_FLOOR) * SMOOTH_FACTOR;

	// Convert fft results to dB. Heavily simplyfied version of the following function:
	// db = (20 * log10(2 * sqrt(r_a^2 + i_a^2) / N) + 20 * log10(2 * sqrt(r_b^2 + i_b^2) / N)) / 2
	// with (r_a^2 + i_a^2) and (r_b^2 + i_b^2) given in specl and specr vectors
	for (uint i = 0, j = 0; i <= s; ++i) {
		float freq = i * (float)sample_rate / (2.0f * s);

		if (freq < (float)lower_freq) {
			// We are still below the lowest displayed frequency
			continue;
		}

		if (freq >= m_bands.at(j)) {
			// we entered the freq range of the next band
			++j;
			if (j >= num_bands) {
				// We are above the highest displayed frequency
				return;
			} else {
				// calc smooth falloff
				hist = DB_FLOOR + (m_spectrum.at(j) - DB_FLOOR) * SMOOTH_FACTOR;
			}
		}


		float val = 5.0 * (log10(specl.at(i) * specr.at(i)) + f);
		m_spectrum[j] = qMax(val, hist);
	}
}

void SpectralMeterWidget::update_band_lut()
{
	float ufl = log10(upper_freq);
	float lfl = log10(lower_freq);
	float step = (ufl - lfl)/(num_bands + 1);

	m_bands.clear();

	for (uint i = 1; i <= num_bands; ++i) {
		m_bands.push_back(pow(10.0, lfl + i*step));
	}
	m_bands.push_back(upper_freq);
}

//eof
