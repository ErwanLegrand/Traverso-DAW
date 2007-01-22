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

    $Id: SpectralMeterWidget.cpp,v 1.24 2007/01/22 15:13:32 r_sijrier Exp $
*/

#include "SpectralMeterWidget.h"
#include <Config.h>
#include <Information.h>
#include <PluginChain.h>
#include <SpectralMeter.h>
#include <Command.h>
#include <ProjectManager.h>
#include <Project.h>
#include <AudioDevice.h>
#include <InputEngine.h>
#include <Song.h>
#include <ContextPointer.h>
#include <ColorManager.h>

#include <QtGui>

#include <math.h>
#include <limits.h>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

#define SMOOTH_FACTOR 0.98
#define DB_FLOOR -140.0

static const float DEFAULT_VAL = -999.0f;
static const int UPDATE_INTERVAL = 40;
static const int FONT_SIZE = 7;
static const uint MAX_SAMPLES = UINT_MAX;


SpectralMeterWidget::SpectralMeterWidget(QWidget* parent)
	: ViewPort(parent)
{
	PENTERCONS;
	setMinimumWidth(40);
	setMinimumHeight(10);
	
	m_item = new SpectralMeterItem(this);
	
	QGraphicsScene* scene = new QGraphicsScene(this);
	setScene(scene);
	scene->addItem(m_item);
	m_item->setPos(0,0);

	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

SpectralMeterWidget::~SpectralMeterWidget()
{
}

void SpectralMeterWidget::resizeEvent( QResizeEvent *  )
{
	m_item->resize();
}

void SpectralMeterWidget::get_pointed_context_items(QList<ContextItem* > &list)
{
	printf("SpectralMeterWidget::get_pointed_view_items\n");
	QList<QGraphicsItem *> itemlist = items(cpointer().on_first_input_event_x(), cpointer().on_first_input_event_y());
	foreach(QGraphicsItem* item, itemlist) {
		list.append((ViewItem*)item);
	}
	
	printf("itemlist size is %d\n", itemlist.size());
}



SpectralMeterItem::SpectralMeterItem(SpectralMeterWidget* widget)
	: ViewItem(0, 0)
	, m_widget(widget)
	, m_meter(0)
{

	m_config = new SpectralMeterConfigWidget(m_widget);
	m_boundingRectangle = QRectF(0, 0, 0, 0);
	load_configuration();
	
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

	connect(m_config, SIGNAL(configChanged()), this, SLOT(load_configuration()));

	// Connections to core:
	connect(&pm(), SIGNAL(projectLoaded(Project*)), this, SLOT(set_project(Project*)));
	connect(&timer, SIGNAL(timeout()), this, SLOT(update_data()));
}

SpectralMeterItem::~SpectralMeterItem()
{
// 	delete m_config;
}

void SpectralMeterItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	painter->drawPixmap(0, 0, bgPixmap);

	// draw the bars
	if (m_spectrum.size()) {
		QRect rect;
		QBrush brush(cm().get("METER_FOREGROUND"), Qt::SolidPattern);
		painter->setClipRegion(m_rect);
		painter->setBrush(brush);
		painter->setPen(Qt::NoPen);

		int spc = 0;
		if (num_bands < 64) spc = 1;

		// draw the freq bands
		for (uint i = 0; i < (uint)m_spectrum.size(); ++i) {
			if (m_bands.at(i+1) < lower_db) {
				continue;
			}

			rect.setTopLeft(QPoint((int)freq2xpos(m_bands.at(i)) + spc, (int)db2ypos(m_spectrum.at(i))));
			rect.setBottomRight(QPoint((int)freq2xpos(m_bands.at(i+1)) - spc, (int)db2ypos(DB_FLOOR)));
			painter->drawRect(rect);
		}

		// draw the average line if requested
		if (show_average) {
			painter->setPen(cm().get("METER_AVERAGE_CURVE"));
			QPoint pt;
			QPoint po((int)m_map_idx2xpos.at(0), (int)db2ypos(m_avg_db.at(0)));
			for (uint i = 0; i < (uint)m_avg_db.size(); ++i) {
				pt.setX((int)m_map_idx2xpos.at(i));
				pt.setY((int)db2ypos(m_avg_db.at(i)));
				painter->drawLine(po, pt);
				po = pt;
			}
		}
	}
}

void SpectralMeterItem::resize()
{
	PENTER;
	
	prepareGeometryChange();
	

	// Make the axis labels disappear when the widget becomes too small
	int x = 0, y = 0, w = m_widget->width(), h = m_widget->height();
	m_boundingRectangle = QRectF(0, 0, w, h);
	
	if (m_widget->width() >= 200) {
		x = margin_l;
		w -= (margin_l + margin_r);
	}

	if (m_widget->height() >= 120) {
		y = margin_t;
		h -= (margin_t + margin_b);
	}

	
	m_rect.setRect(x, y, w, h);

	// update the vectors mapping indices and frequencies to widget coordinates
	update_freq_map();

	// re-draw the background pixmap
	update_background();
}

