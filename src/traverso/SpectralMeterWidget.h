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
 
    $Id: SpectralMeterWidget.h,v 1.7 2006/12/30 14:01:01 n_doebelin Exp $
*/

#ifndef SPECTRALMETERWIDGET_H
#define SPECTRALMETERWIDGET_H

#include <ViewPort.h>
#include <QTimer>
#include <QVector>

class Song;
class Project;
class QRect;
class QPixmap;
class SpectralMeter;
class Command;
class SpectralMeterConfigWidget;

class SpectralMeterWidget : public ViewPort
{
	Q_OBJECT

public:
        SpectralMeterWidget(QWidget* parent);
	~SpectralMeterWidget();

protected:
        void resizeEvent( QResizeEvent* e);
        void paintEvent( QPaintEvent* e);

private:
	Project		*m_project;
	QTimer		timer;
	QVector<float>	specl;
	QVector<float>	specr;
	QVector<float>	m_spectrum;
	QVector<float>	m_history;
	QVector<float>	m_bands;
	QVector<float>	m_freq_labels;
	QVector<float>	m_avg_db;
	QVector<float>	m_map_idx2xpos;
	QVector<float>	m_map_idx2freq;
	SpectralMeter*	m_meter;
	QRect		m_rect;
	SpectralMeterConfigWidget *m_config;
	QPixmap		bgPixmap;
	uint		num_bands;
	uint		sample_rate;
	float		upper_freq;
	float		lower_freq;
	float		upper_db;
	float		lower_db;
	int		margin_l;
	int		margin_r;
	int		margin_t;
	int		margin_b;
	uint		sample_weight;

	uint		fft_size;
	float		xfactor;
	float		upper_freq_log;
	float		lower_freq_log;
	float		freq_step;
	int		bar_width;
	int		bar_offset;
	bool		show_average;

	void		reduce_bands();
	void		update_layout();
	void		update_freq_map();
	float		db2ypos(float);
	float		freq2xpos(float);
	void		update_barwidth();
	void		update_background();

private slots:
	void		set_project( Project* );
	void		set_song( Song* );
	void		update_data();
	void		transfer_started();
	void		transfer_stopped();

public slots:
	Command*	edit_properties();
	Command*	set_mode();
	Command*	reset();
	Command*	show_export_widget();
	void		apply_properties();

};

#endif

