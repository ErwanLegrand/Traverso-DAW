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

$Id: SpectralMeterConfigWidget.cpp,v 1.2 2006/12/30 14:01:01 n_doebelin Exp $
*/

#include "SpectralMeterConfigWidget.h"
#include "ui_SpectralMeterConfigWidget.h"

#include <QSpinBox>
#include <QComboBox>
#include <QString>
#include <QGroupBox>
#include <QCheckBox>

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"

SpectralMeterConfigWidget::SpectralMeterConfigWidget( QWidget * parent )
	: QWidget(parent, Qt::Popup)
{
	setupUi(this);
	groupBoxAdvanced->hide();
	connect(buttonClose, SIGNAL(clicked()), this, SLOT(on_closeButton_clicked()));
	connect(buttonAdvanced, SIGNAL(toggled(bool)), this, SLOT(advancedButton_toggled(bool)));
}

SpectralMeterConfigWidget::~ SpectralMeterConfigWidget( )
{}

void SpectralMeterConfigWidget::on_closeButton_clicked()
{
	emit closed();
}

void SpectralMeterConfigWidget::advancedButton_toggled(bool b)
{
	if (b) {
		groupBoxAdvanced->show();
	} else {
		groupBoxAdvanced->hide();
	}
}

void SpectralMeterConfigWidget::set_upper_freq(int i)
{
	spinBoxUpperFreq->setValue(i);
}

void SpectralMeterConfigWidget::set_lower_freq(int i)
{
	spinBoxLowerFreq->setValue(i);
}

void SpectralMeterConfigWidget::set_upper_db(int i)
{
	spinBoxUpperDb->setValue(i);
}

void SpectralMeterConfigWidget::set_lower_db(int i)
{
	spinBoxLowerDb->setValue(i);
}

void SpectralMeterConfigWidget::set_num_bands(int i)
{
	spinBoxNumBands->setValue(i);
}

void SpectralMeterConfigWidget::set_show_average(bool b)
{
	checkBoxAverage->setChecked(b);
}

bool SpectralMeterConfigWidget::get_show_average()
{
	return checkBoxAverage->isChecked();
}

void SpectralMeterConfigWidget::set_fr_len(int i)
{
	QString str;
	str = QString("%1").arg(i);
	int idx = comboBoxFftSize->findText(str);

	idx = idx == -1 ? 3 : idx;
	comboBoxFftSize->setCurrentIndex(idx);
}

void SpectralMeterConfigWidget::set_windowing_function(int i)
{
	comboBoxWindowing->setCurrentIndex(i);
}

int SpectralMeterConfigWidget::get_upper_freq()
{
	int i = qMax(spinBoxLowerFreq->value(), spinBoxUpperFreq->value());
	return i;
}

int SpectralMeterConfigWidget::get_lower_freq()
{
	int i = qMin(spinBoxLowerFreq->value(), spinBoxUpperFreq->value());
	return i;
}

int SpectralMeterConfigWidget::get_upper_db()
{
	int i = qMax(spinBoxUpperDb->value(), spinBoxLowerDb->value());
	return i;
}

int SpectralMeterConfigWidget::get_lower_db()
{
	int i = qMin(spinBoxUpperDb->value(), spinBoxLowerDb->value());
	return i;
}

int SpectralMeterConfigWidget::get_num_bands()
{
	return spinBoxNumBands->value();
}

int SpectralMeterConfigWidget::get_fr_len()
{
	QString str = comboBoxFftSize->currentText();
	return str.toInt();
}

int SpectralMeterConfigWidget::get_windowing_function()
{
	return comboBoxWindowing->currentIndex();
}


//eof
 