void SpectralMeterItem::update_background()
{
	// draw the background image
	bgPixmap = QPixmap((int)m_boundingRectangle.width(), (int)m_boundingRectangle.height());
	bgPixmap.fill(cm().get("METER_MARGIN"));

	QPainter painter(&bgPixmap);
	painter.fillRect(m_rect, cm().get("METER_BACKGROUND"));
	painter.setFont(QFont("Bitstream Vera Sans", FONT_SIZE));
	QFontMetrics fm(QFont("Bitstream Vera Sans", FONT_SIZE));

	QString spm;

	// draw horizontal lines + labels
	for (float i = upper_db; i >= lower_db; i -= 10.0f) {
		float f = db2ypos(i);

		painter.setPen(cm().get("METER_GRID"));
		painter.drawLine(QPointF(m_rect.x(), f), QPointF(m_rect.right(), f));

		painter.setPen(cm().get("DARK_TEST"));
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
		if (((f - s) > last_pos) && ((f + s) < float(m_boundingRectangle.width()-1))) {
			painter.setPen(cm().get("DARK_TEXT"));
			painter.drawText(QPointF(f - s, m_boundingRectangle.height() - fm.descent() - 3), spm);
			last_pos = f + s + 1.0;
		} else {
			painter.setPen(cm().get("LIGHT_TEXT"));
		}

		painter.drawLine(QPointF(f, m_rect.bottom()), QPointF(f, m_rect.bottom() + 3));
	}
}

void SpectralMeterItem::update_data()
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
	update();
}

void SpectralMeterItem::set_project(Project *project)
{
	if (project) {
		connect(project, SIGNAL(currentSongChanged(Song *)), this, SLOT(set_song(Song*)));
		m_project = project;
	} else {
		timer.stop();
	}
}

void SpectralMeterItem::set_song(Song *song)
{
	PluginChain* chain = song->get_plugin_chain();
	
	connect(song, SIGNAL(transferStarted()), this, SLOT(transfer_started()));
	connect(song, SIGNAL(transferStopped()), this, SLOT(transfer_stopped()));

	foreach(Plugin* plugin, chain->get_plugin_list()) {
		// Nicola: qobject_cast didn't have the behaviour I thought
		// it would have, so I switched it to dynamic_cast!
		m_meter = dynamic_cast<SpectralMeter*>(plugin);
		
		if (m_meter) {
			timer.start(UPDATE_INTERVAL);
			return;
		}
	}
	
	m_meter = new SpectralMeter();
	m_meter->init();
	ie().process_command( chain->add_plugin(m_meter, false) );

	timer.start(UPDATE_INTERVAL);
}

void SpectralMeterItem::reduce_bands()
{
	// check if we have to update some variables
	if ((m_spectrum.size() != (int)num_bands) 
		|| (fft_size != (uint)qMin(specl.size(), specr.size())) 
		|| ((uint)m_map_idx2freq.size() != fft_size)) {
			update_layout();
	}

	// calculate the sample weight for the average curve
	double sweight = 1.0 / (double)sample_weight;
	double oweight = 1.0 - sweight;

	// loop through the freq bands and determine the db-values
	for (int i = 0; i < m_spectrum.size(); ++i) {
		float val = freq2db(m_bands.at(i), m_bands.at(i+1));
		float hist = DB_FLOOR + (m_spectrum.at(i) - DB_FLOOR) * SMOOTH_FACTOR;
		m_spectrum[i] = qMax(val, hist);
	}

	// fill the average sample curve
	if (show_average) {
		for (int i = 0; i < m_avg_db.size(); ++i) {
			float val = 5.0 * (log10(specl.at(i) * specr.at(i)) + xfactor);
			float v = val * sweight + m_avg_db.at(i) * oweight;
			m_avg_db[i] = v;
		}
	}

	// progress the sample weighting for the average curve
	if ((show_average) && (sample_weight < (MAX_SAMPLES - 1))) {
		++sample_weight;
	}
}

// call this function if the size, number of bands, ranges etc. changed.
// it re-calculates some variables
void SpectralMeterItem::update_layout()
{
	timer.stop();

	// recalculate a couple of variables
	fft_size = qMin(specl.size(), specr.size());		// number of frequencies (size of the FFT)
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
	for (uint i = 0; i <= num_bands; ++i) {
		m_bands.push_back(pow(10.0, lower_freq_log + i*freq_step));
	}

	// update related stuff
	update_freq_map();

	timer.start(UPDATE_INTERVAL);
}

// converts db-values into widget y-coordinates
float SpectralMeterItem::db2ypos(float f)
{
	return ((f - upper_db) * m_rect.height()/(lower_db - upper_db)) + m_rect.top();
}

// converts frequencies into widget x-coordinates
float SpectralMeterItem::freq2xpos(float f)
{
	if ((f < lower_freq) || (f > upper_freq)) {
		return 0.0;
	}

	float d = log10(f) - lower_freq_log;
	return (float)margin_l + d * m_rect.width() / (upper_freq_log - lower_freq_log);
}

