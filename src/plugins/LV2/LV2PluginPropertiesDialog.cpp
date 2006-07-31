/*
Copyright (C) 2006 Remon Sijrier

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

$Id: LV2PluginPropertiesDialog.cpp,v 1.1 2006/07/31 13:24:46 r_sijrier Exp $
*/


#include "LV2PluginPropertiesDialog.h"
#include "LV2Plugin.h"
#include "LV2ControlPort.h"

#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include <PluginSlider.h>


LV2PluginPropertiesDialog::LV2PluginPropertiesDialog(LV2Plugin* plugin)
	: QDialog(), m_plugin(plugin)
{
	setup_gui();
}

void LV2PluginPropertiesDialog::setup_gui()
{
	QList<LV2ControlPort* > ports = m_plugin->get_control_ports();

	QWidget* sliderWidget = new QWidget(this);
	QVBoxLayout* sliderWidgetLayout = new QVBoxLayout;
	sliderWidget->setLayout(sliderWidgetLayout);

	QHBoxLayout* dialogLayout = new QHBoxLayout;
	dialogLayout->addWidget(sliderWidget);
	this->setLayout(dialogLayout);


	resize(400, 300);

	foreach(LV2ControlPort* port, ports) {

		QWidget* widget = new QWidget(sliderWidget);
		QHBoxLayout *layout = new QHBoxLayout;

		PluginSlider* slider = new PluginSlider(Qt::Horizontal);
		slider->setFixedWidth(200);
		slider->set_minimum(port->get_min_control_value());
		slider->set_maximum(port->get_max_control_value());
		slider->set_value(port->get_control_value());
		connect(slider, SIGNAL(sliderValueChanged(double )), port, SLOT(set_control_value(double )));

		QLabel* minvalue = new QLabel();
		minvalue->setNum(port->get_min_control_value());
		minvalue->setFixedWidth(20);

		QLabel* maxvalue = new QLabel();
		maxvalue->setNum(port->get_max_control_value());
		maxvalue->setFixedWidth(30);

		QLabel* currentvalue = new QLabel();
		currentvalue->setNum(port->get_control_value());
		currentvalue->setFixedWidth(30);

		connect(slider, SIGNAL(sliderValueChanged(double )), currentvalue, SLOT(setNum (double )));

		QLabel* controlname = new QLabel(port->get_description());
		controlname->setFixedWidth(90);

		layout->addWidget(minvalue);
		layout->addWidget(slider, 4);
		layout->addWidget(maxvalue);
		layout->addWidget(currentvalue);
		layout->addWidget(controlname);

		widget->setLayout(layout);

		sliderWidgetLayout->addWidget(widget);
	}

}

//eof
