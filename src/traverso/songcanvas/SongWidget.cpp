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
 
    $Id: SongWidget.cpp,v 1.8 2007/03/05 20:51:24 r_sijrier Exp $
*/

		
#include "SongWidget.h"
#include "TrackPanelViewPort.h"
#include "ClipsViewPort.h"
#include "TimeLineViewPort.h"
#include "SongView.h"
#include "ViewItem.h"
#include "Themer.h"

#include <Song.h>
#include <QtOpenGL>

#include <Debugger.h>


SongWidget::SongWidget(Song* song, QWidget* parent)
	: QFrame(parent)
{
	m_scene = new QGraphicsScene();
// 	m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);

	m_trackPanel = new TrackPanelViewPort(m_scene, this);
	m_clipsViewPort = new ClipsViewPort(m_scene, this);
	m_timeLine = new TimeLineViewPort(m_scene, this, m_clipsViewPort);
	
// 	m_clipsViewPort->setScene(m_scene);
	
	m_mainLayout = new QGridLayout(this);
	m_mainLayout->addWidget(new QWidget(this), 0, 0);
	m_mainLayout->addWidget(m_timeLine, 0, 1);
	m_mainLayout->addWidget(m_trackPanel, 1, 0);
	m_mainLayout->addWidget(m_clipsViewPort, 1, 1);
	m_mainLayout->addWidget(m_clipsViewPort->horizontalScrollBar(), 2, 1);
	m_mainLayout->addWidget(m_clipsViewPort->verticalScrollBar(), 1, 2);
	
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(0);
	
	setLayout(m_mainLayout);
	
	m_songView = new SongView(m_clipsViewPort, m_trackPanel, m_timeLine, song);
	m_timeLine->set_songview(m_songView);
	
	connect(m_clipsViewPort->horizontalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_timeLine->horizontalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(m_timeLine->horizontalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_clipsViewPort->horizontalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(m_clipsViewPort->verticalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_trackPanel->verticalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(m_trackPanel->verticalScrollBar(), 
		SIGNAL(valueChanged(int)),
		m_clipsViewPort->verticalScrollBar(), 
		SLOT(setValue(int)));
	
	connect(themer(), SIGNAL(themeLoaded()), this, SLOT(load_theme_data()), Qt::QueuedConnection);
	
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
}


SongWidget::~ SongWidget()
{
	delete m_trackPanel;
	delete m_clipsViewPort;
	delete m_timeLine;
	delete m_scene;
}


QSize SongWidget::minimumSizeHint() const
{
	return QSize(400, 200);
}

QSize SongWidget::sizeHint() const
{
	return QSize(700, 600);
}

void SongWidget::set_use_opengl( bool useOpenGL )
{
	m_clipsViewPort->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
	m_trackPanel->setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
}


void SongWidget::load_theme_data()
{
	QList<QGraphicsItem*> list = m_scene->items();
	
	for (int i = 0; i < list.size(); ++i) {
		ViewItem* item = qgraphicsitem_cast<ViewItem*>(list.at(i));
		if (item) {
			item->load_theme_data();
		}
	}
	
}

//eof