// determines the highest db value for frequency rang fl-fu. Does all the interpolation etc.
float SpectralMeterItem::freq2db(float fl, float fu)
{
	float lfreq = qMin(fl, fu);
	float ufreq = qMax(fl, fu);
	float fidxl = lfreq * (2.0f * fft_size) / (float)sample_rate;
	float fidxu = ufreq * (2.0f * fft_size) / (float)sample_rate;

	int idxl = int(floor(fidxl - 1.0f));
	int idxu = int(ceil(fidxu - 1.0f));

	if (idxl < 0) {
		idxl = 0;
	}

	if ((uint)idxu >= fft_size) {
		idxu = fft_size - 1;
	}

	// distinguishing some cases to save cpu cycles.

	// if we're outside the range covered by the FFT, return DB_FLOOR
	if (idxu <= 0) {
		return DB_FLOOR;
	}
	if (idxl >= fft_size) {
		return DB_FLOOR;
	}

	// they are the same (which shouldn't happen)
	if (idxu - idxl == 0) {
		return DB_FLOOR;
	}

	// if we have exactly one bin in the freq range, return it's db-value and exit
	if (idxu - idxl == 2) {
		return 5.0f * (log10(specl.at(idxl + 1) * specr.at(idxl + 1)) + xfactor);
	}

	// if no bin is in between, we have to interpolate in order to get an accurate value
	if (idxu - idxl == 1) {
		float v = fidxu - fidxl;
		float dbvl = specl.at(idxl) * specr.at(idxl);
		float dbvu = specl.at(idxu) * specr.at(idxu);
		return 5.0 * (log10(dbvl + v * (dbvu - dbvl)) + xfactor);
	}

	// if several bins are in between, search the highest db value
	float val = 0.0;
	for (int i = idxl; i <=idxu; ++i) {
		val = qMax(val, specl.at(i) * specr.at(i));
	}

	return 5.0 * (log10(val) + xfactor);
}

// updates a vector mapping fft indices (0, ..., fft_size) to widget x-positions
// and one mapping fft indices to frequency
void SpectralMeterItem::update_freq_map()
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
Command* SpectralMeterItem::edit_properties()
{
	if (!m_meter) {
		return 0;
	}

	m_config->show();
	
	return 0;
}

// is called upon closing the properties dialog
void SpectralMeterItem::load_configuration()
{
	upper_freq = config().get_property("SpectralMeter", "UpperFrequenty", 22050).toInt();
	lower_freq = config().get_property("SpectralMeter", "LowerFrequenty", 20).toInt();
	num_bands = config().get_property("SpectralMeter", "NumberOfBands", 16).toInt();
	upper_db = config().get_property("SpectralMeter", "UpperdB", 0).toInt();
	lower_db = config().get_property("SpectralMeter", "LowerdB", -90).toInt();
	show_average = config().get_property("SpectralMeter", "ShowAvarage", 0).toInt();
	
	if (m_meter) {
		m_meter->set_fr_size(config().get_property("SpectralMeter", "FFTSize", 2048).toInt());
		m_meter->set_windowing_function(config().get_property("SpectralMeter", "WindowingFunction", 1).toInt());
		m_meter->init();
	}

	update_layout();
	update_background();
}

void SpectralMeterItem::transfer_started()
{
	// restarts the average curve
	sample_weight = 1;
}

void SpectralMeterItem::transfer_stopped()
{

}

Command* SpectralMeterItem::set_mode()
{
	show_average = !show_average;
	update_layout();
	return 0;
}

Command* SpectralMeterItem::reset()
{
	sample_weight = 1;
	return 0;
}

Command* SpectralMeterItem::screen_capture( )
{
        QImage image(m_widget->size(), QImage::Format_RGB32);
        QPainter painter(&image);
        m_widget->render(&painter);
	
	QString fn = QFileDialog::getSaveFileName (0, tr("Screen Capture file name"), getenv("HOME"));
	
	// if aborted exit here
	if (fn.isEmpty()) {
		return 0;
	}
	
        if ( ! image.save(fn, "PNG")) {
        	info().warning("Unable to write captured image to hard disk");
        }
 
        return 0;
}

