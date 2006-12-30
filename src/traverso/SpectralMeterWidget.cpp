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

    $Id: SpectralMeterWidget.cpp,v 1.10 2006/12/30 14:01:01 n_doebelin Exp $
*/

#include "SpectralMeterWidget.h"
#include <PluginChain.h>
#include <SpectralMeter.h>
#include <Command.h>
#include <ProjectManager.h>
#include <Project.h>
#include <AudioDevice.h>
#include <InputEngine.h>
#include <Song.h>
#include "SpectralMeterConfigWidget.h"

#include <QPainter>
#include <QColor>
#include <QFontMetrics>
#include <QTimer>
#include <QColor>
#include <QPointF>
#include <QDebug>
#include <QPen>
#include <QVector>
#include <QString>
#include <QRect>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <math.h>
#include <limits.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

#define SMOOTH_FACTOR 0.97
#define DB_FLOOR -140.0

static const float DEFAULT_VAL = -999.0f;
static const int UPDATE_INTERVAL = 40;
static const int FONT_SIZE = 7;
static const uint MAX_SAMPLES = UINT_MAX;

SpectralMeterWidget::SpectralMeterWidget(QWidget* parent)
	: ViewPort(parent)
	, m_meter(0)
{
	setMinimumWidth(40);
	setMinimumHeight(10);

	num_bands = 16;
	upper_freq = 22050.0f;
	lower_freq = 20.0f;
	upper_db = 0.0f;
	lower_db = -90.0f;
	upper_freq_log = log10(upper_freq);
	lower_freq_log = log10(lower_freq);
	sample_rate = audiodevice().get_sample_rate();
	show_average = false;
	sample_weight = 1;

	QFontMetrics fm(QFont("Bitstream Vera Sans", FONT_SIZE));
	margin_l = 5;
	margin_r = fm.width("-XX") + 5;
	margin_t = fm.ascent()/2 + 5;
	margin_b = fm.ascent() + fm.descent() + 10;
	
	// We paint all our pixels ourselves, so no need to let Qt
	// erase and fill it for us prior to the paintEvent.
	// @ Nicola : This is where the high load comes from!
        setAttribute(Qt::WA_OpaquePaintEvent);

	for (int i = 0; i < 4; ++i) {
		m_freq_labels.push_back(10.0f * pow(10.0,i));
		m_freq_labels.push_back(20.0f * pow(10.0,i));
		m_freq_labels.push_back(30.0f * pow(10.0,i));
		m_freq_labels.push_back(40.0f * pow(10.0,i));
		m_freq_labels.push_back(50.0f * pow(10.0,i));
		m_freq_labels.push_back(60.0f * pow(10.0,i));
		m_freq_labels.push_back(70.0f * pow(10.0,i));
		m_freq_labels.push_back(80.0f * pow(10.0,i));
		m_freq_labels.push_back(90.0f * pow(10.0,i));
	}

	m_config = new SpectralMeterConfigWidget(this);
	connect(m_config, SIGNAL(closed()), this, SLOT(apply_properties()));

	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(&timer, SIGNAL(timeout()), this, SLOT(update_data()));
}

SpectralMeterWidget::~SpectralMeterWidget()
{
// 	delete m_config;
}

void SpectralMeterWidget::paintEvent( QPaintEvent *  )
{
	PENTER3;

	QPainter painter(viewport());
	painter.drawPixmap(0, 0, bgPixmap);

	// draw the bars
	if (m_spectrum.size()) {
		QPen pen;
		pen.setColor(QColor(80, 80, 120));
		pen.setWidth(bar_width);
		pen.setCapStyle(Qt::FlatCap);
		painter.setClipRegion(m_rect);
		painter.setPen(pen);

		QPointF pt;

		// draw the freq bands
		for (uint i = 0; i < (uint)m_spectrum.size(); ++i) {
			if (m_spectrum.at(i) < lower_db) {
				continue;
			}
			pt.setX(bar_offset + i * m_rect.width() / num_bands);
			pt.setY(db2ypos(m_spectrum.at(i)));
			painter.drawLine(QPointF(pt.x(), height()), pt);
		}

		// draw the average line if requested
		if (show_average) {
			painter.setPen(Qt::red);
			QPointF po(bar_offset, db2ypos(m_avg_db.at(0)));
			for (uint i = 0; i < (uint)m_avg_db.size(); ++i) {
				pt.setX(m_map_idx2xpos.at(i));
				pt.setY(db2ypos(m_avg_db.at(i)));
				painter.drawLine(po, pt);
				po = pt;
			}
		}
	}
}

