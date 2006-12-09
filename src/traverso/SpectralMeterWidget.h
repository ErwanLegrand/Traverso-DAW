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
 
    $Id: SpectralMeterWidget.h,v 1.1 2006/12/09 08:44:54 n_doebelin Exp $
*/

#ifndef SPECTRALMETERWIDGET_H
#define SPECTRALMETERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include "SpectralMeter.h"

class Song;
class Project;
class QLinearGradient;

class SpectralMeterWidget : public QWidget
{
	Q_OBJECT

public:
        SpectralMeterWidget(QWidget* parent);

protected:
        void resizeEvent( QResizeEvent* e);
        void paintEvent( QPaintEvent* e);

private:
	QTimer		timer;
	QVector<float>	specl;
	QVector<float>	specr;
	QVector<float>	m_spectrum;
	QVector<float>	m_history;
	QVector<float>	m_bands;
	SpectralMeter*	m_meter;
	uint		num_bands;
	uint		sample_rate;
	uint		upper_freq;
	uint		lower_freq;
	int		upper_db;
	int		lower_db;

	void		reduce_bands();
	void		update_band_lut();

private slots:
	void		set_project( Project* );
	void		set_song( Song* );
	void		update_data();

};

#endif

