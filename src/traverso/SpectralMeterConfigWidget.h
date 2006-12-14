/*
    Copyright (C) 2005-2006 Remon Sijrier 
 
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
 
    $Id: SpectralMeterConfigWidget.h,v 1.1 2006/12/14 21:21:19 n_doebelin Exp $
*/

#ifndef SPECTRALMETER_CONFIG_WIDGET_H
#define SPECTRALMETER_CONFIG_WIDGET_H

#include "ui_SpectralMeterConfigWidget.h"
#include <QWidget>

class SpectralMeterConfigWidget : public QWidget, private Ui::SpectralMeterConfigWidget
{
	Q_OBJECT

public:
	SpectralMeterConfigWidget(QWidget* parent = 0);
	~SpectralMeterConfigWidget();

	void set_upper_freq(int);
	void set_lower_freq(int);
	void set_upper_db(int);
	void set_lower_db(int);
	void set_num_bands(int);
	void set_fr_len(int);
	void set_windowing_function(int);

	int get_upper_freq();
	int get_lower_freq();
	int get_upper_db();
	int get_lower_db();
	int get_num_bands();
	int get_fr_len();
	int get_windowing_function();

private:
	
private slots:
	void on_closeButton_clicked();

signals:
	void closed();

};

#endif

//eof


 