void SpectralMeterWidget::resizeEvent( QResizeEvent *  )
{
	PENTER3;

	// Make the axis labels disappear when the widget becomes too small
	int x = 0, y = 0, w = width(), h = height();
	if (width() >= 200) {
		x = margin_l;
		w -= (margin_l + margin_r);
	}

	if (height() >= 120) {
		y = margin_t;
		h -= (margin_t + margin_b);
	}

	m_rect.setRect(x, y, w, h);

	// update the vectors mapping indices and frequencies to widget coordinates
	update_freq_map();

	// re-calculate the bar width
	update_barwidth();

	// re-draw the background pixmap
	update_background();
}

void SpectralMeterWidget::update_background()
{
	// draw the background image
	bgPixmap = QPixmap(width(), height());
	bgPixmap.fill(QColor(246, 246, 255));

	QPainter painter(&bgPixmap);
	painter.fillRect(m_rect, QColor(241, 250, 255));
	painter.setFont(QFont("Bitstream Vera Sans", FONT_SIZE));
	QFontMetrics fm(QFont("Bitstream Vera Sans", FONT_SIZE));

	QString spm;

	// draw horizontal lines + labels
	for (float i = upper_db; i >= lower_db; i -= 10.0f) {
		float f = db2ypos(i);

		painter.setPen(QColor(205,223,255));
		painter.drawLine(QPointF(m_rect.x(), f), QPointF(m_rect.right(), f));

		painter.setPen(QColor(  0,  0,  0));
		spm.sprintf("%2.0f", i);
		painter.drawText(m_rect.right() + 1, (int)f + fm.ascent()/2, spm);
	}

	// draw frequency labels and tickmarks
	float last_pos = 1.0;
	for (int i = 0; i < m_freq_labels.size(); ++i) {
		// check if we have space to draw the labels by checking if the
		// m_rect is borderless
		if (!m_rect.top()) {
			break;
		}

		float f = freq2xpos(m_freq_labels.at(i));

		// check if the freq is in the visible range
		if (!f) {
			continue;
		}

		spm.sprintf("%2.0f", m_freq_labels.at(i));
		float s = (float)fm.width(spm)/2.0f;


		// draw text only if there is enough space for it
		if (((f - s) > last_pos) && ((f + s) < float(width()-1))) {
			painter.setPen(Qt::black);
			painter.drawText(QPointF(f - s, height() - fm.descent() - 3), spm);
			last_pos = f + s + 1.0;
		} else {
			painter.setPen(QColor(150, 150, 150));
		}

		painter.drawLine(QPointF(f, m_rect.bottom()), QPointF(f, m_rect.bottom() + 3));
	}
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

	// process the data
	reduce_bands();

	// paint the widget
	viewport()->update();
}

void SpectralMeterWidget::set_project(Project *project)
{
	if (project) {
		connect(project, SIGNAL(currentSongChanged(Song *)), this, SLOT(set_song(Song*)));
		m_project = project;
	} else {
		timer.stop();
	}
}

void SpectralMeterWidget::set_song(Song *song)
{
	PluginChain* chain = song->get_plugin_chain();
	
	connect(song, SIGNAL(transferStarted()), this, SLOT(transfer_started()));
	connect(song, SIGNAL(transferStopped()), this, SLOT(transfer_stopped()));

	foreach(Plugin* plugin, chain->get_plugin_list()) {
		// Nicola: qobject_cast didn't have the behaviour I thought
		// it would have, so I switched it to dynamic_cast!
		m_meter = dynamic_cast<SpectralMeter*>(plugin);
		
		if (m_meter) {
			printf("using existing meter\n");
			timer.start(UPDATE_INTERVAL);
			return;
		}
	}
	
	m_meter = new SpectralMeter();
	m_meter->init();
	ie().process_command( chain->add_plugin(m_meter, false) );

	timer.start(UPDATE_INTERVAL);
}