Command* SpectralMeterItem::export_avarage_curve()
{
	// check if all requirements are met
	if ((!show_average) || (!m_project)) {
		printf("No average data available.");
		return 0;
	}

	// check if there actually is data to export
	int s = qMin(m_map_idx2freq.size(), m_avg_db.size());
	if (!s) {
		printf("No average data available.");
		return 0;
	}

	QStringList oFormats;

	QString formatA = "plain text";
	QString formatB = "XMGrace";

	oFormats.append(formatA);
	oFormats.append(formatB);

	bool ok;
	QString format = QInputDialog::getItem( 0, tr("Select output format"), tr("Output format:"), 
						oFormats, 0, false, &ok );

	if (!ok) {
		printf("Aborted.");
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

	if (format == formatA) {
		// export the data in text format
		for (int i = 0; i < s; ++i) {
			out << str.sprintf("%.6f %.6f\n", m_map_idx2freq.at(i), m_avg_db.at(i));
		}

		return 0;
	}

	if (format == formatB) {
		// export the data in XMGrace format
		out << get_xmgr_string();
		return 0;
	}

	return 0;
}

QString SpectralMeterItem::get_xmgr_string()
{
	QString s = "# Grace project file\n";
	s += "#\n";
	s += "@version 50120\n";
	s += "@page size 842, 594\n";
	s += "@page scroll 5\%\n";
	s += "@page inout 5\%\n";
	s += "@link page off\n";
	s += "@map font 39 to \"Courier\", \"Courier\"\n";
	s += "@map font 40 to \"Courier-Bold\", \"Courier-Bold\"\n";
	s += "@map font 11 to \"Courier-BoldOblique\", \"Courier-BoldOblique\"\n";
	s += "@map font 9 to \"Courier-Oblique\", \"Courier-Oblique\"\n";
	s += "@map font 4 to \"Helvetica\", \"Helvetica\"\n";
	s += "@map font 6 to \"Helvetica-Bold\", \"Helvetica-Bold\"\n";
	s += "@map font 7 to \"Helvetica-BoldOblique\", \"Helvetica-BoldOblique\"\n";
	s += "@map font 15 to \"Helvetica-Narrow\", \"Helvetica-Narrow\"\n";
	s += "@map font 16 to \"Helvetica-Narrow-Bold\", \"Helvetica-Narrow-Bold\"\n";
	s += "@map font 17 to \"Helvetica-Narrow-BoldOblique\", \"Helvetica-Narrow-BoldOblique\"\n";
	s += "@map font 18 to \"Helvetica-Narrow-Oblique\", \"Helvetica-Narrow-Oblique\"\n";
	s += "@map font 5 to \"Helvetica-Oblique\", \"Helvetica-Oblique\"\n";
	s += "@map font 20 to \"NewCenturySchlbk-Bold\", \"NewCenturySchlbk-Bold\"\n";
	s += "@map font 21 to \"NewCenturySchlbk-BoldItalic\", \"NewCenturySchlbk-BoldItalic\"\n";
	s += "@map font 22 to \"NewCenturySchlbk-Italic\", \"NewCenturySchlbk-Italic\"\n";
	s += "@map font 23 to \"NewCenturySchlbk-Roman\", \"NewCenturySchlbk-Roman\"\n";
	s += "@map font 24 to \"Palatino-Bold\", \"Palatino-Bold\"\n";
	s += "@map font 25 to \"Palatino-BoldItalic\", \"Palatino-BoldItalic\"\n";
	s += "@map font 26 to \"Palatino-Italic\", \"Palatino-Italic\"\n";
	s += "@map font 27 to \"Palatino-Roman\", \"Palatino-Roman\"\n";
	s += "@map font 12 to \"Symbol\", \"Symbol\"\n";
	s += "@map font 2 to \"Times-Bold\", \"Times-Bold\"\n";
	s += "@map font 3 to \"Times-BoldItalic\", \"Times-BoldItalic\"\n";
	s += "@map font 1 to \"Times-Italic\", \"Times-Italic\"\n";
	s += "@map font 0 to \"Times-Roman\", \"Times-Roman\"\n";
	s += "@map font 33 to \"ZapfChancery-MediumItalic\", \"ZapfChancery-MediumItalic\"\n";
	s += "@map font 13 to \"ZapfDingbats\", \"ZapfDingbats\"\n";
	s += "@map font 35 to \"CharterBT-Bold\", \"CharterBT-Bold\"\n";
	s += "@map font 36 to \"CharterBT-BoldItalic\", \"CharterBT-BoldItalic\"\n";
	s += "@map font 37 to \"CharterBT-Italic\", \"CharterBT-Italic\"\n";
	s += "@map font 38 to \"CharterBT-Roman\", \"CharterBT-Roman\"\n";
	s += "@map font 41 to \"Courier-BoldItalic\", \"Courier-BoldItalic\"\n";
	s += "@map font 42 to \"Courier-Italic\", \"Courier-Italic\"\n";
	s += "@map font 43 to \"Hershey-Gothic-English\", \"Hershey-Gothic-English\"\n";
	s += "@map font 44 to \"Hershey-Gothic-German\", \"Hershey-Gothic-German\"\n";
	s += "@map font 45 to \"Hershey-Gothic-Italian\", \"Hershey-Gothic-Italian\"\n";
	s += "@map font 46 to \"Hershey-Plain-Duplex\", \"Hershey-Plain-Duplex\"\n";
	s += "@map font 47 to \"Hershey-Plain-Duplex-Italic\", \"Hershey-Plain-Duplex-Italic\"\n";
	s += "@map font 48 to \"Hershey-Plain-Triplex\", \"Hershey-Plain-Triplex\"\n";
	s += "@map font 49 to \"Hershey-Plain-Triplex-Italic\", \"Hershey-Plain-Triplex-Italic\"\n";
	s += "@map font 50 to \"Hershey-Script-Complex\", \"Hershey-Script-Complex\"\n";
	s += "@map font 51 to \"Hershey-Script-Simplex\", \"Hershey-Script-Simplex\"\n";
	s += "@map font 52 to \"LuxiMono\", \"LuxiMono\"\n";
	s += "@map font 53 to \"LuxiMono-Bold\", \"LuxiMono-Bold\"\n";
	s += "@map font 54 to \"LuxiMono-BoldOblique\", \"LuxiMono-BoldOblique\"\n";
	s += "@map font 55 to \"LuxiMono-Oblique\", \"LuxiMono-Oblique\"\n";
	s += "@map font 56 to \"LuxiSans\", \"LuxiSans\"\n";
	s += "@map font 57 to \"LuxiSans-Bold\", \"LuxiSans-Bold\"\n";
	s += "@map font 58 to \"LuxiSans-BoldOblique\", \"LuxiSans-BoldOblique\"\n";
	s += "@map font 59 to \"LuxiSans-Oblique\", \"LuxiSans-Oblique\"\n";
	s += "@map font 60 to \"LuxiSerif\", \"LuxiSerif\"\n";
	s += "@map font 61 to \"LuxiSerif-Bold\", \"LuxiSerif-Bold\"\n";
	s += "@map font 62 to \"LuxiSerif-BoldOblique\", \"LuxiSerif-BoldOblique\"\n";
	s += "@map font 63 to \"LuxiSerif-Oblique\", \"LuxiSerif-Oblique\"\n";
	s += "@map font 64 to \"Utopia-Bold\", \"Utopia-Bold\"\n";
	s += "@map font 65 to \"Utopia-BoldItalic\", \"Utopia-BoldItalic\"\n";
	s += "@map font 66 to \"Utopia-Italic\", \"Utopia-Italic\"\n";
	s += "@map font 67 to \"Utopia-Regular\", \"Utopia-Regular\"\n";
	s += "@map color 0 to (255, 255, 255), \"white\"\n";
	s += "@map color 1 to (0, 0, 0), \"black\"\n";
	s += "@map color 2 to (255, 0, 0), \"red\"\n";
	s += "@map color 3 to (0, 255, 0), \"green\"\n";
	s += "@map color 4 to (0, 0, 255), \"blue\"\n";
	s += "@map color 5 to (255, 255, 0), \"yellow\"\n";
	s += "@map color 6 to (188, 143, 143), \"brown\"\n";
	s += "@map color 7 to (220, 220, 220), \"grey\"\n";
	s += "@map color 8 to (148, 0, 211), \"violet\"\n";
	s += "@map color 9 to (0, 255, 255), \"cyan\"\n";
	s += "@map color 10 to (255, 0, 255), \"magenta\"\n";
	s += "@map color 11 to (255, 165, 0), \"orange\"\n";
	s += "@map color 12 to (114, 33, 188), \"indigo\"\n";
	s += "@map color 13 to (103, 7, 72), \"maroon\"\n";
	s += "@map color 14 to (64, 224, 208), \"turquoise\"\n";
	s += "@map color 15 to (0, 139, 0), \"green4\"\n";
	s += "@reference date 0\n";
	s += "@date wrap off\n";
	s += "@date wrap year 1950\n";
	s += "@default linewidth 1.0\n";
	s += "@default linestyle 1\n";
	s += "@default color 1\n";
	s += "@default pattern 1\n";
	s += "@default font 0\n";
	s += "@default char size 1.000000\n";
	s += "@default symbol size 1.000000\n";
	s += "@default sformat \"\%.8g\"\n";
	s += "@background color 0\n";
	s += "@page background fill on\n";
	s += "@timestamp off\n";
	s += "@timestamp 0.03, 0.03\n";
	s += "@timestamp color 1\n";
	s += "@timestamp rot 0\n";
	s += "@timestamp font 0\n";
	s += "@timestamp char size 1.000000\n";
	s += "@timestamp def \"Sun Jan 21 12:56:47 2007\"\n";
	s += "@r0 off\n";
	s += "@link r0 to g0\n";
	s += "@r0 type above\n";
	s += "@r0 linestyle 1\n";
	s += "@r0 linewidth 1.0\n";
	s += "@r0 color 1\n";
	s += "@r0 line 0, 0, 0, 0\n";
	s += "@r1 off\n";
	s += "@link r1 to g0\n";
	s += "@r1 type above\n";
	s += "@r1 linestyle 1\n";
	s += "@r1 linewidth 1.0\n";
	s += "@r1 color 1\n";
	s += "@r1 line 0, 0, 0, 0\n";
	s += "@r2 off\n";
	s += "@link r2 to g0\n";
	s += "@r2 type above\n";
	s += "@r2 linestyle 1\n";
	s += "@r2 linewidth 1.0\n";
	s += "@r2 color 1\n";
	s += "@r2 line 0, 0, 0, 0\n";
	s += "@r3 off\n";
	s += "@link r3 to g0\n";
	s += "@r3 type above\n";
	s += "@r3 linestyle 1\n";
	s += "@r3 linewidth 1.0\n";
	s += "@r3 color 1\n";
	s += "@r3 line 0, 0, 0, 0\n";
	s += "@r4 off\n";
	s += "@link r4 to g0\n";
	s += "@r4 type above\n";
	s += "@r4 linestyle 1\n";
	s += "@r4 linewidth 1.0\n";
	s += "@r4 color 1\n";
	s += "@r4 line 0, 0, 0, 0\n";
	s += "@g0 on\n";
	s += "@g0 hidden false\n";
	s += "@g0 type XY\n";
	s += "@g0 stacked false\n";
	s += "@g0 bar hgap 0.000000\n";
	s += "@g0 fixedpoint off\n";
	s += "@g0 fixedpoint type 0\n";
	s += "@g0 fixedpoint xy 0.000000, 0.000000\n";
	s += "@g0 fixedpoint format general general\n";
	s += "@g0 fixedpoint prec 6, 6\n";
	s += "@with g0\n";
	s += "@    world 20, -90, 20000, 0\n";
	s += "@    stack world 0, 0, 0, 0\n";
	s += "@    znorm 1\n";
	s += "@    view 0.130000, 0.120000, 1.300000, 0.920000\n";
	s += "@    title \"\"\n";
	s += "@    title font 0\n";
	s += "@    title size 1.500000\n";
	s += "@    title color 1\n";
	s += "@    subtitle \"\"\n";
	s += "@    subtitle font 0\n";
	s += "@    subtitle size 1.000000\n";
	s += "@    subtitle color 1\n";
	s += "@    xaxes scale Logarithmic\n";
	s += "@    yaxes scale Normal\n";
	s += "@    xaxes invert off\n";
	s += "@    yaxes invert off\n";
	s += "@    xaxis  on\n";
	s += "@    xaxis  type zero false\n";
	s += "@    xaxis  offset 0.000000 , 0.000000\n";
	s += "@    xaxis  bar on\n";
	s += "@    xaxis  bar color 1\n";
	s += "@    xaxis  bar linestyle 1\n";
	s += "@    xaxis  bar linewidth 1.0\n";
	s += "@    xaxis  label \"Frequency (Hz)\"\n";
	s += "@    xaxis  label layout para\n";
	s += "@    xaxis  label place auto\n";
	s += "@    xaxis  label char size 1.000000\n";
	s += "@    xaxis  label font 0\n";
	s += "@    xaxis  label color 1\n";
	s += "@    xaxis  label place normal\n";
	s += "@    xaxis  tick on\n";
	s += "@    xaxis  tick major 10\n";
	s += "@    xaxis  tick minor ticks 9\n";
	s += "@    xaxis  tick default 6\n";
	s += "@    xaxis  tick place rounded true\n";
	s += "@    xaxis  tick in\n";
	s += "@    xaxis  tick major size 0.500000\n";
	s += "@    xaxis  tick major color 1\n";
	s += "@    xaxis  tick major linewidth 1.0\n";
	s += "@    xaxis  tick major linestyle 1\n";
	s += "@    xaxis  tick major grid on\n";
	s += "@    xaxis  tick minor color 1\n";
	s += "@    xaxis  tick minor linewidth 1.0\n";
	s += "@    xaxis  tick minor linestyle 2\n";
	s += "@    xaxis  tick minor grid on\n";
	s += "@    xaxis  tick minor size 0.250000\n";
	s += "@    xaxis  ticklabel on\n";
	s += "@    xaxis  ticklabel format general\n";
	s += "@    xaxis  ticklabel prec 5\n";
	s += "@    xaxis  ticklabel formula \"\"\n";
	s += "@    xaxis  ticklabel append \"\"\n";
	s += "@    xaxis  ticklabel prepend \"\"\n";
	s += "@    xaxis  ticklabel angle 0\n";
	s += "@    xaxis  ticklabel skip 0\n";
	s += "@    xaxis  ticklabel stagger 0\n";
	s += "@    xaxis  ticklabel place normal\n";
	s += "@    xaxis  ticklabel offset auto\n";
	s += "@    xaxis  ticklabel offset 0.000000 , 0.010000\n";
	s += "@    xaxis  ticklabel start type auto\n";
	s += "@    xaxis  ticklabel start 0.000000\n";
	s += "@    xaxis  ticklabel stop type auto\n";
	s += "@    xaxis  ticklabel stop 0.000000\n";
	s += "@    xaxis  ticklabel char size 0.750000\n";
	s += "@    xaxis  ticklabel font 0\n";
	s += "@    xaxis  ticklabel color 1\n";
	s += "@    xaxis  tick place both\n";
	s += "@    xaxis  tick spec type both\n";
	s += "@    xaxis  tick spec 37\n";
	s += "@    xaxis  tick major 0, 10\n";
	s += "@    xaxis  ticklabel 0, \"10\"\n";
	s += "@    xaxis  tick major 1, 20\n";
	s += "@    xaxis  ticklabel 1, \"20\"\n";
	s += "@    xaxis  tick minor 2, 30\n";
	s += "@    xaxis  tick minor 3, 40\n";
	s += "@    xaxis  tick major 4, 50\n";
	s += "@    xaxis  ticklabel 4, \"50\"\n";
	s += "@    xaxis  tick minor 5, 60\n";
	s += "@    xaxis  tick minor 6, 70\n";
	s += "@    xaxis  tick minor 7, 80\n";
	s += "@    xaxis  tick minor 8, 90\n";
	s += "@    xaxis  tick major 9, 100\n";
	s += "@    xaxis  ticklabel 9, \"100\"\n";
	s += "@    xaxis  tick major 10, 200\n";
	s += "@    xaxis  ticklabel 10, \"200\"\n";
	s += "@    xaxis  tick minor 11, 300\n";
	s += "@    xaxis  tick minor 12, 400\n";
	s += "@    xaxis  tick major 13, 500\n";
	s += "@    xaxis  ticklabel 13, \"500\"\n";
	s += "@    xaxis  tick minor 14, 600\n";
	s += "@    xaxis  tick minor 15, 700\n";
	s += "@    xaxis  tick minor 16, 800\n";
	s += "@    xaxis  tick minor 17, 900\n";
	s += "@    xaxis  tick major 18, 1000\n";
	s += "@    xaxis  ticklabel 18, \"1000\"\n";
	s += "@    xaxis  tick major 19, 2000\n";
	s += "@    xaxis  ticklabel 19, \"2000\"\n";
	s += "@    xaxis  tick minor 20, 3000\n";
	s += "@    xaxis  tick minor 21, 4000\n";
	s += "@    xaxis  tick major 22, 5000\n";
	s += "@    xaxis  ticklabel 22, \"5000\"\n";
	s += "@    xaxis  tick minor 23, 6000\n";
	s += "@    xaxis  tick minor 24, 7000\n";
	s += "@    xaxis  tick minor 25, 8000\n";
	s += "@    xaxis  tick minor 26, 9000\n";
	s += "@    xaxis  tick major 27, 10000\n";
	s += "@    xaxis  ticklabel 27, \"10000\"\n";
	s += "@    xaxis  tick major 28, 20000\n";
	s += "@    xaxis  ticklabel 28, \"20000\"\n";
	s += "@    xaxis  tick minor 29, 30000\n";
	s += "@    xaxis  tick minor 30, 40000\n";
	s += "@    xaxis  tick minor 31, 50000\n";
	s += "@    xaxis  tick minor 32, 60000\n";
	s += "@    xaxis  tick minor 33, 70000\n";
	s += "@    xaxis  tick minor 34, 80000\n";
	s += "@    xaxis  tick minor 35, 90000\n";
	s += "@    xaxis  tick minor 36, 100000\n";
	s += "@    xaxis  ticklabel 36, \"1e+05\"\n";
	s += "@    yaxis  on\n";
	s += "@    yaxis  type zero false\n";
	s += "@    yaxis  offset 0.000000 , 0.000000\n";
	s += "@    yaxis  bar on\n";
	s += "@    yaxis  bar color 1\n";
	s += "@    yaxis  bar linestyle 1\n";
	s += "@    yaxis  bar linewidth 1.0\n";
	s += "@    yaxis  label \"Level (dB)\"\n";
	s += "@    yaxis  label layout para\n";
	s += "@    yaxis  label place auto\n";
	s += "@    yaxis  label char size 1.000000\n";
	s += "@    yaxis  label font 0\n";
	s += "@    yaxis  label color 1\n";
	s += "@    yaxis  label place normal\n";
	s += "@    yaxis  tick on\n";
	s += "@    yaxis  tick major 10\n";
	s += "@    yaxis  tick minor ticks 1\n";
	s += "@    yaxis  tick default 6\n";
	s += "@    yaxis  tick place rounded true\n";
	s += "@    yaxis  tick in\n";
	s += "@    yaxis  tick major size 0.500000\n";
	s += "@    yaxis  tick major color 1\n";
	s += "@    yaxis  tick major linewidth 1.0\n";
	s += "@    yaxis  tick major linestyle 1\n";
	s += "@    yaxis  tick major grid on\n";
	s += "@    yaxis  tick minor color 1\n";
	s += "@    yaxis  tick minor linewidth 1.0\n";
	s += "@    yaxis  tick minor linestyle 2\n";
	s += "@    yaxis  tick minor grid on\n";
	s += "@    yaxis  tick minor size 0.250000\n";
	s += "@    yaxis  ticklabel on\n";
	s += "@    yaxis  ticklabel format general\n";
	s += "@    yaxis  ticklabel prec 5\n";
	s += "@    yaxis  ticklabel formula \"\"\n";
	s += "@    yaxis  ticklabel append \"\"\n";
	s += "@    yaxis  ticklabel prepend \"\"\n";
	s += "@    yaxis  ticklabel angle 0\n";
	s += "@    yaxis  ticklabel skip 0\n";
	s += "@    yaxis  ticklabel stagger 0\n";
	s += "@    yaxis  ticklabel place normal\n";
	s += "@    yaxis  ticklabel offset auto\n";
	s += "@    yaxis  ticklabel offset 0.000000 , 0.010000\n";
	s += "@    yaxis  ticklabel start type auto\n";
	s += "@    yaxis  ticklabel start 0.000000\n";
	s += "@    yaxis  ticklabel stop type auto\n";
	s += "@    yaxis  ticklabel stop 0.000000\n";
	s += "@    yaxis  ticklabel char size 0.750000\n";
	s += "@    yaxis  ticklabel font 0\n";
	s += "@    yaxis  ticklabel color 1\n";
	s += "@    yaxis  tick place both\n";
	s += "@    yaxis  tick spec type none\n";
	s += "@    altxaxis  off\n";
	s += "@    altyaxis  off\n";
	s += "@    legend on\n";
	s += "@    legend loctype view\n";
	s += "@    legend 0.84941423, 0.8\n";
	s += "@    legend box color 1\n";
	s += "@    legend box pattern 1\n";
	s += "@    legend box linewidth 1.0\n";
	s += "@    legend box linestyle 1\n";
	s += "@    legend box fill color 0\n";
	s += "@    legend box fill pattern 1\n";
	s += "@    legend font 0\n";
	s += "@    legend char size 1.000000\n";
	s += "@    legend color 1\n";
	s += "@    legend length 4\n";
	s += "@    legend vgap 1\n";
	s += "@    legend hgap 1\n";
	s += "@    legend invert false\n";
	s += "@    frame type 0\n";
	s += "@    frame linestyle 1\n";
	s += "@    frame linewidth 1.0\n";
	s += "@    frame color 1\n";
	s += "@    frame pattern 1\n";
	s += "@    frame background color 0\n";
	s += "@    frame background pattern 0\n";
	s += "@    s0 hidden false\n";
	s += "@    s0 type xy\n";
	s += "@    s0 symbol 9\n";
	s += "@    s0 symbol size 0.500000\n";
	s += "@    s0 symbol color 2\n";
	s += "@    s0 symbol pattern 1\n";
	s += "@    s0 symbol fill color 2\n";
	s += "@    s0 symbol fill pattern 0\n";
	s += "@    s0 symbol linewidth 1.0\n";
	s += "@    s0 symbol linestyle 1\n";
	s += "@    s0 symbol char 65\n";
	s += "@    s0 symbol char font 0\n";
	s += "@    s0 symbol skip 0\n";
	s += "@    s0 line type 1\n";
	s += "@    s0 line linestyle 1\n";
	s += "@    s0 line linewidth 2.0\n";
	s += "@    s0 line color 1\n";
	s += "@    s0 line pattern 1\n";
	s += "@    s0 baseline type 0\n";
	s += "@    s0 baseline off\n";
	s += "@    s0 dropline off\n";
	s += "@    s0 fill type 0\n";
	s += "@    s0 fill rule 0\n";
	s += "@    s0 fill color 1\n";
	s += "@    s0 fill pattern 1\n";
	s += "@    s0 avalue off\n";
	s += "@    s0 avalue type 2\n";
	s += "@    s0 avalue char size 1.000000\n";
	s += "@    s0 avalue font 0\n";
	s += "@    s0 avalue color 1\n";
	s += "@    s0 avalue rot 0\n";
	s += "@    s0 avalue format general\n";
	s += "@    s0 avalue prec 3\n";
	s += "@    s0 avalue prepend \"\"\n";
	s += "@    s0 avalue append \"\"\n";
	s += "@    s0 avalue offset 0.000000 , 0.000000\n";
	s += "@    s0 errorbar off\n";
	s += "@    s0 errorbar place both\n";
	s += "@    s0 errorbar color 2\n";
	s += "@    s0 errorbar pattern 1\n";
	s += "@    s0 errorbar size 1.000000\n";
	s += "@    s0 errorbar linewidth 1.0\n";
	s += "@    s0 errorbar linestyle 1\n";
	s += "@    s0 errorbar riser linewidth 1.0\n";
	s += "@    s0 errorbar riser linestyle 1\n";
	s += "@    s0 errorbar riser clip off\n";
	s += "@    s0 errorbar riser clip length 0.100000\n";
	s += "@    s0 comment \"Exported from Traverso\"\n";
	s += "@    s0 legend  \"\"\n";
	s += "@target G0.S0\n";
	s += "@type xy\n";

	int n = qMin(m_map_idx2freq.size(), m_avg_db.size());
	QString str;

	for (int i = 0; i < n; ++i) {
		s += str.sprintf("%.6f %.6f\n", m_map_idx2freq.at(i), m_avg_db.at(i));
	}

	s += "&\n";

	return s;
}


/*******************************************/
/*        SpectralMeterConfWidget          */
/*******************************************/

SpectralMeterConfigWidget::SpectralMeterConfigWidget( QWidget * parent )
	: QDialog(parent)
{
	setupUi(this);
	groupBoxAdvanced->hide();
	
	load_configuration();
	
	connect(buttonAdvanced, SIGNAL(toggled(bool)), this, SLOT(advancedButton_toggled(bool)));
}

void SpectralMeterConfigWidget::on_buttonApply_clicked()
{
	save_configuration();
	emit configChanged();
}

void SpectralMeterConfigWidget::on_buttonClose_clicked( )
{
	hide();
}

void SpectralMeterConfigWidget::advancedButton_toggled(bool b)
{
	if (b) {
		groupBoxAdvanced->show();
	} else {
		groupBoxAdvanced->hide();
	}
}

void SpectralMeterConfigWidget::save_configuration( )
{
	config().set_property(	"SpectralMeter",
				"UpperFrequenty",
				qMax(spinBoxLowerFreq->value(), spinBoxUpperFreq->value()) );
	config().set_property(	"SpectralMeter",
				"LowerFrequenty",
				qMin(spinBoxLowerFreq->value(), spinBoxUpperFreq->value()) );
	config().set_property(	"SpectralMeter",
				"UpperdB",
				qMax(spinBoxUpperDb->value(), spinBoxLowerDb->value()) );
	config().set_property(	"SpectralMeter",
				"LowerdB",
				qMin(spinBoxUpperDb->value(), spinBoxLowerDb->value()) );
	config().set_property("SpectralMeter", "NumberOfBands", spinBoxNumBands->value() );
	config().set_property("SpectralMeter", "ShowAvarage", checkBoxAverage->isChecked() );
	
	config().set_property("SpectralMeter", "FFTSize", comboBoxFftSize->currentText().toInt() );
	config().set_property("SpectralMeter", "WindowingFunction", comboBoxWindowing->currentIndex() );
}

void SpectralMeterConfigWidget::load_configuration( )
{
	int value;
	value = config().get_property("SpectralMeter", "UpperFrequenty", 22050).toInt();
	spinBoxUpperFreq->setValue(value);
	value = config().get_property("SpectralMeter", "LowerFrequenty", 20).toInt();
	spinBoxLowerFreq->setValue(value);
	value = config().get_property("SpectralMeter", "UpperdB", 0).toInt();
	spinBoxUpperDb->setValue(value);
	value = config().get_property("SpectralMeter", "LowerdB", -90).toInt();
	spinBoxLowerDb->setValue(value);
	value = config().get_property("SpectralMeter", "NumberOfBands", 16).toInt();
	spinBoxNumBands->setValue(value);
	value = config().get_property("SpectralMeter", "ShowAvarage", 0).toInt();
	checkBoxAverage->setChecked(value);
	value = config().get_property("SpectralMeter", "FFTSize", 2048).toInt();
	QString str;
	str = QString("%1").arg(value);
	int idx = comboBoxFftSize->findText(str);
	idx = idx == -1 ? 3 : idx;
	comboBoxFftSize->setCurrentIndex(idx);
	value = config().get_property("SpectralMeter", "WindowingFunction", 1).toInt();
	comboBoxWindowing->setCurrentIndex(value);
}

//eof
