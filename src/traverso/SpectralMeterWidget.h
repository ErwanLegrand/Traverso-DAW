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
 
*/

#ifndef SPECTRALMETERWIDGET_H
#define SPECTRALMETERWIDGET_H

#include <ViewPort.h>
#include <ViewItem.h>
#include <QTimer>
#include <QVector>
#include "ui_SpectralMeterConfigWidget.h"
#include <QDialog>

class Song;
class Project;
class QRect;
class QPixmap;
class SpectralMeter;
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

class SpectralMeterWidget : public ViewPort
{
public:
        SpectralMeterWidget(QWidget* parent);
	~SpectralMeterWidget();

	void get_pointed_context_items(QList<ContextItem* > &list);

protected:
        void resizeEvent( QResizeEvent* e);
	void hideEvent ( QHideEvent * event );
	void showEvent ( QShowEvent * event );
	QSize minimumSizeHint () const;
	QSize sizeHint () const;
	
private:
	SpectralMeterView* m_item;
};


class SpectralMeterView : public ViewItem
{
	Q_OBJECT
	
	Q_CLASSINFO("edit_properties", tr("Settings..."))
	Q_CLASSINFO("set_mode", tr("Toggle avarage curve"))
	Q_CLASSINFO("reset", tr("Reset average curve"))
	Q_CLASSINFO("export_avarage_curve", tr("Export avarage curve"))
	Q_CLASSINFO("screen_capture", tr("Capture Screen"))
	

public:
        SpectralMeterView(SpectralMeterWidget* widget);
	~SpectralMeterView();
	
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	void resize();
	void hide_event();
	void show_event();

private:
	SpectralMeterWidget* m_widget;
	SpectralMeter*	m_meter;
	Project*	m_project;
	Song*		m_song;
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

	void		reduce_bands();
	void		update_layout();
	void		update_freq_map();
	float		db2ypos(float);
	float		freq2xpos(float);
	void		update_background();
	float		freq2db(float, float);
	QString		get_xmgr_string();


private slots:
	void		set_project( Project* );
	void		update_data();
	void		transfer_started();
	void		transfer_stopped();


public slots:
	void		load_configuration();
	void		set_song( Song* );
	
	Command*	edit_properties();
	Command*	set_mode();
	Command*	reset();
	Command*	export_avarage_curve();
	Command*	screen_capture();
};

#endif