// most of the calculations is done here:
//	1. the gain values must be converted to db
//	2. since we don't want to show every frequency bin, we reduce the number of freqs
//		and split them into bands evenly distributed on a log10 x-scale.
//		This is pretty complicated and is the reason for most of the weired code below
//	3. calculate the average db levels
void SpectralMeterWidget::reduce_bands()
{
	// check if we have to update some variables
	if ((m_spectrum.size() != (int)num_bands) 
		|| (fft_size != qMin(specl.size(), specr.size())) 
		|| (m_map_idx2freq.size() != fft_size)) {
			update_layout();
	}

	// calculate the sample weight for the average curve
	double sweight = 1.0 / (double)sample_weight;
	double oweight = 1.0 - sweight;

	// used for smooth falloff
	float hist = DB_FLOOR + (m_spectrum.at(0) - DB_FLOOR) * SMOOTH_FACTOR;

	bool skip = false;

	// Convert fft results to dB. Heavily re-arranged version of the following function:
	// db = (20 * log10(2 * sqrt(r_a^2 + i_a^2) / N) + 20 * log10(2 * sqrt(r_b^2 + i_b^2) / N)) / 2
	// with (r_a^2 + i_a^2) and (r_b^2 + i_b^2) given in specl and specr vectors
	m_spectrum[0] = DB_FLOOR + (m_spectrum.at(0) - DB_FLOOR) * SMOOTH_FACTOR;

	// loop through the fft vectors
	for (uint i = 0, j = 0; i < fft_size; ++i) {
		float freq = m_map_idx2freq.at(i);

		if (freq < (float)lower_freq) {
			// We are still below the lowest displayed frequency
			skip = true;
		} else {
			skip = false;
		}

		if (freq >= m_bands.at(j)) {
			// we entered the freq range of the next band
			++j;
			if (j >= (uint)m_spectrum.size()) {
				// We are above the highest displayed frequency
				skip = true;
			} else {
				// move to the next band and fill it with the smooth falloff value as default
				hist = DB_FLOOR + (m_spectrum.at(j) - DB_FLOOR) * SMOOTH_FACTOR;
				m_spectrum[j] = hist;
			}
		}

		// calculate the db value of the current bin
		float val = 5.0 * (log10(specl.at(i) * specr.at(i)) + xfactor);

		// fill the average sample curve
		if (show_average) {
			if (i < (uint)m_avg_db.size()) {
				double dv = val * sweight + m_avg_db.at(i) * oweight;
				m_avg_db[i] = dv;
			}
		}

		// write back the actual db value to the current freq band
		if (!skip) {
			m_spectrum[j] = qMax(val, m_spectrum.at(j));
		}
	}

	// progress the sample weighting for the average curve
	if ((show_average) && (sample_weight < (MAX_SAMPLES - 1))) {
		++sample_weight;
	}
}

// call this function if the size, number of bands, ranges etc. changed.
// it re-calculates some variables
void SpectralMeterWidget::update_layout()
{
	timer.stop();

	// recalculate a couple of variables
	fft_size = qMin(specl.size(), specr.size());		// number of frequencies
	xfactor = 4.0f * log10(2.0f / float(fft_size));	// a constant factor for conversion to dB
	upper_freq_log = log10(upper_freq);
	lower_freq_log = log10(lower_freq);
	freq_step = (upper_freq_log - lower_freq_log)/(num_bands);
	sample_weight = 1;

	// recreate the vector containing the levels and frequency bands
	m_spectrum.fill(DEFAULT_VAL, num_bands);
	m_avg_db.fill(DEFAULT_VAL, fft_size);

	// recreate the vector containing border frequencies of the freq bands
	m_bands.clear();
	for (uint i = 0; i < num_bands; ++i) {
		m_bands.push_back(pow(10.0, lower_freq_log + (i+1)*freq_step));
	}

	// update related stuff
	update_barwidth();
	update_freq_map();

	timer.start(UPDATE_INTERVAL);
}

