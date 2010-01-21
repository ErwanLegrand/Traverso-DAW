/*
    Copyright (C) 2008 Remon Sijrier
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
 
*/

#ifndef SPECTRALMETERWIDGET_H
#define SPECTRALMETERWIDGET_H

#include "MeterWidget.h"

#include <QVector>
#include "ui_SpectralMeterConfigWidget.h"
#include <QDialog>

class Sheet;
class Project;
class QRect;
class QPixmap;
class Command;
class SpectralMeterView;


class SpectralMeterConfigWidget : public QDialog, private Ui::SpectralMeterConfigWidget
{
	Q_OBJECT

public:
	SpectralMeterConfigWidget(QWidget* parent = 0);

private:
	void save_configuration();
	void load_configuration();
	
private slots:
	void on_buttonClose_clicked();
	void on_buttonApply_clicked();
	void advancedButton_toggled(bool);

signals:
	void configChanged();

};

class SpectralMeterWidget : public MeterWidget
{
public:
        SpectralMeterWidget(QWidget* parent);

private:
	SpectralMeterView* get_item();
};


class SpectralMeterView : public MeterView
{
	Q_OBJECT
	
	Q_CLASSINFO("edit_properties", tr("Settings..."))
        Q_CLASSINFO("set_mode", tr("Toggle average curve"))
	Q_CLASSINFO("reset", tr("Reset average curve"))
        Q_CLASSINFO("export_average_curve", tr("Export average curve"))
	Q_CLASSINFO("screen_capture", tr("Capture Screen"))
	

public:
        SpectralMeterView(SpectralMeterWidget* widget);
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual void resize();

private:
	QVector<float>	specl;
	QVector<float>	specr;
	QVector<float>	m_spectrum;
	QVector<float>	m_history;
	QVector<float>	m_bands;
	QVector<float>	m_freq_labels;
	QVector<float>	m_avg_db;
	QVector<float>	m_map_idx2xpos;
	QVector<float>	m_map_idx2freq;
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
	int		bar_offset;
	bool		show_average;
	bool		update_average;

	void		reduce_bands();
	void		update_layout();
	void		update_freq_map();
	float		db2ypos(float);
	float		freq2xpos(float);
	void		update_background();
	float		freq2db(float, float);
	QString		get_xmgr_string();
	QBrush		m_brushBg;
	QBrush		m_brushFg;
	QBrush		m_brushMargin;
	QPen		m_penAvgCurve;
	QPen		m_penTickMain;
	QPen		m_penTickSub;
	QPen		m_penFont;
	QPen		m_penGrid;

private slots:
	void		update_data();
	void		load_theme_data();

public slots:
	void		load_configuration();
	void		set_sheet(Sheet* sheet);

	Command*	edit_properties();
	Command*	set_mode();
	Command*	reset();
	Command*	export_average_curve();
	Command*	screen_capture();
};

#endif

