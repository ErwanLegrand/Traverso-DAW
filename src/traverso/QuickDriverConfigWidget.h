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
 
    $Id: QuickDriverConfigWidget.h,v 1.1 2006/10/18 12:09:47 r_sijrier Exp $
*/

#ifndef QUICK_DRIVERCONFIG_WIDGET_H
#define QUICK_DRIVERCONFIG_WIDGET_H

#include "ui_QuickDriverConfigWidget.h"
#include <QWidget>
#include <QList>

class QuickDriverConfigWidget : public QWidget, private Ui::QuickDriverConfigWidget
{
	Q_OBJECT

public:
	QuickDriverConfigWidget(QWidget* parent = 0);
	~QuickDriverConfigWidget();

private:
	QList<int>	periodBufferSizesList;
	
	void update_latency_combobox();
        void update_driver_info();
	
private slots:
	void on_applyButton_clicked();
	void on_saveButton_clicked();
	
	void driver_combobox_index_changed(QString);
	void rate_combobox_index_changed(QString);
	void driver_params_changed();
};

#endif

//eof


 