// converts db-values into widget y-coordinates
float SpectralMeterWidget::db2ypos(float f)
{
	return ((f - upper_db) * m_rect.height()/(lower_db - upper_db)) + m_rect.top();
}

// converts frequencies into widget x-coordinates
float SpectralMeterWidget::freq2xpos(float f)
{
	if ((f < lower_freq) || (f > upper_freq)) {
		return 0.0;
	}

	float d = log10(f) - lower_freq_log;
	return (float)margin_l + d * m_rect.width() / (upper_freq_log - lower_freq_log);
}

// re-calculates the bar width of the frequency bands
void SpectralMeterWidget::update_barwidth()
{
	int i = num_bands < 128 ? 2 : 0;
	bar_width = int(0.5 + (float)m_rect.width() / (float)num_bands) - i;
	bar_width = bar_width < 1 ? 1 : bar_width;
	bar_offset = int((float)m_rect.width() / (2.0f * num_bands));
	bar_offset += m_rect.x();
}

// updates a vector mapping fft indices (0, ..., fft_size) to widget x-positions
// and one mapping fft indices to frequency
void SpectralMeterWidget::update_freq_map()
{
	m_map_idx2xpos.clear();
	m_map_idx2freq.clear();
	for (uint i = 0; i < fft_size; ++i) {
		float freq = float(i+1) * (float)sample_rate / (2.0f * fft_size);
		m_map_idx2freq.push_back(freq);
		m_map_idx2xpos.push_back(freq2xpos(freq));
	}
}

// opens the properties dialog
Command* SpectralMeterWidget::edit_properties()
{
	if (!m_meter) {
		return 0;
	}

	m_config->show();
	m_config->set_upper_freq((int)upper_freq);
	m_config->set_lower_freq((int)lower_freq);
	m_config->set_num_bands(num_bands);
	m_config->set_upper_db((int)upper_db);
	m_config->set_lower_db((int)lower_db);
	m_config->set_show_average(show_average);

	m_config->set_fr_len(m_meter->get_fr_size());
	m_config->set_windowing_function(m_meter->get_windowing_function());

	return 0;
}

// is called upon closing the properties dialog
void SpectralMeterWidget::apply_properties()
{
	m_config->hide();
	upper_freq = (float)m_config->get_upper_freq();
	lower_freq = (float)m_config->get_lower_freq();
	num_bands = m_config->get_num_bands();
	upper_db = (float)m_config->get_upper_db();
	lower_db = (float)m_config->get_lower_db();
	show_average = m_config->get_show_average();

	if (m_meter) {
		m_meter->set_fr_size(m_config->get_fr_len());
		m_meter->set_windowing_function(m_config->get_windowing_function());
		m_meter->init();
	}

	update_layout();
	update_background();
}

void SpectralMeterWidget::transfer_started()
{
	// restarts the average curve
	sample_weight = 1;
}

void SpectralMeterWidget::transfer_stopped()
{

}

Command* SpectralMeterWidget::set_mode()
{
	show_average = !show_average;
	update_layout();
	return 0;
}

Command* SpectralMeterWidget::reset()
{
	sample_weight = 1;
	return 0;
}

Command* SpectralMeterWidget::show_export_widget()
{
	// check if all requirements are met
	if ((!show_average) || (!m_avg_db.size()) || (!m_project)) {
		printf("No average data available.");
		return 0;
	}

	// check if there actually is data to export
	int s = qMin(m_map_idx2freq.size(), m_avg_db.size());
	if (!s) {
		printf("No average data available.");
		return 0;
	}

	QString fn = QFileDialog::getSaveFileName (0, tr("Export average dB curve"), m_project->get_root_dir());

	// if aborted exit here
	if (fn.isEmpty()) {
		return 0;
	}

	QFile file(fn);

	// check if the selected file can be opened for writing
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		printf("Could not open file for writing.");
		return 0;
	}

	QTextStream out(&file);
	QString separator = " ";
	QString str;

	// export the data
	// bin 0 contains no useful data (freq is 0 Hz anyway)
	for (int i = 0; i < s; ++i) {
		out << str.sprintf("%.6f %.6f\n", m_map_idx2freq.at(i), m_avg_db.at(i));
	}

	return 0;
}

//eof
