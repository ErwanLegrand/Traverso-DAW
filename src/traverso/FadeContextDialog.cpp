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

$Id: FadeContextDialog.cpp,v 1.4 2006/08/08 20:59:49 r_sijrier Exp $
*/

#include "FadeContextDialog.h"
#include "FadeContextDialogView.h"
#include "ViewPort.h"
#include "FadeCurve.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>


FadeContextDialog::FadeContextDialog(FadeCurve* fadeCurve)
	: ContextDialog(), m_fade(fadeCurve)
{
	m_fadeCDV = new FadeContextDialogView(m_vp, fadeCurve);
	
	setWindowTitle(tr("Fade Editor"));

	// set the background color
	setAutoFillBackground(false);
	setAttribute(Qt::WA_PaintOnScreen);

	resize(450, 220);

	QVBoxLayout* mainlayout = new QVBoxLayout(this); 
	
	QWidget* definitionWidget = new QWidget(this);
	QHBoxLayout* definitionLayout = new QHBoxLayout(definitionWidget);
	
	QLabel* rasterDef = new QLabel("Raster: <r>");
	QLabel* bendDef = new QLabel("Bend: [b]");
	QLabel* strengthDef = new QLabel("Strength: [s]");
	QLabel* modeDef = new QLabel("Mode: <m>");
	QLabel* resetDef = new QLabel("Reset: <l>");
	
	definitionLayout->addWidget(rasterDef);
	definitionLayout->addWidget(bendDef);
	definitionLayout->addWidget(strengthDef);
	definitionLayout->addWidget(modeDef);
	definitionLayout->addWidget(resetDef);
	definitionLayout->setMargin(0);
	definitionLayout->setSpacing(0);
	definitionWidget->setLayout(definitionLayout);

	
	QWidget* valuesWidget = new QWidget(this);
	QHBoxLayout* valuesLayout = new QHBoxLayout(valuesWidget);
	
	m_bendLabel =  new QLabel();
	m_strengthLabel = new QLabel();
	m_modeLabel = new QLabel();
	m_rasterLabel = new QLabel();
	update_bend_value();
	update_mode_value();
	update_raster_value();
	update_strength_value();
	
	valuesLayout->addWidget(m_bendLabel);
	valuesLayout->addWidget(m_strengthLabel);
	valuesLayout->addWidget(m_modeLabel);
	valuesLayout->addWidget(m_rasterLabel);
	valuesLayout->setMargin(0);
	valuesLayout->setSpacing(0);
	valuesWidget->setLayout(valuesLayout);
	
	mainlayout->addWidget(definitionWidget);
	mainlayout->addWidget(m_vp, 10);
	mainlayout->addWidget(valuesWidget);
	mainlayout->setMargin(0);
	mainlayout->setSpacing(2);
	
	setLayout(mainlayout);
	
	connect(m_fade, SIGNAL(modeChanged()), this, SLOT(update_mode_value()));
	connect(m_fade, SIGNAL(bendValueChanged()), this, SLOT(update_bend_value()));
	connect(m_fade, SIGNAL(strengthValueChanged()), this, SLOT(update_strength_value()));
	connect(m_fade, SIGNAL(rasterChanged()), this, SLOT(update_raster_value()));
}

FadeContextDialog::~FadeContextDialog()
{
}

void FadeContextDialog::update_bend_value( )
{
	m_bendLabel->setText(QString("Bending: %1").arg(m_fade->get_bend_factor(), 0, 'f', 2)); 
}

void FadeContextDialog::update_strength_value( )
{
	m_strengthLabel->setText(QString("Strength: %1").arg(m_fade->get_strenght_factor(), 0, 'f', 2)); 
}

void FadeContextDialog::update_raster_value( )
{
	if (m_fade->get_raster()) {
		m_rasterLabel->setText("Raster: on");
	} else {
		m_rasterLabel->setText("Raster: off");
	}
}

void FadeContextDialog::update_mode_value( )
{
	if (m_fade->get_bend_factor() == 0.5) {
		m_modeLabel->setText("Mode: linear");
	} else {
		if (m_fade->get_mode() == 0)
			m_modeLabel->setText("Mode: bended");
		if (m_fade->get_mode() == 1)
			m_modeLabel->setText("Mode: s-shape");
		if (m_fade->get_mode() == 2)
			m_modeLabel->setText("Mode: long");
	}
}

//eof


