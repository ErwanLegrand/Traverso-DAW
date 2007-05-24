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

$Id: LV2PluginPropertiesDialog.cpp,v 1.6 2007/05/24 10:56:43 r_sijrier Exp $
*/


#include "LV2PluginPropertiesDialog.h"
#include "LV2Plugin.h"

#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>

#include <PluginSlider.h>
#include "Command.h"

LV2PluginPropertiesDialog::LV2PluginPropertiesDialog(LV2Plugin* plugin)
	: QDialog()
	, m_plugin(plugin)
{
	setModal(true);
	
	QWidget* sliderWidget = new QWidget(this);
	QVBoxLayout* sliderWidgetLayout = new QVBoxLayout;
	sliderWidget->setLayout(sliderWidgetLayout);

	QWidget* optionsWidget = new QWidget(this);
	QHBoxLayout* optionsLayout = new QHBoxLayout;
	optionsWidget->setLayout(optionsLayout);
	QPushButton* bypassButton = new QPushButton(tr("Bypass"), optionsWidget);
	QPushButton* closeButton = new QPushButton(tr("Close"), optionsWidget);
	optionsLayout->addWidget(bypassButton);
	optionsLayout->addStretch(10);
	optionsLayout->addWidget(closeButton);
	optionsLayout->setSpacing(0);
	
	QVBoxLayout* dialogLayout = new QVBoxLayout;
	dialogLayout->addWidget(sliderWidget);
	dialogLayout->addWidget(optionsWidget);
	dialogLayout->setMargin(0);
	setLayout(dialogLayout);


	foreach(LV2ControlPort* port, m_plugin->get_control_ports()) {

		QWidget* widget = new QWidget(sliderWidget);
		QHBoxLayout* lay = new QHBoxLayout();
		lay->setSpacing(12);
		lay->setMargin(3);

		PluginSlider* slider = new PluginSlider();
		slider->setFixedWidth(200);
		slider->set_minimum(port->get_min_control_value());
		slider->set_maximum(port->get_max_control_value());
		slider->set_value(port->get_control_value());
		connect(slider, SIGNAL(sliderValueChanged(float)), port, SLOT(set_control_value(float)));
		
		// in case the plugin has a slave 'map' the signal to the slave port control slot too!
		if (m_plugin->get_slave()) {
			connect(slider, SIGNAL(sliderValueChanged(float)), m_plugin->get_slave()->get_control_port_by_index(port->get_index()), SLOT(set_control_value(float)));
		}
		
		QLabel* minvalue = new QLabel();
		minvalue->setNum(port->get_min_control_value());
		minvalue->setFixedWidth(35);
		minvalue->setAlignment(Qt::AlignRight);

		QLabel* maxvalue = new QLabel();
		maxvalue->setNum(port->get_max_control_value());
		maxvalue->setFixedWidth(35);
		maxvalue->setAlignment(Qt::AlignRight);

		QLabel* controlname = new QLabel(port->get_description());

		lay->addWidget(controlname);
		lay->addStretch(1);
		lay->addWidget(minvalue);
		lay->addWidget(slider);
		lay->addWidget(maxvalue);

		widget->setLayout(lay);

		sliderWidgetLayout->addWidget(widget);
	}
	
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(bypassButton, SIGNAL(clicked()), this, SLOT(bypass_button_clicked()));
}

void LV2PluginPropertiesDialog::bypass_button_clicked()
{
	m_plugin->toggle_bypass();
}

//eof

